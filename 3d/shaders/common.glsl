
// common uniforms
uniform	vec2	common_viewport_size;

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
