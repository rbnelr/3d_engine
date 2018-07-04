#version 330 core // version 3.3

in		vec3	vs_pos_cam;
in		vec3	vs_normal_cam;
in		vec4	vs_tangent_cam;
in		vec2	vs_uv;
in		vec4	vs_col;

out		vec4	frag_col;

uniform	mat4	_3d_view_world_to_cam;
uniform	mat4	_3d_view_cam_to_world;

uniform mat3	common_cubemap_gl_ori_to_z_up;
uniform	mat3	common_world_to_skybox;

uniform vec3	common_skybox_light_dir_world;

uniform vec3	common_ambient_light;

uniform sampler2D tex_albedo;
uniform sampler2D tex_metallic;
uniform sampler2D tex_roughness;
uniform sampler2D tex_normal;

uniform samplerCube skybox;

// for debugging
uniform bool	wireframe_enable = false;

in		vec3	vs_barycentric;

uniform float	wireframe_width = 1.2;

float wireframe_edge_factor () {
	vec3 d = fwidth(vs_barycentric);
	vec3 a3 = smoothstep(vec3(0.0), d * wireframe_width * 2, vs_barycentric);
	return min(min(a3.x, a3.y), a3.z);
}

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

vec3 fresnel_schlick (float cos_theta, vec3 F0) {
    return F0 +(1.0 -F0) * pow(1.0 -cos_theta, 5.0);
}
vec3 fresnel (vec3 frag_to_light, vec3 half_vec, vec3 albedo, float metallic) {
	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 F  = fresnelSchlick(max(dot(half_, frag_to_light), 0.0), F0);
	return F;
}

vec3 lighting_ambient (vec3 albedo, vec3 ambient_light) { // all in cam space
	return albedo * ambient_light;
}

vec3 lighting_dir_light (vec3 albedo, vec3 frag_to_light, vec3 normal) { // all in cam space
	float d = dot(frag_to_light, normal);

	vec3 lighting = albedo * max(d,0);

	return lighting;
}

void main () {
	
	vec4	 albedo_sample = texture(tex_albedo, vs_uv) * vs_col;
	vec3	 normal_sample = texture(tex_normal, vs_uv).rgb;
	float	 metallic_sample = texture(tex_metallic, vs_uv).r;
	float	 roughness_sample = texture(tex_roughness, vs_uv).r;
	
	vec3 albedo = albedo_sample.rgb;
	float alpha = albedo_sample.a;
	
	vec3 frag_to_light =	normalize(mat3(_3d_view_world_to_cam) * common_skybox_light_dir_world);
	vec3 vertex_normal =	normalize(vs_normal_cam);
	vec4 tangent = vec4(	normalize(vs_tangent_cam.xyz), vs_tangent_cam.w );
	
	vec3 normal = normalmapping(normal_sample, vertex_normal, tangent);

	vec3 lighting =  lighting_dir_light(albedo, frag_to_light, normal)
					+lighting_ambient(	albedo, common_ambient_light);

	frag_col = vec4(lighting, alpha);
	//frag_col = vec4(metallic_sample,metallic_sample,metallic_sample,1);
	
	//if (frag_col.a == 0) discard;
	//frag_col = vec4(normalize(vs_tangent_cam.xyz), 1);
	//frag_col = vec4(vs_uv,0,1);

	//vec3 reflected_cam = reflect(normalize(vs_pos_cam -0), normalize(vs_normal_cam));
	//vec3 reflected_world = mat3(_3d_view_cam_to_world) * reflected_cam;
	//frag_col = texture(skybox, common_cubemap_gl_ori_to_z_up * common_world_to_skybox * reflected_world);

	if (wireframe_enable) {
		float ef = wireframe_edge_factor();

		if (ef >= 0.5) discard;
		
		frag_col = mix(vec4(1,1,0,1), vec4(0,0,0,1), ef * 2);
	}
}
