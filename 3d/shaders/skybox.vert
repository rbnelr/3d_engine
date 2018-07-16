$include "common.vert"

in		vec3	pos_world;

out		vec3	vs_dir_world;

uniform	mat4	cam_cam_to_clip;
uniform	mat4	cam_world_to_cam;

void vert () {
	gl_Position =		cam_cam_to_clip * vec4(mat3(cam_world_to_cam) * pos_world, 1);
	vs_dir_world =		pos_world;
}
