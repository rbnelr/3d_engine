#version 330 core // version 3.3

in		vec3	vs_dir_cubemap;

out		vec4	frag_col;

uniform mat3	common_cubemap_gl_ori_to_z_up;

uniform	samplerCube	tex;

void main () {
	frag_col = texture(tex, common_cubemap_gl_ori_to_z_up * normalize(vs_dir_cubemap));
}
