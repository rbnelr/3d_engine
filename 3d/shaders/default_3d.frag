$include "common.frag"
$include "normalmapping.glsl"
$include "pbr_formulas.glsl"

in		vec3	vs_pos_cam;
in		vec3	vs_normal_cam;
in		vec4	vs_tangent_cam;
in		vec2	vs_uv;
in		vec4	vs_col;

uniform	mat4	cam_world_to_cam;
uniform	mat4	cam_cam_to_world;

uniform vec3	common_skybox_light_dir_world;
uniform	vec3	common_skybox_light_radiance;

// material
uniform sampler2D	albedo_tex;
uniform vec4		albedo_mult;
uniform vec4		albedo_offs;

uniform sampler2D	metallic_tex;
uniform float		metallic_mult;
uniform float		metallic_offs;

uniform sampler2D	roughness_tex;
uniform float		roughness_mult;
uniform float		roughness_offs;

uniform sampler2D	normal_tex;
uniform	vec3		normal_mult;
uniform	vec3		normal_offs;

uniform sampler2D	ao_tex;
uniform	float		ao_mult;
uniform	float		ao_offs;

//
uniform	mat3 common_world_to_skybox;
uniform	mat3 common_cubemap_gl_ori_to_z_up;
uniform	mat3 common_cubemap_z_up_to_gl_ori;

uniform samplerCube irradiance;
uniform samplerCube prefilter;
uniform sampler2D	brdf_LUT;

uniform	float		pbr_prefilter_levels;

vec3 cook_torrance_BRDF (vec3 albedo, float metallic, float roughness, float ao, vec3 frag_to_light, vec3 frag_to_cam, vec3 normal) { // all in cam space
	vec3 N = normalize(normal);
	vec3 V = normalize(frag_to_cam);
	vec3 L = normalize(frag_to_light);
	vec3 H = normalize(V + L);

	float NL = max(dot(N, L), 0.0);
	float NV = max(dot(N, V), 0.0);
	float VH = max(dot(H, V), 0.0);
	float NH = max(dot(H, N), 0.0);

	vec3 light_out = vec3(0);
	
	{ // analytical light (direct lights: point lights, directional lights)
		vec3 F = fresnel(VH, albedo, metallic);
		vec3 k_diffuse = 1 -F;

		k_diffuse *= 1 -metallic;
	
		float NDF = distribution_GGX(NH, roughness);       
		float G   = geometry_smith(NV, NL, roughness);

		vec3 numerator    = NDF * G * F;
		float denominator = 4.0 * NV * NL;
		vec3 specular     = numerator / max(denominator, 0.001);  

		light_out += (k_diffuse * albedo / PI + specular) * common_skybox_light_radiance * NL * ao;
	}
	{ // ibl (indirect or ambient light)
		vec3 F0 = mix(vec3(0.04), albedo, metallic);
		vec3 F = fresnel_schlick_roughness(NV, F0, roughness);
		
		{ // ibl diffuse
			vec3 k_diffuse = 1 -F;

			k_diffuse *= 1 -metallic;
		
			vec3 irradiance_sample = texture(irradiance, common_cubemap_z_up_to_gl_ori * common_world_to_skybox * mat3(cam_cam_to_world) * N).rgb;

			light_out += k_diffuse * irradiance_sample * albedo * ao;
		}
		{ // ibl specular
			vec3 R = reflect(-V, N);
			vec3 R_skybox = common_cubemap_z_up_to_gl_ori * mat3(common_world_to_skybox) * mat3(cam_cam_to_world) * R;

			vec3 prefiltered_color = textureLod(prefilter, R_skybox, roughness * pbr_prefilter_levels).rgb;

			vec2 brdf = texture(brdf_LUT, vec2(NV, roughness)).rg;
			
			vec3 specular = prefiltered_color * (F * brdf.x + brdf.y);

			light_out += specular * ao;
		}
	}

	return light_out;
}

vec4 frag () {
	
	vec4	 albedo_sample =	(texture(albedo_tex, vs_uv)			* albedo_mult		+ albedo_offs) * vs_col;
	float	 metallic_sample =	 texture(metallic_tex, vs_uv).r		* metallic_mult		+ metallic_offs;
	float	 roughness_sample =	 texture(roughness_tex, vs_uv).r	* roughness_mult	+ roughness_offs;
	vec3	 normal_sample =	 texture(normal_tex, vs_uv).rgb		* normal_mult		+ normal_offs;
	float	 ao_sample =		 texture(ao_tex, vs_uv).r			* ao_mult			+ ao_offs;
	
	vec3 albedo = albedo_sample.rgb;
	float alpha = albedo_sample.a;
	
	vec3 frag_to_cam =		normalize(-vs_pos_cam);
	vec3 frag_to_light =	normalize(mat3(cam_world_to_cam) * common_skybox_light_dir_world);
	vec3 vertex_normal =	normalize(vs_normal_cam);
	vec4 tangent = vec4(	normalize(vs_tangent_cam.xyz), vs_tangent_cam.w );
	
	vec3 normal = normalmapping(normal_sample, vertex_normal, tangent);
	
	vec3 radiance =  cook_torrance_BRDF(albedo, metallic_sample, roughness_sample, ao_sample, frag_to_light, frag_to_cam, normal);
	
	//radiance += texture(skybox, common_cubemap_z_up_to_gl_ori * common_world_to_skybox * mat3(cam_cam_to_world) * reflect(-frag_to_cam, normal)).rgb;
	
	vec3 color = radiance / (radiance + vec3(1.0)); // Reinhard tonemapping

	return vec4(color, alpha);
}
