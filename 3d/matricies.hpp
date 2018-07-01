
struct M2 {
	V2 arr[2];

	ALWAYSINLINE explicit M2 () {}
private: // don't allow a contructor with columns mayor order because it could be confusing, use static functions rows and columns instead, still need a contructor though, to implement the functions below
	ALWAYSINLINE explicit constexpr M2 (V2 a, V2 b): arr{a,b} {}
public:

	static INL constexpr M2 rows (		V2 a, V2 b ) {				return M2{V2(a.x,b.x),V2(b.y,b.y)}; }
	static INL constexpr M2 columns (	V2 a, V2 b ) {				return M2{a,b}; }
	static INL constexpr M2 rows (		T a, T b,
										T e, T f ) {				return M2{V2(a,e),V2(b,f)}; }
	static INL constexpr M2 ident () {								return rows(1,0, 0,1); }
	static INL constexpr M2 nan () {								return rows(QNAN,QNAN, QNAN,QNAN); }

	INL M2& operator*= (M2 r);
};
struct M3 {
	friend struct HM;

	V3 arr[3];

	ALWAYSINLINE explicit M3 () {}
private: //
	ALWAYSINLINE explicit constexpr M3 (V3 a, V3 b, V3 c): arr{a,b,c} {}
public:

	static INL constexpr M3 rows (		V3 a, V3 b, V3 c ) {		return M3{V3(a.x,b.x,c.x),V3(a.y,b.y,c.y),V3(a.z,b.z,c.z)}; }
	static INL constexpr M3 columns (	V3 a, V3 b, V3 c ) {		return M3{a,b,c}; }
	static INL constexpr M3 rows (		T a, T b, T c,
										T e, T f, T g,
										T i, T j, T k ) {			return M3{V3(a,e,i),V3(b,f,j),V3(c,g,k)}; }
	static INL constexpr M3 ident () {								return rows(1,0,0, 0,1,0, 0,0,1); }
	static INL constexpr M3 nan () {								return rows(QNAN,QNAN,QNAN, QNAN,QNAN,QNAN, QNAN,QNAN,QNAN); }
	INL constexpr M3 (M2 m): arr{V3(m.arr[0], 0), V3(m.arr[1], 0), V3(0,0,1)} {}

	INL M2 m2 () const {											return M2::columns( arr[0].xy(), arr[1].xy() ); }

	INL M3& operator*= (M3 r);
};
struct M4 {
	V4 arr[4];

	ALWAYSINLINE explicit M4 () {}
private: //
	ALWAYSINLINE explicit constexpr M4 (V4 a, V4 b, V4 c, V4 d): arr{a,b,c,d} {}
public:

	static INL constexpr M4 rows (		V4 a, V4 b, V4 c, V4 d ) {	return M4{V4(a.x,b.x,c.x,d.x),V4(a.y,b.y,c.y,d.y),V4(a.z,b.z,c.z,d.z),V4(a.w,b.w,c.w,d.w)}; }
	static INL constexpr M4 columns (	V4 a, V4 b, V4 c, V4 d ) {	return M4{a,b,c,d}; }
	static INL constexpr M4 rows (		T a, T b, T c, T d,
										T e, T f, T g, T h,
										T i, T j, T k, T l,
										T m, T n, T o, T p ) {		return M4{V4(a,e,i,m),V4(b,f,j,n),V4(c,g,k,o),V4(d,h,l,p)}; }
	static INL constexpr M4 ident () {								return rows(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1); }
	static INL constexpr M4 nan () {								return rows(QNAN,QNAN,QNAN,QNAN, QNAN,QNAN,QNAN,QNAN, QNAN,QNAN,QNAN,QNAN, QNAN,QNAN,QNAN,QNAN); }
	INL constexpr M4 (M2 m): arr{V4(m.arr[0], 0,0), V4(m.arr[1], 0,0), V4(0,0,1,0), V4(0,0,0,1)} {}
	INL constexpr M4 (M3 m): arr{V4(m.arr[0], 0), V4(m.arr[1], 0), V4(m.arr[2], 0), V4(0,0,0,1)} {}

	INL M2 m2 () const {											return M2::columns( arr[0].xy(), arr[1].xy() ); }
	INL M3 m3 () const {											return M3::columns( arr[0].xyz(), arr[1].xyz(), arr[2].xyz() ); }

