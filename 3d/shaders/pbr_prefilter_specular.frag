$include "common.frag"
$include "pbr_formulas.glsl"

in		vec3	vs_dir;

uniform	mat3	common_cubemap_gl_ori_to_z_up;
uniform	mat3	common_cubemap_z_up_to_gl_ori;

uniform	float	roughness;
uniform	int		sample_count;

uniform	samplerCube	source_radiance;
uniform	float		source_radiance_res;

float chetan_jags_lod (float NdotH, float HdotV, float source_radiance_res, int sample_count, float roughness) {
	
	float D   = distribution_GGX(NdotH, roughness);
	float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001; 

	float saTexel  = 4.0 * PI / (6.0 * source_radiance_res * source_radiance_res);
	float saSample = 1.0 / (float(sample_count) * pdf + 0.0001);

	float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
	return mipLevel;
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
				float lod = chetan_jags_lod(dot(N, H), dot(H, V), source_radiance_res, sample_count, roughness);

				prefiltered_color += textureLod(source_radiance, common_cubemap_z_up_to_gl_ori * L, lod).rgb * NdotL;
				total_weight      += NdotL;
			}
			
		}
		prefiltered_color = prefiltered_color / total_weight;
	}

	return vec4(prefiltered_color, 1);
}
