#version 330 core // version 3.3

//$predefined_macros
#define WIREFRAME 1

void vert ();

#if WIREFRAME
out		vec3	vs_barycentric;

const vec3[] BARYCENTRIC = vec3[] ( vec3(1,0,0), vec3(0,1,0), vec3(0,0,1) );
#endif

$include "common.glsl"

void main () {
	vert();
	
#if WIREFRAME
	vs_barycentric = BARYCENTRIC[gl_VertexID % 3];
#endif
}
