#version 330 core // version 3.3

in		vec3	pos_world;

out		vec3	vs_dir_world;

uniform	mat3	world_to_cam;
uniform	mat4	cam_to_clip;

void main () {
	gl_Position =		cam_to_clip * vec4(world_to_cam * pos_world, 1);
	vs_dir_world =		pos_world;
}
