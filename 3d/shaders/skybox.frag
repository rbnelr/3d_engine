#version 330 core // version 3.3

in		vec3	vs_dir_cubemap;

out		vec4	frag_col;

uniform	samplerCube	tex;

void main () {
	frag_col = texture(tex, normalize(vs_dir_cubemap));
}
