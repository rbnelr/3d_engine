$include "common.frag"

in		vec3	vs_dir;

uniform	mat3	common_cubemap_gl_ori_to_z_up;
uniform	mat3	common_cubemap_z_up_to_gl_ori;

uniform	sampler2D	equirectangular;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec4 frag () {
	
	vec3 dir = normalize(vs_dir);

	vec2 uv = vec2(atan(dir.x, dir.y), asin(dir.z));
	
	uv *= invAtan;
	uv += 0.5;

	return texture(equirectangular, uv);
}
