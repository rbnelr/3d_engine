$include "common.frag"

in		vec3	vs_dir;

uniform	mat3	common_cubemap_gl_ori_to_z_up;
uniform	mat3	common_cubemap_z_up_to_gl_ori;

uniform	float	roughness;
uniform	int		sample_count;

uniform	samplerCube	source_radiance;

const float PI = 3.1415926535897932384626433832795;

float radical_inverse_VdC (uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
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
	vec3 up        = abs(N.z) < 0.999999 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 0.0, 1.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	
	return normalize(sampleVec);
}  

vec4 frag () {
	
	vec3 N = normalize(vs_dir);    
	vec3 R = N;
	vec3 V = R;
	
	vec3 prefiltered_color; 

	if (roughness == 0) { // first prefilter level is always with 0 roughness -> no blurring at all -> 75% speedup
		
		prefiltered_color = texture(source_radiance, common_cubemap_z_up_to_gl_ori * N).rgb;

	} else {
		
		prefiltered_color = vec3(0.0); 
		float total_weight = 0.0;   
	
		for(uint i = 0u; i<uint(sample_count); ++i) {
	   
			vec2 Xi = hammersley(i, sample_count);
			vec3 H  = importance_sample_GGX(Xi, N, roughness);
			vec3 L  = normalize( -reflect(V, H) );
	
			float NdotL = max(dot(N, L), 0.0);
			if(NdotL > 0.0) {
				prefiltered_color += texture(source_radiance, common_cubemap_z_up_to_gl_ori * L).rgb * NdotL;
				total_weight      += NdotL;
			}
		
		}
		prefiltered_color = prefiltered_color / total_weight;
	}

	return vec4(prefiltered_color, 1);
}
