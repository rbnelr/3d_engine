#version 330 core // version 3.3

in		vec3	pos_model;

out		vec3	vs_dir_cubemap;

uniform	mat4	_3d_view_cam_to_clip;
uniform	mat4	_3d_view_world_to_cam;

// for debugging
out		vec3	vs_barycentric;

const vec3[] BARYCENTRIC = vec3[] ( vec3(1,0,0), vec3(0,1,0), vec3(0,0,1) );

void main () {
	gl_Position =		_3d_view_cam_to_clip * vec4(mat3(_3d_view_world_to_cam) * pos_model, 1);
	vs_dir_cubemap =	pos_model;

	vs_barycentric = BARYCENTRIC[gl_VertexID % 3];
}