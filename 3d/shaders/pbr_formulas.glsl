
vec3 fresnel_schlick (float cos_theta, vec3 F0) {
	return F0 +(1.0 -F0) * pow(1.0 -cos_theta, 5.0);
}
vec3 fresnel (float VH, vec3 albedo, float metallic) {
	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 F  = fresnel_schlick(VH, F0);

	return F;
}

vec3 fresnel_schlick_roughness (float cos_theta, vec3 F0, float roughness) {
	return F0 +(max(vec3(1.0 -roughness), F0) -F0) * pow(1.0 -cos_theta, 5.0);
}
vec3 fresnel_roughness (float VH, vec3 albedo, float metallic, float roughness) {
	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 F  = fresnel_schlick_roughness(VH, F0, roughness);

	return F;
}

float distribution_GGX (float NH, float roughness) {
	float a      = roughness*roughness;
	float a2     = a*a;
	float NH2 = NH*NH;

	float num   = a2;
	float denom = (NH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float geometry_schlick_GGX (float NV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num   = NV;
	float denom = NV * (1.0 - k) + k;

	return num / denom;
}
float geometry_smith (float NV, float NL, float roughness) {
	float ggx2  = geometry_schlick_GGX(NV, roughness);
	float ggx1  = geometry_schlick_GGX(NL, roughness);

	return ggx1 * ggx2;
}

float geometry_schlick_GGX_IBL (float NV, float roughness) {
	float a = roughness;
	float k = (a * a) / 2.0;

	float num   = NV;
	float denom = NV * (1.0 - k) + k;

	return num / denom;
}
float geometry_smith_IBL (float NV, float NL, float roughness) {
	float ggx2  = geometry_schlick_GGX_IBL(NV, roughness);
	float ggx1  = geometry_schlick_GGX_IBL(NL, roughness);

	return ggx1 * ggx2;
}


float radical_inverse_VdC (uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
vec2 hammersley (uint i, int N) {
	return vec2(float(i) / float(N), radical_inverse_VdC(i));
}

vec3 importance_sample_GGX (vec2 Xi, vec3 N, float roughness) {
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

	// from spherical coordinates to cartesian coordinates
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	vec3 up        = abs(N.z) < 0.9999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;

	return normalize(sampleVec);
}
