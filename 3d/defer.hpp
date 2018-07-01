#pragma once

#include "compiler_specific.hpp"

template <typename FUNC>
struct _At_Scope_Exit {
	FUNC	f;
	void operator= (_At_Scope_Exit &) = delete;

	FORCEINLINE _At_Scope_Exit (FUNC f): f(f) {}
	FORCEINLINE ~_At_Scope_Exit () { f(); }
};

struct _Defer_Helper {};

template<typename FUNC>
FORCEINLINE _At_Scope_Exit<FUNC> operator+(_Defer_Helper, FUNC f) {
	return _At_Scope_Exit<FUNC>(f);
}

#define _defer(counter) auto CONCAT(_defer_helper, counter) = _Defer_Helper() +[&] () 
#define defer _defer(__COUNTER__)
// use like: defer { lambda code };
