#version 330 core // version 3.3

in		vec3	pos_world;

out		vec3	vs_dir_world;

uniform	mat4	world_to_cubemap;
uniform	mat4	cubemap_to_cam;
uniform	mat4	cam_to_clip;

void main () {
	gl_Position =		cam_to_clip * cubemap_to_cam * world_to_cubemap * vec4(pos_world,1);
	vs_dir_world =		pos_world;
}
