$include "common.frag"

in		vec3	vs_dir_world;

uniform mat3	common_cubemap_gl_ori_to_z_up;
uniform	mat3	common_world_to_skybox;

uniform vec3	common_skybox_light_dir_world;

uniform	samplerCube	tex;

vec4 frag () {
	vec3 dir_world = normalize(vs_dir_world);
	
	//if (dot(common_skybox_light_dir_world, dir_world) > 0.999)
	//	DEBUG(vec3(0,1,0));
	
	return texture(tex, common_cubemap_gl_ori_to_z_up * common_world_to_skybox * dir_world);
}