	INL M4& operator*= (M4 r);
};

/*
Lots of people seem to use 4x4 matrices to store/apply scale/rotation/translation
The matrix then looks like this:
a b c x
d e f y
g h i z
0 0 0 1
Which they then multipy by a vector like this: (x y z 1) if they want the translation or like this: (x y z 0) if they want to just rotate

Which is mathematicly elegant but wastes 4 floats (16 bytes) that always store (0 0 0 1) in storage and quite a lot of processing power for no reason

As an optimization i use a matrix called hm / HM (for homogeneous matrix, which i think this is called ??)
that just stores the 3x3 matrix (rotation / scale) and a 3d vetctor (translation) and have an extra set of operator overloads
*/
struct HM {
	M3 mat;
	V3 transl;

	ALWAYSINLINE explicit HM () {}
private: //
	ALWAYSINLINE explicit constexpr HM (V3 a, V3 b, V3 c, V3 d): mat{a,b,c}, transl{d} {}
public:

	static INL constexpr HM rows (		V4 a, V4 b, V4 c ) {		return HM{V3(a.x,b.x,c.x),V3(a.y,b.y,c.y),V3(a.z,b.z,c.z),V3(a.w,b.w,c.w)}; }
	static INL constexpr HM columns (	V3 a, V3 b, V3 c, V3 d ) {	return HM{a,b,c,d}; }
	static INL constexpr HM rows (		T a, T b, T c, T d,
										T e, T f, T g, T h,
										T i, T j, T k, T l ) {		return HM{V3(a,e,i),V3(b,f,j),V3(c,g,k),V3(d,h,l)}; }
	static INL constexpr HM ident () {								return rows(1,0,0,0, 0,1,0,0, 0,0,1,0); }
	static INL constexpr HM nan () {								return rows(QNAN,QNAN,QNAN,QNAN, QNAN,QNAN,QNAN,QNAN, QNAN,QNAN,QNAN,QNAN); }
	INL constexpr HM (M2 m): mat{V3(m.arr[0], 0), V3(m.arr[1], 0), V3(0,0,1)}, transl{0} {}
	INL constexpr HM (M3 m): mat{m}, transl{0} {}
	INL constexpr HM (M3 m, V3 t): mat{m}, transl{t} {}

	INL M2 m2 () const {											return M2::columns( mat.arr[0].xy(), mat.arr[1].xy() ); }
	INL M3 m3 () const {											return mat; }
	INL M4 m4 () const {											return M4::columns( V4(mat.arr[0],0), V4(mat.arr[1],0), V4(mat.arr[2],0), V4(transl,1) ); }

	INL HM& operator*= (HM r);
};

INL V2 operator* (M2 m, V2 v) {
	V2 ret;
	ret.x = m.arr[0].x * v.x  +m.arr[1].x * v.y;
	ret.y = m.arr[0].y * v.x  +m.arr[1].y * v.y;
	return ret;
}
INL M2 operator* (M2 l, M2 r) {
	M2 ret;
	ret.arr[0] = l * r.arr[0];
	ret.arr[1] = l * r.arr[1];
	return ret;
}

INL V3 operator* (M3 m, V3 v) {
	V3 ret;
	ret.x = m.arr[0].x * v.x  +m.arr[1].x * v.y  +m.arr[2].x * v.z;
	ret.y = m.arr[0].y * v.x  +m.arr[1].y * v.y  +m.arr[2].y * v.z;
	ret.z = m.arr[0].z * v.x  +m.arr[1].z * v.y  +m.arr[2].z * v.z;
	return ret;
}
INL M3 operator* (M3 l, M3 r) {
	M3 ret;
	ret.arr[0] = l * r.arr[0];
	ret.arr[1] = l * r.arr[1];
	ret.arr[2] = l * r.arr[2];
	return ret;
}

INL V4 operator* (M4 m, V4 v) {
	V4 ret;
	ret.x = m.arr[0].x * v.x  +m.arr[1].x * v.y  +m.arr[2].x * v.z  +m.arr[3].x * v.w;
	ret.y = m.arr[0].y * v.x  +m.arr[1].y * v.y  +m.arr[2].y * v.z  +m.arr[3].y * v.w;
	ret.z = m.arr[0].z * v.x  +m.arr[1].z * v.y  +m.arr[2].z * v.z  +m.arr[3].z * v.w;
	ret.w = m.arr[0].w * v.x  +m.arr[1].w * v.y  +m.arr[2].w * v.z  +m.arr[3].w * v.w;
	return ret;
}
INL M4 operator* (M4 l, M4 r) {
	M4 ret;
	ret.arr[0] = l * r.arr[0];
	ret.arr[1] = l * r.arr[1];
	ret.arr[2] = l * r.arr[2];
	ret.arr[3] = l * r.arr[3];
	return ret;
}

