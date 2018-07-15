
// common uniforms
uniform	vec2	common_viewport_size;
uniform	vec2	common_mcursor_pos;

// srgb conversions
vec3 to_srgb (vec3 linear) {
	bvec3 cutoff = lessThanEqual(linear, vec3(0.00313066844250063));
	vec3 higher = vec3(1.055) * pow(linear, vec3(1.0/2.4)) -vec3(0.055);
	vec3 lower = linear * vec3(12.92);

	return mix(higher, lower, cutoff);
}
vec3 to_linear (vec3 srgb) {
	bvec3 cutoff = lessThanEqual(srgb, vec3(0.0404482362771082));
	vec3 higher = pow((srgb +vec3(0.055)) / vec3(1.055), vec3(2.4));
	vec3 lower = srgb / vec3(12.92);

	return mix(higher, lower, cutoff);
}
vec4 to_srgb (vec4 linear) {	return vec4(to_srgb(linear.rgb), linear.a); }
vec4 to_linear (vec4 srgba) {	return vec4(to_linear(srgba.rgb), srgba.a); }

float map (float x, float a, float b) {
	return (x -a) / (b -a);
}
float map (float x, float in_a, float in_b, float out_a, float out_b) {
	return mix(out_a, out_b, map(x, in_a, in_b));
}

#define PI 3.1415926535897932384626433832795

#define DEG_TO_RAD	0.01745329251994329576923690768489	// 180/PI
#define RAD_TO_DEG	57.295779513082320876798154814105	// PI/180

float to_rad (float ang) {	return ang * DEG_TO_RAD; }
float deg (float ang) {		return ang * DEG_TO_RAD; }
float to_deg (float ang) {	return ang * RAD_TO_DEG; }
