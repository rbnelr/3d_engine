#version 330 core // version 3.3

in		vec4	pos_clip;
in		vec2	uv;

out		vec2	vs_uv;

void main () {
	gl_Position =		pos_clip;
	vs_uv =				uv;
}
