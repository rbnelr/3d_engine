#version 330 core // version 3.3

in		vec3	vs_pos_cam;
in		vec3	vs_normal_cam;
in		vec4	vs_tangent_cam;
in		vec2	vs_uv;
in		vec4	vs_col;

out		vec4	frag_col;

uniform	mat4	cam_to_world;

uniform mat3	common_cubemap_gl_ori_to_z_up;

uniform sampler2D tex;

uniform samplerCube skybox;

// for debugging
uniform bool	wireframe_enable = false;

in		vec3	vs_barycentric;

uniform float	wireframe_width = 1.2;

float wireframe_edge_factor () {
	vec3 d = fwidth(vs_barycentric);
	vec3 a3 = smoothstep(vec3(0.0), d * wireframe_width * 2, vs_barycentric);
	return min(min(a3.x, a3.y), a3.z);
}

void main () {
	//frag_col = texture(tex, vs_uv) * vs_col;
	//if (frag_col.a == 0) discard;
	//frag_col = vec4(normalize(vs_tangent_cam.xyz), 1);
	//frag_col = vec4(vs_uv,0,1);

	vec3 reflected_cam = reflect(normalize(vs_pos_cam -0), normalize(vs_normal_cam));
	vec3 reflected_world = mat3(cam_to_world) * reflected_cam;
	frag_col = texture(skybox, common_cubemap_gl_ori_to_z_up * reflected_world);

	if (wireframe_enable) {
		float ef = wireframe_edge_factor();

		if (ef >= 0.5) discard;
		
		frag_col = mix(vec4(1,1,0,1), vec4(0,0,0,1), ef * 2);
	}
}