INL V3 operator* (HM m, V3 v) { // the common case of wanting to translate/rotate/scale some v3 -> if you just want to rotate/scale, instead of doing this: "some_hm * v4(some_v3,0)" -> just do: "some_hm.m3() * some_v3"
	return m.mat * v +m.transl;
}
INL HM operator* (HM l, HM r) {
	HM ret;
	#if 1
	ret.mat.arr[0] = l.m3() * r.mat.arr[0];	// implicit r.columns[0].w = 0
	ret.mat.arr[1] = l.m3() * r.mat.arr[1];	// implicit r.columns[1].w = 0
	ret.mat.arr[2] = l.m3() * r.mat.arr[2];	// implicit r.columns[2].w = 0
	ret.transl =	 l * r.transl;			// implicit r.columns[3].w = 1
	#else // not tested
	ret.arr[0].x = l.mat.arr[0].x * r.mat.arr[0].x  +l.mat.arr[1].x * r.mat.arr[0].y  +l.mat.arr[2].x * r.mat.arr[0].z;
	ret.arr[0].y = l.mat.arr[0].y * r.mat.arr[0].x  +l.mat.arr[1].y * r.mat.arr[0].y  +l.mat.arr[2].y * r.mat.arr[0].z;
	ret.arr[0].z = l.mat.arr[0].z * r.mat.arr[0].x  +l.mat.arr[1].z * r.mat.arr[0].y  +l.mat.arr[2].z * r.mat.arr[0].z;
	
	ret.arr[1].x = l.mat.arr[0].x * r.mat.arr[1].x  +l.mat.arr[1].x * r.mat.arr[1].y  +l.mat.arr[2].x * r.mat.arr[1].z;
	ret.arr[1].y = l.mat.arr[0].y * r.mat.arr[1].x  +l.mat.arr[1].y * r.mat.arr[1].y  +l.mat.arr[2].y * r.mat.arr[1].z;
	ret.arr[1].z = l.mat.arr[0].z * r.mat.arr[1].x  +l.mat.arr[1].z * r.mat.arr[1].y  +l.mat.arr[2].z * r.mat.arr[1].z;
	
	ret.arr[2].x = l.mat.arr[0].x * r.mat.arr[2].x  +l.mat.arr[1].x * r.mat.arr[2].y  +l.mat.arr[2].x * r.mat.arr[2].z;
	ret.arr[2].y = l.mat.arr[0].y * r.mat.arr[2].x  +l.mat.arr[1].y * r.mat.arr[2].y  +l.mat.arr[2].y * r.mat.arr[2].z;
	ret.arr[2].z = l.mat.arr[0].z * r.mat.arr[2].x  +l.mat.arr[1].z * r.mat.arr[2].y  +l.mat.arr[2].z * r.mat.arr[2].z;
	
	ret.arr[3].x = l.mat.arr[0].x * r.transl.x  +l.mat.arr[1].x * r.transl.y  +l.mat.arr[2].x * r.transl.z  +l.transl.x;
	ret.arr[3].y = l.mat.arr[0].y * r.transl.x  +l.mat.arr[1].y * r.transl.y  +l.mat.arr[2].y * r.transl.z  +l.transl.y;
	ret.arr[3].z = l.mat.arr[0].z * r.transl.x  +l.mat.arr[1].z * r.transl.y  +l.mat.arr[2].z * r.transl.z  +l.transl.z;
	#endif
	return ret;
}

INL M2& M2::operator*= (M2 r) {
	return *this = *this * r;
}
INL M3& M3::operator*= (M3 r) {
	return *this = *this * r;
}
INL M4& M4::operator*= (M4 r) {
	return *this = *this * r;
}
INL HM& HM::operator*= (HM r) {
	return *this = *this * r;
}

