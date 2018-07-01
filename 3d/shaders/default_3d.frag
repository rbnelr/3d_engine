#version 330 core // version 3.3

in		vec2	vs_uv;
in		vec4	vs_col;

out		vec4	frag_col;

uniform sampler2D tex;

// for debugging
uniform bool	wireframe_enable = false;

in		vec3	vs_barycentric;

uniform float	wireframe_width = 1.2;

float wireframe_edge_factor () {
	vec3 d = fwidth(vs_barycentric);
	vec3 a3 = smoothstep(vec3(0.0), d * wireframe_width * 2, vs_barycentric);
	return min(min(a3.x, a3.y), a3.z);
}

void main () {
	frag_col = texture(tex, vs_uv) * vs_col;
	//if (frag_col.a == 0) discard;

	if (wireframe_enable) {
		float ef = wireframe_edge_factor();

		if (ef >= 0.5) discard;
		
		frag_col = mix(vec4(1,1,0,1), vec4(0,0,0,1), ef * 2);
	}
}
