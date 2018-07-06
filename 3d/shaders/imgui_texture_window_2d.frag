$include "common.frag"

in		vec2	vs_uv;

uniform	bvec4	show_channels;
uniform	float	show_lod;

uniform	float checker_board_pattern_tile_size = 10;

uniform sampler2D	tex;

vec4 frag () {
	ivec4 ishow_channels = ivec4(show_channels);
	int shown_count = (ishow_channels.r + ishow_channels.g) + (ishow_channels.b + ishow_channels.a);

	vec4 color = textureLod(tex, vs_uv, show_lod);

	vec4 mixed = mix(vec4(0), color, show_channels);
	
	if (shown_count == 1)
		return vec4(vec3((mixed.r + mixed.g) + (mixed.b + mixed.a)), 1);

	if (!show_channels.a)
		return vec4(mixed.rgb, 1);

	ivec2 tmp = ivec2(gl_FragCoord.xy / vec2(checker_board_pattern_tile_size));

	float checker_board_pattern = (tmp.x % 2) != (tmp.y % 2) ? 1 : 0;

	return vec4(mix(vec3(checker_board_pattern), mixed.rgb, vec3(color.a)), 1);
}
