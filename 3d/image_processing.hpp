#pragma once

#include "string.h"

#include "basic_typedefs.hpp"

namespace engine {
	void flip_vertical_inplace (void* rows, uptr row_size, uptr rows_count) {
		for (uptr row=0; row<rows_count/2; ++row) {
			char* row_a = (char*)rows +row_size * row;
			char* row_b = (char*)rows +row_size * (rows_count -1 -row);
			for (uptr i=0; i<row_size; ++i) {
				std::swap(row_a[i], row_b[i]);
			}
		}
	}
	void flip_vertical_copy (void* dst_rows, void* src_rows, uptr row_size, uptr rows_count) {
		for (uptr row=0; row<rows_count; ++row) {
			char* dst = (char*)dst_rows +row_size * row;
			char* src = (char*)src_rows +row_size * (rows_count -1 -row);
			memcpy(dst, src, row_size);
		}
	}
}
