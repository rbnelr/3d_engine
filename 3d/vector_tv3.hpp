
union V3 {
	struct {
		T	x, y, z;
	};
	T		arr[3];
	
	ALWAYSINLINE T& operator[] (u32 i) {					return arr[i]; }
	constexpr ALWAYSINLINE T operator[] (u32 i) const {	return arr[i]; }
	
	ALWAYSINLINE V3 () {}
	ALWAYSINLINE constexpr V3 (T all):				x{all},	y{all},	z{all} {}
	ALWAYSINLINE constexpr V3 (T x, T y, T z):		x{x},	y{y},	z{z} {}
	ALWAYSINLINE constexpr V3 (V2 v, T z):			x{v.x},	y{v.y},	z{z} {}
	
	ALWAYSINLINE constexpr V2 xy () const {			return V2(x,y); };
	
#if !BOOLVEC
	INL V3& operator+= (V3 r) {						return *this = V3(x +r.x, y +r.y, z +r.z); }
	INL V3& operator-= (V3 r) {						return *this = V3(x -r.x, y -r.y, z -r.z); }
	INL V3& operator*= (V3 r) {						return *this = V3(x * r.x, y * r.y, z * r.z); }
	INL V3& operator/= (V3 r) {						return *this = V3(x / r.x, y / r.y, z / r.z); }

	#if FLTVEC
	INL constexpr operator u8v3() const;
	INL constexpr operator s64v3() const;
	INL constexpr operator s32v3() const;
	#endif
	#if INTVEC
	INL constexpr operator fv3() const {			return fv3((f32)x, (f32)y, (f32)z); }
	#endif
#endif
};

#if BOOLVEC
	INL constexpr bool all (V3 b) {				return b.x && b.y && b.z; }
	INL constexpr bool any (V3 b) {				return b.x || b.y || b.z; }
	
	INL constexpr V3 operator! (V3 b) {			return V3(!b.x,			!b.y,		!b.z); }
	INL constexpr V3 operator&& (V3 l, V3 r) {	return V3(l.x && r.x,	l.y && r.y,	l.z && r.z); }
	INL constexpr V3 operator|| (V3 l, V3 r) {	return V3(l.x || r.x,	l.y || r.y,	l.z || r.z); }
	INL constexpr V3 XOR (V3 l, V3 r) {			return V3(BOOL_XOR(l.x, r.x),	BOOL_XOR(l.y, r.y),	BOOL_XOR(l.z, r.z)); }
	
#else
	INL constexpr BV3 operator < (V3 l, V3 r) {	return BV3(l.x  < r.x,	l.y  < r.y,	l.z  < r.z); }
	INL constexpr BV3 operator<= (V3 l, V3 r) {	return BV3(l.x <= r.x,	l.y <= r.y,	l.z <= r.z); }
	INL constexpr BV3 operator > (V3 l, V3 r) {	return BV3(l.x  > r.x,	l.y  > r.y,	l.z  > r.z); }
	INL constexpr BV3 operator>= (V3 l, V3 r) {	return BV3(l.x >= r.x,	l.y >= r.y,	l.z >= r.z); }
	INL constexpr BV3 operator== (V3 l, V3 r) {	return BV3(l.x == r.x,	l.y == r.y,	l.z == r.z); }
	INL constexpr BV3 operator!= (V3 l, V3 r) {	return BV3(l.x != r.x,	l.y != r.y,	l.z != r.z); }
	INL constexpr V3 select (BV3 c, V3 l, V3 r) {
		return V3(	c.x ? l.x : r.x,	c.y ? l.y : r.y,	c.z ? l.z : r.z );
	}
	
	INL constexpr bool equal (V3 l, V3 r) {		return l.x == r.x && l.y == r.y && l.z == r.z; }
	
	INL constexpr V3 operator+ (V3 v) {			return v; }
	INL constexpr V3 operator- (V3 v) {			return V3(-v.x, -v.y, -v.z); }

	INL constexpr V3 operator+ (V3 l, V3 r) {	return V3(l.x +r.x, l.y +r.y, l.z +r.z); }
	INL constexpr V3 operator- (V3 l, V3 r) {	return V3(l.x -r.x, l.y -r.y, l.z -r.z); }
	INL constexpr V3 operator* (V3 l, V3 r) {	return V3(l.x * r.x, l.y * r.y, l.z * r.z); }
	INL constexpr V3 operator/ (V3 l, V3 r) {	return V3(l.x / r.x, l.y / r.y, l.z / r.z); }
	#if INTVEC
	INL constexpr V3 operator% (V3 l, V3 r) {	return V3(l.x % r.x, l.y % r.y, l.z % r.z); }
	#endif
	
	INL constexpr T dot(V3 l, V3 r) {			return l.x*r.x + l.y*r.y + l.z*r.z; }

	INL constexpr V3 cross(V3 l, V3 r) {
		return V3(l.y*r.z - l.z*r.y, l.z*r.x - l.x*r.z, l.x*r.y - l.y*r.x);
	}

	INL V3 abs(V3 v) {							return V3(math::abs(v.x), math::abs(v.y), math::abs(v.z)); }
	INL T max_component(V3 v) {					return math::max(math::max(v.x, v.y), v.z); }

	INL constexpr V3 min(V3 l, V3 r) {			return select(l <= r, l, r); }
	INL constexpr V3 max(V3 l, V3 r) {			return select(r >= l, r, l); }
	
	INL constexpr V3 clamp (V3 val, V3 l, V3 h) {	return min( max(val,l), h ); }
	
#if FLTVEC
	INL constexpr V3 lerp (V3 a, V3 b, V3 t) {								return (a * (V3(1) -t)) +(b * t); }
	INL constexpr V3 map (V3 x, V3 in_a, V3 in_b) {							return (x -in_a)/(in_b -in_a); }
	INL constexpr V3 map (V3 x, V3 in_a, V3 in_b, V3 out_a, V3 out_b) {		return lerp(out_a, out_b, map(x, in_a, in_b)); }
	
	INL T length (V3 v) {						return sqrt(v.x*v.x +v.y*v.y +v.z*v.z); }
	INL T length_sqr (V3 v) {					return v.x*v.x +v.y*v.y +v.z*v.z; }
	INL T distance (V3 a, V3 b) {				return length(b -a); }
	INL V3 normalize (V3 v) {					return v / V3(length(v)); }
	
	INL constexpr V3 to_deg (V3 v) {			return v * RAD_TO_DEG; }
	INL constexpr V3 to_rad (V3 v) {			return v * DEG_TO_RAD; }

	INL V3 mymod (V3 val, V3 range) {			return V3(	math::mymod(val.x, range.x),	math::mymod(val.y, range.y),	math::mymod(val.z, range.z) ); }
	
	INL V3 floor (V3 v) {						return V3(math::floor(v.x),	math::floor(v.y),	math::floor(v.z)); }
	INL V3 ceil (V3 v) {						return V3(math::ceil(v.x),	math::ceil(v.y),	math::ceil(v.z)); }

	INL V3 pow (V3 v, V3 e) {					return V3(math::pow(v.x,e.x),	math::pow(v.y,e.y),	math::pow(v.z,e.z)); }
	#endif
#endif
