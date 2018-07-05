$include "common.vert"

in		vec3	pos_world;

out		vec3	vs_dir_world;

uniform	mat4	_3d_view_cam_to_clip;
uniform	mat4	_3d_view_world_to_cam;

void vert () {
	gl_Position =		_3d_view_cam_to_clip * vec4(mat3(_3d_view_world_to_cam) * pos_world, 1);
	vs_dir_world =		pos_world;
}
