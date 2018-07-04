#version 330 core // version 3.3

in		vec3	vs_dir_world;

out		vec4	frag_col;

uniform mat3	common_cubemap_gl_ori_to_z_up;
uniform	mat3	common_world_to_skybox;

uniform vec3	common_skybox_light_dir_world;

uniform	samplerCube	tex;

void main () {
	vec3 dir_world = normalize(vs_dir_world);
	
	frag_col = texture(tex, common_cubemap_gl_ori_to_z_up * common_world_to_skybox * dir_world);

	//if (dot(common_skybox_light_dir_world, dir_world) > 0.999)
	//	frag_col = vec4(0,1,0,1);
		
}
