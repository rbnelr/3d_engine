$include "common.frag"

in		vec3	vs_dir;

uniform	mat3	common_cubemap_gl_ori_to_z_up;
uniform	mat3	common_cubemap_z_up_to_gl_ori;

uniform	vec3	dir_to_sun;

uniform	vec3	horiz_col = vec3(100,130,90) / 255;
uniform	vec3	sky_col = vec3(70,90,250) / 255;
uniform	vec3	ground_col = vec3(5,5,5) / 255;
uniform	vec3	sun_col = vec3(10,8,5) * 20;

vec4 frag () {
	//return vec4(normalize(vs_dir), 1);
	
	vec3 dir = normalize(vs_dir);

	float z = dir.z;

	vec3 col;
	if (z > 0)
		col = mix(horiz_col, sky_col, pow(z, 1.0 / 1.2));
	else
		col = mix(horiz_col, ground_col, pow(-z, 1.0 / 2));
	
	col += sun_col * max( map(dot(dir_to_sun, dir), cos(deg(3.0f)),1), 0);

	return vec4(col, 1);
}
