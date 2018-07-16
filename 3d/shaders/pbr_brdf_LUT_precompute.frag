$include "common.frag"
$include "pbr_formulas.glsl"

uniform	int sample_count;

vec2 integrate_BRDF (float NdotV, float roughness) {
    vec2 uv = vec2(NdotV, roughness);
	
	vec3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;
	
    float A = 0.0;
    float B = 0.0;

    vec3 N = vec3(0.0, 0.0, 1.0);
	
    for (uint i = 0u; i<uint(sample_count); ++i) {
        vec2 Xi = hammersley(i, sample_count);
        vec3 H  = importance_sample_GGX(Xi, N, roughness);
        vec3 L  = normalize( reflect(-V, H) );
		
		float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.00001);
        float VdotH = max(dot(V, H), 0.0);
		
        if (NdotL > 0.0) {
            float G = geometry_smith_IBL(NdotV, NdotL, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
        
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    A /= float(sample_count);
    B /= float(sample_count);

    return vec2(A, B);
}

in		vec2	vs_uv;

vec4 frag () {
	vec2 integrated_BRDF = integrate_BRDF(vs_uv.x, vs_uv.y);
	return vec4(integrated_BRDF, 0, 1);
}
