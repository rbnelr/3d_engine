
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

uniform sampler2D	height_tex;
uniform	float		height_mult;
uniform	float		height_offs;

vec2 parallax_mapping (vec3 vertex_normal_cam, vec4 tangent_cam, vec2 real_uv, vec3 frag_pos_cam) {
	
	float bitangent_sign = tangent_cam.w;
	vec3 bitangent_cam = cross(vertex_normal_cam, tangent_cam.xyz);

	tangent_cam.xyz = normalize(tangent_cam.xyz -(dot(tangent_cam.xyz, vertex_normal_cam) * vertex_normal_cam)); // make sure tangent space axes are orthogonal

	bitangent_cam *= bitangent_sign;

	mat3 tangent_to_cam = mat3(tangent_cam.xyz, bitangent_cam, vertex_normal_cam);
	mat3 cam_to_tangent = transpose(tangent_to_cam);

	vec3 frag_pos_tang = vec3(0);
	vec3 cam_pos_tang = cam_to_tangent * normalize(-frag_pos_cam);

	float	height = texture(height_tex, real_uv).r * height_mult + height_offs;

	vec2 offs = cam_pos_tang.xy / vec2(cam_pos_tang.z) * vec2(height);

	vec2 uv = real_uv + offs;

	return uv;
}
