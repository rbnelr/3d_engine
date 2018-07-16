#version 330 core // version 3.3

// dont use "common.frag" here, so that only when actuall changing "imgui.frag" this gets reloaded, also we dont want wireframe for imgui

in		vec2	vs_uv;
in		vec4	vs_col;

out		vec4	frag_col;

uniform sampler2D	tex;

void main () {
	frag_col = texture(tex, vs_uv).rgba * vs_col;
}
