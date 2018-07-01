#version 330 core // version 3.3

in		vec3	pos_model;
in		vec2	uv;
in		vec4	col_lrgba;

out		vec2	vs_uv;
out		vec4	vs_col;

uniform	mat4	_3d_view_cam_to_clip;
uniform	mat4	_3d_view_world_to_cam;

uniform mat4	model_to_world;

// for debugging
out		vec3	vs_barycentric;

const vec3[] BARYCENTRIC = vec3[] ( vec3(1,0,0), vec3(0,1,0), vec3(0,0,1) );

void main () {
	gl_Position =		_3d_view_cam_to_clip * _3d_view_world_to_cam * model_to_world * vec4(pos_model, 1);
	vs_uv =				uv;
	vs_col =			col_lrgba;

	vs_barycentric = BARYCENTRIC[gl_VertexID % 3];
}
