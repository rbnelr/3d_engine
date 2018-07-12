#version 330 core // version 3.3

// dont use "common.vert" here, so that only when actuall changing "imgui.vert" this gets reloaded, also we dont want wireframe for imgui

in		vec2	pos_screen;
in		vec2	uv;
in		vec4	col_srgba;

out		vec2	vs_uv;
out		vec4	vs_col;

//
uniform	vec2	common_viewport_size;

vec3 to_linear (vec3 srgb) {
	bvec3 cutoff = lessThanEqual(srgb, vec3(0.0404482362771082));
	vec3 higher = pow((srgb +vec3(0.055)) / vec3(1.055), vec3(2.4));
	vec3 lower = srgb / vec3(12.92);

	return mix(higher, lower, cutoff);
}
vec4 to_linear (vec4 srgba) {	return vec4(to_linear(srgba.rgb), srgba.a); }

void main () {
	vec2 pos = pos_screen / common_viewport_size;
	pos.y = 1 -pos.y; // positions are specified top-down
	
	gl_Position =		vec4(pos * 2 -1, 0,1);
	vs_uv =				uv;
	vs_col =			to_linear(col_srgba);
}
