$include "common.vert"

in		vec3	pos;

out		vec3	vs_dir;

uniform	mat3	to_cam;
uniform	mat4	cam_to_clip;

void vert () {
	gl_Position =	cam_to_clip * vec4(to_cam * pos, 1);
	vs_dir =		pos;
}
