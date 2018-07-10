$include "common.vert"

in		vec3	pos_cubemap_space;

out		vec3	vs_dir_cubemap_space;

uniform	mat3	cubemap_space_to_cam;
uniform	mat4	cam_to_clip;

void vert () {
	gl_Position =				cam_to_clip * vec4(cubemap_space_to_cam * pos_cubemap_space, 1);
	vs_dir_cubemap_space =		pos_cubemap_space;
}
