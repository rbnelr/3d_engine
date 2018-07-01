#pragma once

// needs v3 v4 typedefs

#include "vector.hpp"

namespace engine {
namespace colors {
	template <typename T> inline T to_linear (T srgb) {
		return select(srgb <= T(0.0404482362771082f),
					  srgb * (T(1)/T(12.92f)),
					  pow( (srgb +T(0.055f)) * T(1/1.055f), T(2.4f) )
		);
	}
	template <typename T> inline T to_srgb (T linear) {
		return select(linear <= T(0.00313066844250063f),
					  linear * T(12.92f),
					  ( T(1.055f) * pow(linear, T(1/2.4f)) ) -T(0.055f)
		);
	}

	typedef v3		lrgb;
	typedef v4		lrgba;

	struct srgb8 {
		u8v3	v;

		constexpr explicit srgb8 (u8v3 v): v{v} {}

		srgb8 () {}
		constexpr srgb8 (u8 all): v{all} {}
		constexpr explicit srgb8 (u8 r, u8 g, u8 b): v{r,g,b} {}

		static srgb8 from_linear (lrgb l) {
			return srgb8( (u8v3)(to_srgb(l) * 255 +0.5f) );
		}
		lrgb to_lrgb () const {
			return to_linear((v3)v / 255);
		}
	};
	struct srgba8 {
		u8v4	v;

		constexpr explicit srgba8 (u8v4 v): v{v} {}

		srgba8 () {}
		constexpr srgba8 (u8 all): v{all, 255} {}
		constexpr srgba8 (srgb8 rgb): v{rgb.v, 255} {}
		constexpr explicit srgba8 (u8 r, u8 g, u8 b): v{r,g,b,255} {}
		constexpr explicit srgba8 (u8 r, u8 g, u8 b, u8 a): v{r,g,b,a} {}

		static srgba8 from_linear (lrgba l) {
			return srgba8( (u8v4)(v4(to_srgb(l.xyz()), l.w) * 255 +0.5f) );
		}
		lrgba to_lrgb () const {
			v4 tmp = (v4)v / 255;
			return v4(to_linear(tmp.xyz()), tmp.z);
		}
	};

	// TODO: think about srgb vs lrgb here
	inline v3 _hsl_to_rgb (v3 hsl) { // hue is periodic since it represents the angle on the color wheel, so it can be out of the range [0,1]
		flt hue = hsl.x;
		flt sat = hsl.y;
		flt lht = hsl.z;

		flt hue6 = mymod(hue, 1.0f) * 6.0f;

		flt c = sat*(1.0f -abs(2.0f*lht -1.0f));
		flt x = c * (1.0f -abs(fmodf(hue6, 2.0f) -1.0f));
		flt m = lht -(c/2.0f);

		v3 srgb;
		if (		hue6 < 1.0f )	srgb = v3(c,x,0);
		else if (	hue6 < 2.0f )	srgb = v3(x,c,0);
		else if (	hue6 < 3.0f )	srgb = v3(0,c,x);
		else if (	hue6 < 4.0f )	srgb = v3(0,x,c);
		else if (	hue6 < 5.0f )	srgb = v3(x,0,c);
		else						srgb = v3(c,0,x);
		srgb += m;

		return srgb;
	}
}
using namespace colors;
}
