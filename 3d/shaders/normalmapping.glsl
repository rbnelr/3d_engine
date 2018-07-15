
vec3 normalmapping (vec3 normal_map_val, vec3 vertex_normal_cam, vec4 tangent_cam) {
	vec3 normal_tangent = normal_map_val * 2 -1;

	float bitangent_sign = tangent_cam.w;
	vec3 bitangent_cam = cross(vertex_normal_cam, tangent_cam.xyz);

	tangent_cam.xyz = normalize(tangent_cam.xyz -(dot(tangent_cam.xyz, vertex_normal_cam) * vertex_normal_cam)); // make sure tangent space axes are orthogonal

	bitangent_cam *= bitangent_sign;

	mat3 tangent_to_cam = mat3(tangent_cam.xyz, bitangent_cam, vertex_normal_cam);

	vec3 normal_cam = tangent_to_cam * normal_tangent;

	return normal_cam;
}
