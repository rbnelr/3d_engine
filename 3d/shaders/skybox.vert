#version 330 core // version 3.3

in		vec3	pos_world;

out		vec3	vs_dir_world;

uniform	mat4	_3d_view_cam_to_clip;
uniform	mat4	_3d_view_world_to_cam;

// for debugging
out		vec3	vs_barycentric;

const vec3[] BARYCENTRIC = vec3[] ( vec3(1,0,0), vec3(0,1,0), vec3(0,0,1) );

void main () {
	gl_Position =		_3d_view_cam_to_clip * vec4(mat3(_3d_view_world_to_cam) * pos_world, 1);
	vs_dir_world =		pos_world;

	vs_barycentric = BARYCENTRIC[gl_VertexID % 3];
}
