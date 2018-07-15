$include "common.frag"

in		vec3	vs_dir_world;

uniform mat3	common_cubemap_z_up_to_gl_ori;
uniform	mat3	common_world_to_skybox;

uniform vec3	common_skybox_light_dir_world;

uniform	samplerCube	skybox;

vec4 frag () {
	vec3 dir_world = normalize(vs_dir_world);
	
	//DEBUG( normalize(vs_dir_world) );

	//if (dot(common_skybox_light_dir_world, dir_world) > 0.9999)
	//	DEBUG(vec3(0,1,0));
	
	vec3 radiance = texture(skybox, common_cubemap_z_up_to_gl_ori * common_world_to_skybox * dir_world).rgb;
	
	vec3 color = radiance / (radiance + vec3(1.0)); // Reinhard tonemapping
	
	return vec4(color, 1);
}
