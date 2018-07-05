$include "common.vert"

in		vec2	pos_screen;
in		vec2	uv;
in		vec4	col_srgba;

out		vec2	vs_uv;
out		vec4	vs_col;

void vert () {
	vec2 pos = pos_screen / common_viewport_size;
	pos.y = 1 -pos.y; // positions are specified top-down
	
	gl_Position =		vec4(pos * 2 -1, 0,1);
	vs_uv =				uv;
	vs_col =			to_linear(col_srgba);
}
