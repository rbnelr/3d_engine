
union V2 {
	struct {
		T	x, y;
	};
	T		arr[2];
	
	ALWAYSINLINE T& operator[] (u32 i) {					return arr[i]; }
	constexpr ALWAYSINLINE T operator[] (u32 i) const {	return arr[i]; }
	
	ALWAYSINLINE V2 () {}
	ALWAYSINLINE constexpr V2 (T all):				x{all},	y{all} {}
	ALWAYSINLINE constexpr V2 (T x, T y):			x{x},	y{y} {}
	
#if !BOOLVEC
	INL V2& operator+= (V2 r) {						return *this = V2(x +r.x, y +r.y); }
	INL V2& operator-= (V2 r) {						return *this = V2(x -r.x, y -r.y); }
	INL V2& operator*= (V2 r) {						return *this = V2(x * r.x, y * r.y); }
	INL V2& operator/= (V2 r) {						return *this = V2(x / r.x, y / r.y); }
	
	#if FLTVEC
	INL constexpr operator u8v2() const;
	INL constexpr operator s64v2() const;
	INL constexpr operator s32v2() const;
	#endif
	#if INTVEC
	INL constexpr operator fv2() const {			return fv2((f32)x, (f32)y); }
	#endif
#endif
};

#if BOOLVEC
	INL constexpr bool all (V2 b) {				return b.x && b.y; }
	INL constexpr bool any (V2 b) {				return b.x || b.y; }
	
	INL constexpr V2 operator! (V2 b) {			return V2(!b.x,			!b.y); }
	INL constexpr V2 operator&& (V2 l, V2 r) {	return V2(l.x && r.x,	l.y && r.y); }
	INL constexpr V2 operator|| (V2 l, V2 r) {	return V2(l.x || r.x,	l.y || r.y); }
	INL constexpr V2 XOR (V2 l, V2 r) {			return V2(BOOL_XOR(l.x, r.x),	BOOL_XOR(l.y, r.y)); }
	
#else
	INL constexpr BV2 operator < (V2 l, V2 r) {	return BV2(l.x  < r.x,	l.y  < r.y); }
	INL constexpr BV2 operator<= (V2 l, V2 r) {	return BV2(l.x <= r.x,	l.y <= r.y); }
	INL constexpr BV2 operator > (V2 l, V2 r) {	return BV2(l.x  > r.x,	l.y  > r.y); }
	INL constexpr BV2 operator>= (V2 l, V2 r) {	return BV2(l.x >= r.x,	l.y >= r.y); }
	INL constexpr BV2 operator== (V2 l, V2 r) {	return BV2(l.x == r.x,	l.y == r.y); }
	INL constexpr BV2 operator!= (V2 l, V2 r) {	return BV2(l.x != r.x,	l.y != r.y); }
	INL constexpr V2 select (BV2 c, V2 l, V2 r) {
		return V2(	c.x ? l.x : r.x,	c.y ? l.y : r.y );
	}
	
	INL constexpr bool equal (V2 l, V2 r) {		return l.x == r.x && l.y == r.y; }
	
	INL constexpr V2 operator+ (V2 v) {			return v; }
	INL constexpr V2 operator- (V2 v) {			return V2(-v.x, -v.y); }
	
	INL constexpr V2 operator+ (V2 l, V2 r) {	return V2(l.x +r.x, l.y +r.y); }
	INL constexpr V2 operator- (V2 l, V2 r) {	return V2(l.x -r.x, l.y -r.y); }
	INL constexpr V2 operator* (V2 l, V2 r) {	return V2(l.x * r.x, l.y * r.y); }
	INL constexpr V2 operator/ (V2 l, V2 r) {	return V2(l.x / r.x, l.y / r.y); }
	#if INTVEC
	INL constexpr V2 operator% (V2 l, V2 r) {	return V2(l.x % r.x, l.y % r.y); }
	#endif
	
	INL constexpr T dot (V2 l, V2 r) {			return l.x*r.x + l.y*r.y; }

	INL V2 abs (V2 v) {							return V2(math::abs(v.x), math::abs(v.y)); }
	INL T max_component (V2 v) {				return math::max(v.x, v.y); }

	INL constexpr V2 min (V2 l, V2 r) {			return select(l <= r, l, r); }
	INL constexpr V2 max (V2 l, V2 r) {			return select(r >= l, r, l); }

	INL constexpr V2 clamp (V2 val, V2 l, V2 h) {	return min( max(val,l), h ); }
	
	#if FLTVEC
	INL constexpr V2 lerp (V2 a, V2 b, V2 t) {								return (a * (V2(1) -t)) +(b * t); }
	INL constexpr V2 map (V2 x, V2 in_a, V2 in_b) {							return (x -in_a)/(in_b -in_a); }
	INL constexpr V2 map (V2 x, V2 in_a, V2 in_b, V2 out_a, V2 out_b) {		return lerp(out_a, out_b, map(x, in_a, in_b)); }
	
	INL T length (V2 v) {						return sqrt(v.x*v.x +v.y*v.y); }
	INL T length_sqr (V2 v) {					return v.x*v.x +v.y*v.y; }
	INL T distance (V2 a, V2 b) {				return length(b -a); }
	INL V2 normalize (V2 v) {					return v / V2(length(v)); }
	
	INL constexpr V2 to_deg (V2 v) {			return v * RAD_TO_DEG; }
	INL constexpr V2 to_rad (V2 v) {			return v * DEG_TO_RAD; }
	
	INL V2 mymod (V2 val, V2 range) {			return V2(	math::mymod(val.x, range.x),	math::mymod(val.y, range.y) ); }

	INL V2 floor (V2 v) {						return V2(math::floor(v.x),		math::floor(v.y)); }
	INL V2 ceil (V2 v) {						return V2(math::ceil(v.x),		math::ceil(v.y)); }

	INL V2 pow (V2 v, V2 e) {					return V2(math::pow(v.x,e.x),	math::pow(v.y,e.y)); }
	#endif
#endif
