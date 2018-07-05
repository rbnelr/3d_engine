$include "common.frag"

in		vec3	vs_dir_world;

uniform	mat3	common_cubemap_z_up_to_gl_ori;

vec4 frag () {
	//frag_col = vec4(normalize(vs_dir_world), 1);
	
	///*
	float z = normalize(common_cubemap_z_up_to_gl_ori * vs_dir_world).z;
	
	vec3 horiz_col = vec3(100,130,90) / 255;
	vec3 sky_col = vec3(70,90,250) / 255;
	vec3 ground_col = vec3(5,5,5) / 255;

	vec3 col;
	if (z > 0)
		col = mix(horiz_col, sky_col, pow(z, 1.0 / 1.2));
	else
		col = mix(horiz_col, ground_col, pow(-z, 1.0 / 2));

	return vec4(col, 1);
}
