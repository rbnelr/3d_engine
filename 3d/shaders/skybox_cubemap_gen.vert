$include "common.vert"

in		vec3	pos_skybox;

out		vec3	vs_dir_skybox;

uniform	mat3	skybox_to_cam;
uniform	mat4	cam_to_clip;

void vert () {
	gl_Position =	cam_to_clip * vec4(skybox_to_cam * pos_skybox, 1);
	vs_dir_skybox =	pos_skybox;
}
