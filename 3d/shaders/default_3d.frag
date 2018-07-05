#version 330 core // version 3.3

in		vec3	vs_pos_cam;
in		vec3	vs_normal_cam;
in		vec4	vs_tangent_cam;
in		vec2	vs_uv;
in		vec4	vs_col;

out		vec4	frag_col;

uniform	mat4	_3d_view_world_to_cam;
uniform	mat4	_3d_view_cam_to_world;

uniform mat3	common_cubemap_gl_ori_to_z_up;
uniform	mat3	common_world_to_skybox;

uniform vec3	common_skybox_light_dir_world;

uniform vec3	common_ambient_light;

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

uniform samplerCube skybox;

// for debugging
void SHOW (float v) {	frag_col = vec4(v,v,v,1); }
void SHOW (vec2 v) {	frag_col = vec4(v,0,1); }
void SHOW (vec3 v) {	frag_col = vec4(v,1); }
void SHOW (vec4 v) {	frag_col = v; }

uniform bool	wireframe_enable = false;

in		vec3	vs_barycentric;

uniform float	wireframe_width = 1.2;

float wireframe_edge_factor () {
	vec3 d = fwidth(vs_barycentric);
	vec3 a3 = smoothstep(vec3(0.0), d * wireframe_width * 2, vs_barycentric);
	return min(min(a3.x, a3.y), a3.z);
}

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

#define PI 3.1415926535897932384626433832795

vec3 fresnel_schlick (float cos_theta, vec3 F0) {
    return F0 +(1.0 -F0) * pow(1.0 -cos_theta, 5.0);
}
vec3 fresnel (vec3 frag_to_light, float VH, vec3 albedo, float metallic) {
	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 F  = fresnel_schlick(VH, F0);

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


vec3 lighting_ambient (vec3 albedo, vec3 ambient_light) { // all in cam space
	return albedo * ambient_light;
}

vec3 lighting_dir_light (vec3 albedo, float metallic, float roughness, vec3 frag_to_light, vec3 frag_to_cam, vec3 normal) { // all in cam space
	vec3 N = normalize(normal);
	vec3 V = normalize(frag_to_cam);
	vec3 L = normalize(frag_to_light);
	vec3 H = normalize(V + L);

	float NL = max(dot(N, L), 0.0);
	float NV = max(dot(N, V), 0.0);
	float VH = max(dot(H, V), 0.0);
	float NH = max(dot(H, N), 0.0);

	vec3 F = fresnel(frag_to_light, VH, albedo, metallic);
	vec3 k_diffuse = 1 -F;

	k_diffuse *= 1 -metallic;
	
	float NDF = distribution_GGX(NH, roughness);       
	float G   = geometry_smith(NV, NL, roughness);

	vec3 numerator    = NDF * G * F;
	float denominator = 4.0 * NV * NL;
	vec3 specular     = numerator / max(denominator, 0.001);  

	vec3 lighting = (k_diffuse * albedo / PI + specular) * NL;

	return lighting;
}

void main () {
	
	vec4	 albedo_sample =	(texture(albedo_tex, vs_uv)			* albedo_mult		+ albedo_offs) * vs_col;
	float	 metallic_sample =	 texture(metallic_tex, vs_uv).r		* metallic_mult		+ metallic_offs;
	float	 roughness_sample =	 texture(roughness_tex, vs_uv).r	* roughness_mult	+ roughness_offs;
	vec3	 normal_sample =	 texture(normal_tex, vs_uv).rgb;
	
	vec3 albedo = albedo_sample.rgb;
	float alpha = albedo_sample.a;
	
	vec3 frag_to_cam =		normalize(-vs_pos_cam);
	vec3 frag_to_light =	normalize(mat3(_3d_view_world_to_cam) * common_skybox_light_dir_world);
	vec3 vertex_normal =	normalize(vs_normal_cam);
	vec4 tangent = vec4(	normalize(vs_tangent_cam.xyz), vs_tangent_cam.w );
	
	vec3 normal = normalmapping(normal_sample, vertex_normal, tangent);

	vec3 Lo =  lighting_dir_light(albedo, metallic_sample, roughness_sample, frag_to_light, frag_to_cam, normal)
					+lighting_ambient(	albedo, common_ambient_light);
	
	vec3 color = Lo / (Lo + vec3(1.0)); // Reinhard tonemapping

	frag_col = vec4(color, alpha);
	//frag_col = vec4(metallic_sample,metallic_sample,metallic_sample,1);
	
	if (wireframe_enable) {
		float ef = wireframe_edge_factor();

		if (ef >= 0.5) discard;
		
		frag_col = mix(vec4(1,1,0,1), vec4(0,0,0,1), ef * 2);
	}
}
