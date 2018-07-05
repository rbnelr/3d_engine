$include "common.frag"

in		vec2	vs_uv;
in		vec4	vs_col;

uniform sampler2D	tex;

vec4 frag () {
	return texture(tex, vs_uv).rgba * vs_col;
}
