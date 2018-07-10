$include "common.frag"

in		vec3	vs_dir_cubemap_space;

uniform	mat3	common_cubemap_gl_ori_to_z_up;
uniform	mat3	common_cubemap_z_up_to_gl_ori;

uniform	samplerCube	source_radiance;

const float PI = 3.1415926535897932384626433832795;

vec4 frag () {
	vec3 irradiance_total = vec3(0.0);
	
	vec3 normal = normalize(vs_dir_cubemap_space);
	
    vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, normal);
	up         = cross(normal, right);

	float sample_delta = 0.025;
	float samples_count = 0.0; 

	for (float phi = 0.0; phi < 2.0 * PI; phi += sample_delta) {
		for (float theta = 0.0; theta < 0.5 * PI; theta += sample_delta) {
			// spherical to cartesian (in tangent space)
			vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

			irradiance_total += texture(source_radiance, sampleVec).rgb * cos(theta) * sin(theta);
			samples_count++;
		}
	}

	vec3 irradiance = PI * irradiance_total * (1.0 / float(samples_count));

	return vec4(irradiance, 1);
}
