
vec3 normalmapping (vec3 normal_map_val, vec3 vertex_normal_cam, vec4 tangent_cam) {
	vec3 normal_tangent = normal_map_val * 2 -1;

	float bitangent_sign = tangent_cam.w;
	vec3 bitangent_cam = cross(vertex_normal_cam, tangent_cam.xyz);

	tangent_cam.xyz = normalize(tangent_cam.xyz -(dot(tangent_cam.xyz, vertex_normal_cam) * vertex_normal_cam)); // make sure tangent space axes are orthogonal

	bitangent_cam *= bitangent_sign;

	mat3 tangent_to_cam = mat3(tangent_cam.xyz, bitangent_cam, vertex_normal_cam);

	vec3 normal_cam = tangent_to_cam * normal_tangent;

	return normal_cam;
}

uniform sampler2D	displacement_tex;
uniform	float		displacement_size;
uniform	int			displacement_layers_count = 10;

vec2 parallax_mapping (vec3 vertex_normal_cam, vec4 tangent_cam, vec2 real_uv, vec3 frag_pos_cam) {
	
	float bitangent_sign = tangent_cam.w;
	vec3 bitangent_cam = cross(vertex_normal_cam, tangent_cam.xyz);

	tangent_cam.xyz = normalize(tangent_cam.xyz -(dot(tangent_cam.xyz, vertex_normal_cam) * vertex_normal_cam)); // make sure tangent space axes are orthogonal

	bitangent_cam *= bitangent_sign;

	mat3 tangent_to_cam = mat3(tangent_cam.xyz, bitangent_cam, vertex_normal_cam);
	mat3 cam_to_tangent = transpose(tangent_to_cam);

	vec3 frag_pos_tang = vec3(0);
	vec3 view_dir = cam_to_tangent * normalize(-frag_pos_cam);

	vec2 uv = real_uv;
	{
		float layers_depth = abs(displacement_size);
		float layer_depth = layers_depth / float(displacement_layers_count);

		vec2 uv_step = view_dir.xy / view_dir.zz * layer_depth;

		float ray_depth = 0.0;

		for (;;) {
			
			float surface_depth = texture(displacement_tex, uv).r;
			if (displacement_size < 0.0)
				surface_depth = 1.0 - surface_depth;
			surface_depth *= layers_depth;

			if (ray_depth >= surface_depth)
				break;

			uv -= uv_step;
			ray_depth += layer_depth;
		}
		
	}
	return uv;
}
