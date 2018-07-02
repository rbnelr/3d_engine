#pragma once
#include "basic_typedefs.hpp"
#include "math.hpp"
#include "preprocessor_stuff.hpp" // for BOOL_XOR

namespace engine {
namespace vector {
//

#if 0 // debug
	#define constexpr 
	#define ALWAYSINLINE inline
	#define INL inline
#else
	#define ALWAYSINLINE FORCEINLINE // to prevent tons of small functions in debug mode
	#define INL inline
#endif

union u8v2;
union u8v3;
union u8v4;
union s32v2;
union s32v3;
union s32v4;
union s64v2;
union s64v3;
union s64v4;

//
#define T	bool
#define V2	bv2
#define V3	bv3
#define V4	bv4
#define BOOLVEC	1
	
	#include "vector_tv2.hpp"
	#include "vector_tv3.hpp"
	#include "vector_tv4.hpp"
	
#undef T
#undef V2
#undef V3
#undef V4
#undef BOOLVEC

#define BOOLVEC	0
#define BV2	bv2
#define BV3	bv3
#define BV4	bv4

//
#define FLTVEC 1

#define T	f32
#define V2	fv2
#define V3	fv3
#define V4	fv4
	
	#include "vector_tv2.hpp"
	#include "vector_tv3.hpp"
	#include "vector_tv4.hpp"
	
#undef T
#undef V2
#undef V3
#undef V4

//
#define T	f64
#define V2	dv2
#define V3	dv3
#define V4	dv4
	
	#include "vector_tv2.hpp"
	#include "vector_tv3.hpp"
	#include "vector_tv4.hpp"
	
#undef T
#undef V2
#undef V3
#undef V4

#undef FLTVEC

//
#define INTVEC	1

#define T	s32
#define V2	s32v2
#define V3	s32v3
#define V4	s32v4
	
	#include "vector_tv2.hpp"
	#include "vector_tv3.hpp"
	#include "vector_tv4.hpp"
	
#undef T
#undef V2
#undef V3
#undef V4

#define T	s64
#define V2	s64v2
#define V3	s64v3
#define V4	s64v4
	
	#include "vector_tv2.hpp"
	#include "vector_tv3.hpp"
	#include "vector_tv4.hpp"
	
#undef T
#undef V2
#undef V3
#undef V4

#define T	u8
#define V2	u8v2
#define V3	u8v3
#define V4	u8v4
	
	#include "vector_tv2.hpp"
	#include "vector_tv3.hpp"
	#include "vector_tv4.hpp"
	
#undef T
#undef V2
#undef V3
#undef V4

#undef INTVEC

#undef BOOLVEC
#undef BV2
#undef BV3
#undef BV4

INL constexpr fv2::operator u8v2() const {	return u8v2((u8)x, (u8)y); }
INL constexpr fv3::operator u8v3() const {	return u8v3((u8)x, (u8)y, (u8)z); }
INL constexpr fv4::operator u8v4() const {	return u8v4((u8)x, (u8)y, (u8)z, (u8)w); }

INL constexpr fv2::operator s32v2() const {	return s32v2((s32)x, (s32)y); }
INL constexpr fv3::operator s32v3() const {	return s32v3((s32)x, (s32)y, (s32)z); }
INL constexpr fv4::operator s32v4() const {	return s32v4((s32)x, (s32)y, (s32)z, (s32)w); }

INL constexpr fv2::operator s64v2() const {	return s64v2((s64)x, (s64)y); }
INL constexpr fv3::operator s64v3() const {	return s64v3((s64)x, (s64)y, (s64)z); }
INL constexpr fv4::operator s64v4() const {	return s64v4((s64)x, (s64)y, (s64)z, (s64)w); }

//
#define T	f32
#define V2	fv2
#define V3	fv3
#define V4	fv4
#define M2	fm2
#define M3	fm3
#define M4	fm4
#define HM	fhm

#include "matricies.hpp"

#define QUAT fquat

union QUAT {
	struct {
		T	x, y, z, w;
	};
	T		arr[4];
	
	ALWAYSINLINE QUAT () {}
	ALWAYSINLINE constexpr QUAT (T x, T y, T z, T w):	x{x},	y{y},	z{z}, w{w} {}
	ALWAYSINLINE constexpr QUAT (V3 v, T w):			x{v.x},	y{v.y},	z{v.z}, w{w} {}
	
	ALWAYSINLINE constexpr V3 xyz () const {	return V3(x,y,z); };
	
	static INL constexpr QUAT ident () {		return QUAT(0,0,0, 1); }
	static INL constexpr QUAT nan () {			return QUAT(QNAN,QNAN,QNAN,QNAN); }

	INL QUAT& operator*= (QUAT r);
};

INL M3 convert_to_m3 (QUAT q) {
	M3 ret;

	T xx = q.x * q.x;
	T yy = q.y * q.y;
	T zz = q.z * q.z;
	T xz = q.x * q.z;
	T xy = q.x * q.y;
	T yz = q.y * q.z;
	T wx = q.w * q.x;
	T wy = q.w * q.y;
	T wz = q.w * q.z;

	ret.arr[0] = V3(	1 -(2 * (yy +zz)),		// y z 
						    2 * (xy +wz),	// yx zw
						    2 * (xz -wy) ); // zx yw

	ret.arr[1] = V3(	    2 * (xy -wz),	// xy zw
						1 -(2 * (xx +zz)),		// x z
						    2 * (yz +wx) ); // zy xw

	ret.arr[2] = V3(	    2 * (xz +wy),	// xz yw
						    2 * (yz -wx),	// yz xw
						1 -(2 * (xx +yy)) );	// x y

	return ret;
}

INL V3 operator* (QUAT q, V3 v) { // not tested

	V3 l_vec = q.xyz();
	V3 uv = cross(l_vec, v);
	V3 uuv = cross(l_vec, uv);

	return v + (uv * q.w +uuv) * 2;
}

INL QUAT operator* (QUAT l, QUAT r) {
	return QUAT(	(l.w * r.x) +(l.x * r.w) +(l.y * r.z) -(l.z * r.y),
					(l.w * r.y) +(l.y * r.w) +(l.z * r.x) -(l.x * r.z),
					(l.w * r.z) +(l.z * r.w) +(l.x * r.y) -(l.y * r.x),
					(l.w * r.w) -(l.x * r.x) -(l.y * r.y) -(l.z * r.z) );
}
INL QUAT& QUAT::operator*= (QUAT r) {
	return *this = *this * r;
}

INL QUAT conjugate (QUAT q) { // not tested
	return QUAT(-q.xyz(), q.w);
}

INL QUAT inverse (QUAT q) { // not tested
	QUAT conj = conjugate(q);

	V4 qv = V4(q.x,q.y,q.z,q.w);
	T d = dot(qv, qv);

	return QUAT(	conj.x / d,
					conj.y / d,
					conj.z / d,
					conj.w / d );
}

fquat rotateQ (V3 axis, T ang) {
	auto res = sin_cos(ang / 2.0f);
	return fquat( axis * V3(res.s), res.c );
}
fquat rotateQ_X (T ang) {
	return rotateQ(V3(+1, 0, 0), ang);
}
fquat rotateQ_Y (T ang) {
	return rotateQ(V3( 0,+1, 0), ang);
}
fquat rotateQ_Z (T ang) {
	return rotateQ(V3( 0, 0,+1), ang);
}

#undef T
#undef V2
#undef V3
#undef V4
#undef M2
#undef M3
#undef M4

#undef constexpr 
#undef ALWAYSINLINE
#undef INL

//
}
using namespace vector;
}
