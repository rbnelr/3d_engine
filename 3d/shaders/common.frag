#version 330 core // version 3.3

//$predefined_macros
#define WIREFRAME 1

$include "common.glsl"

vec4 frag ();

// Debug output
bool dbg_output_set = false;

out		vec4	frag_col;

void DEBUG (float v) {	dbg_output_set = true; frag_col = vec4(v,v,v,1); }
void DEBUG (vec2 v) {	dbg_output_set = true; frag_col = vec4(v,0,1); }
void DEBUG (vec3 v) {	dbg_output_set = true; frag_col = vec4(v,1); }
void DEBUG (vec4 v) {	dbg_output_set = true; frag_col = v; }

bool dbg_left () {
	if (gl_FragCoord.x == common_mcursor_pos.x)
		DEBUG(0);
	return gl_FragCoord.x < common_mcursor_pos.x;
}
bool dbg_right () {
	if (gl_FragCoord.x == common_mcursor_pos.x)
		DEBUG(0);
	return gl_FragCoord.x > common_mcursor_pos.x;
}
bool dbg_down () {
	if (gl_FragCoord.y == common_mcursor_pos.y)
		DEBUG(0);
	return gl_FragCoord.y < common_mcursor_pos.y;
}
bool dbg_up () {
	if (gl_FragCoord.y == common_mcursor_pos.y)
		DEBUG(0);
	return gl_FragCoord.y > common_mcursor_pos.y;
}

vec2 mcursor () {
	return common_mcursor_pos / common_viewport_size;
}

#if WIREFRAME
uniform bool	wireframe_enable = false;

in		vec3	vs_barycentric;

uniform float	wireframe_width = 1.2;

float wireframe_edge_factor () {
	vec3 d = fwidth(vs_barycentric);
	vec3 a3 = smoothstep(vec3(0.0), d * wireframe_width * 2, vs_barycentric);
	return min(min(a3.x, a3.y), a3.z);
}
#endif

void main () {
	
	vec4 col = frag();
	if (!dbg_output_set)
		frag_col = col;

#if WIREFRAME
	if (wireframe_enable) {
		float ef = wireframe_edge_factor();

		if (ef >= 0.5) discard;
		
		frag_col = mix(vec4(1,1,0,1), vec4(0,0,0,1), ef * 2);
	}
#endif
}
