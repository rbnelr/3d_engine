#pragma once
#include <string>
#include <cstdarg>

#include "basic_typedefs.hpp"
#include "math.hpp"

#define ANSI_COLOUR_CODE_RED	"\033[1;31m"
#define ANSI_COLOUR_CODE_YELLOW	"\033[1;33m"
#define ANSI_COLOUR_CODE_NC		"\033[0m"

namespace engine {
	// Printf that outputs to a std::string
	inline void vprints (std::string* s, cstr format, va_list vl) { // print 
		size_t old_size = s->size();
		for (;;) {
			auto ret = vsnprintf(&(*s)[old_size], s->size() -old_size +1, format, vl); // i think i'm technically not allowed to overwrite the null terminator
			ret = max(0, ret);
			bool was_bienough = (u32)ret < (s->size() -old_size +1);
			s->resize(old_size +(u32)ret);
			if (was_bienough) break;
			// buffer was to small, buffer size was increased
			// now snprintf has to succeed, so call it again
		}
	}
	inline void prints (std::string* s, cstr format, ...) {
		va_list vl;
		va_start(vl, format);

		vprints(s, format, vl);

		va_end(vl);
	}
	inline std::string prints (cstr format, ...) {
		va_list vl;
		va_start(vl, format);

		std::string ret;
		vprints(&ret, format, vl);

		va_end(vl);

		return ret;
	}

	bool is_lower (char c) {
		return c >= 'a' && c <= 'z';
	}
	bool is_upper (char c) {
		return c >= 'A' && c <= 'Z';
	}

	char to_lower (char c) {
		return is_upper(c) ? c +('a' -'A') : c;
	}
	char to_upper (char c) {
		return is_lower(c) ? c +('A' -'a') : c;
	}

}