INL M2 inverse (M2 m) {
	T inv_det = T(1) / ( (m.arr[0].x * m.arr[1].y) -(m.arr[1].x * m.arr[0].y) );

	M2 ret;
	ret.arr[0].x = m.arr[1].y * +inv_det;
	ret.arr[0].y = m.arr[0].y * -inv_det;
	ret.arr[1].x = m.arr[1].x * -inv_det;
	ret.arr[1].y = m.arr[0].x * +inv_det;
	return ret;
}
// TODO: m3 and m4 inverse, but i try to program in such a way, that in never need to calc a inverse

INL M2 scale2 (V2 v) {
	return M2::columns(	V2(v.x,0),
						V2(0,v.y) );
}
INL M2 rotate2 (T ang) {
	auto sc = sin_cos(ang);
	return M2::rows(	+sc.c,	-sc.s,
						+sc.s,	+sc.c );
}

INL fv2 rotate2_90 (fv2 v) {
	return V2(-v.y, v.x);
}
INL fv2 rotate2_180 (fv2 v) {
	return -v;
}
INL fv2 rotate2_270 (fv2 v) {
	return V2(v.y, -v.x);
}

INL M3 scale3 (V3 v) {
	return M3::columns(	V3(v.x,0,0),
						V3(0,v.y,0),
						V3(0,0,v.z) );
}
INL M3 rotate3_X (T ang) {
	auto sc = sin_cos(ang);
	return M3::rows(	1,		0,		0,
						0,		+sc.c,	-sc.s,
						0,		+sc.s,	+sc.c);
}
INL M3 rotate3_Y (T ang) {
	auto sc = sin_cos(ang);
	return M3::rows(	+sc.c,	0,		+sc.s,
						0,		1,		0,
						-sc.s,	0,		+sc.c);
}
INL M3 rotate3_Z (T ang) {
	auto sc = sin_cos(ang);
	return M3::rows(	+sc.c,	-sc.s,	0,
						+sc.s,	+sc.c,	0,
						0,		0,		1);
}

INL M4 translate4 (V3 v) {
	return M4::columns(	V4(1,0,0,0),
						V4(0,1,0,0),
						V4(0,0,1,0),
						V4(v,1) );
}
INL M4 scale4 (V3 v) {
	return M4::columns(	V4(v.x,0,0,0),
						V4(0,v.y,0,0),
						V4(0,0,v.z,0),
						V4(0,0,0,1) );
}
INL M4 rotate4_X (T ang) {
	auto sc = sin_cos(ang);
	return M4::rows(	1,		0,		0,		0,
						0,		+sc.c,	-sc.s,	0,
						0,		+sc.s,	+sc.c,	0,
						0,		0,		0,		1 );
}
INL M4 rotate4_Y (T ang) {
	auto sc = sin_cos(ang);
	return M4::rows(	+sc.c,	0,		+sc.s,	0,
						0,		1,		0,		0,
						-sc.s,	0,		+sc.c,	0,
						0,		0,		0,		1 );
}
INL M4 rotate4_Z (T ang) {
	auto sc = sin_cos(ang);
	return M4::rows(	+sc.c,	-sc.s,	0,		0,
						+sc.s,	+sc.c,	0,		0,
						0,		0,		1,		0,
						0,		0,		0,		1 );
}

INL HM translateH (V3 v) {
	return HM::columns(	V3(1,0,0),
						V3(0,1,0),
						V3(0,0,1),
						v );
}
INL HM scaleH (V3 v) {
	return HM::columns(	V3(v.x,0,0),
						V3(0,v.y,0),
						V3(0,0,v.z),
						V3(0,0,0) );
}
INL HM rotateH_X (T ang) {
	auto sc = sin_cos(ang);
	return HM::rows(	1,		0,		0,		0,
						0,		+sc.c,	-sc.s,	0,
						0,		+sc.s,	+sc.c,	0 );
}
INL HM rotateH_Y (T ang) {
	auto sc = sin_cos(ang);
	return HM::rows(	+sc.c,	0,		+sc.s,	0,
						0,		1,		0,		0,
						-sc.s,	0,		+sc.c,	0 );
}
INL HM rotateH_Z (T ang) {
	auto sc = sin_cos(ang);
	return HM::rows(	+sc.c,	-sc.s,	0,		0,
						+sc.s,	+sc.c,	0,		0,
						0,		0,		1,		0 );
}
