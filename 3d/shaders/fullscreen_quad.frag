$include "common.frag"

in		vec2	vs_uv;

uniform sampler2D tex;

vec4 frag () {
	return texture(tex, vs_uv);
}
