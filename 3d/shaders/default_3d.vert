#version 330 core // version 3.3

in		vec3	pos_model;
in		vec3	normal_model;
in		vec4	tangent_model;
in		vec2	uv;
in		vec4	col_lrgba;

out		vec3	vs_pos_cam;
out		vec3	vs_normal_cam;
out		vec4	vs_tangent_cam;
out		vec2	vs_uv;
out		vec4	vs_col;

uniform	mat4	_3d_view_cam_to_clip;
uniform	mat4	_3d_view_world_to_cam;

uniform mat4	model_to_world;

// for debugging
out		vec3	vs_barycentric;

const vec3[] BARYCENTRIC = vec3[] ( vec3(1,0,0), vec3(0,1,0), vec3(0,0,1) );

void main () {
	mat4 model_to_cam = _3d_view_world_to_cam * model_to_world;

	gl_Position =		_3d_view_cam_to_clip * model_to_cam * vec4(pos_model, 1);
	
	vs_pos_cam =		(model_to_cam * vec4(pos_model, 1)).xyz;
	vs_normal_cam =		      mat3(model_to_cam) * normal_model;
	vs_tangent_cam =	vec4( mat3(model_to_cam) * tangent_model.xyz, tangent_model.w);
	vs_uv =				uv;
	vs_col =			col_lrgba;

	vs_barycentric = BARYCENTRIC[gl_VertexID % 3];
}
