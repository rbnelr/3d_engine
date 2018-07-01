#pragma once

// needs v3 v4 typedefs

#include "vector.hpp"

namespace engine {
namespace colors {
	typedef u8v3	rgb8;
	typedef u8v4	rgba8;
	typedef v3		rgbf;
	typedef v4		rgbaf;

	constexpr rgb8	colf_to_col8 (rgbf c) {		return  (rgb8)(	c * 255 +0.5f ); }
	constexpr rgba8	colf_to_col8 (rgbaf c) {	return (rgba8)(	c * 255 +0.5f ); }
	constexpr rgbf	col8_to_colf (rgb8 c) {		return    (rgbf)c / 255; }
	constexpr rgbaf	col8_to_colf (rgba8 c) {	return   (rgbaf)c / 255; }

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

	inline rgbf hsl_to_rgb (rgbf hsl) { // hue is periodic since it represents the angle on the color wheel, so it can be out of the range [0,1]
		flt hue = hsl.x;
		flt sat = hsl.y;
		flt lht = hsl.z;

		flt hue6 = mymod(hue, 1.0f) * 6.0f;

		flt c = sat*(1.0f -abs(2.0f*lht -1.0f));
		flt x = c * (1.0f -abs(fmodf(hue6, 2.0f) -1.0f));
		flt m = lht -(c/2.0f);

		rgbf rgb;
		if (		hue6 < 1.0f )	rgb = rgbf(c,x,0);
		else if (	hue6 < 2.0f )	rgb = rgbf(x,c,0);
		else if (	hue6 < 3.0f )	rgb = rgbf(0,c,x);
		else if (	hue6 < 4.0f )	rgb = rgbf(0,x,c);
		else if (	hue6 < 5.0f )	rgb = rgbf(x,0,c);
		else						rgb = rgbf(c,0,x);
		rgb += m;

		return to_linear(rgb);
	}
}
using namespace colors;
}
