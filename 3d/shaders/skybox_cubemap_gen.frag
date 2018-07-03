#version 330 core // version 3.3

in		vec3	vs_dir_world;

out		vec4	frag_col;



void main () {
	//frag_col = vec4(normalize(vs_dir_world), 1);
	
	float z = normalize(vs_dir_world).z;
	
	vec3 horiz_col = vec3(100,130,90) / 255;
	vec3 sky_col = vec3(70,90,250) / 255;
	vec3 ground_col = vec3(5,5,5) / 255;

	vec3 col;
	if (z > 0)
		col = mix(horiz_col, sky_col, pow(z, 1.0 / 1.2));
	else
		col = mix(horiz_col, ground_col, pow(-z, 1.0 / 2));

	frag_col = vec4(col, 1);
}
