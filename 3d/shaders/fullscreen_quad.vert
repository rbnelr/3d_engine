$include "common.vert"

in		vec4	pos_clip;
in		vec2	uv;

out		vec2	vs_uv;

void vert () {
	gl_Position =		pos_clip;
	vs_uv =				uv;
}
