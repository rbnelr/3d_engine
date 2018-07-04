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

	void inplace_rotate_180 (void* pixels, uptr pixel_size, iv2 size_px) {
		uptr row = 0;
		for (; row < size_px.y / 2; ++row) {
			
			uptr reverse_row = size_px.y -1 -row;

			for (uptr x=0; x<size_px.x; ++x) {
				uptr reverse_x = size_px.x -1 -x;

				char* px_a = (char*)pixels +(row * size_px.x * pixel_size) +(x * pixel_size);
				char* px_b = (char*)pixels +(reverse_row * size_px.x * pixel_size) +(reverse_x * pixel_size);
				
				for (uptr i=0; i<pixel_size; ++i) {
					std::swap(px_a[i], px_b[i]);
				}
			}
		}
		if (size_px.y % 2 == 1) { // is odd (has middle row)

			for (uptr x=0; x < size_px.x / 2; ++x) {
				uptr reverse_x = size_px.x -1 -x;

				char* px_a = (char*)pixels +(row * size_px.x * pixel_size) +(x * pixel_size);
				char* px_b = (char*)pixels +(row * size_px.x * pixel_size) +(reverse_x * pixel_size);

				for (uptr i=0; i<pixel_size; ++i) {
					std::swap(px_a[i], px_b[i]);
				}
			}
		}
	}
	
	void copy_rotate_90 (void* src_pixels, void* dst_pixels, uptr pixel_size, iv2 size_px) {
		for (int y=0; y<size_px.y; ++y) {
			for (int x=0; x<size_px.x; ++x) {
				
				int src_x = y;
				int src_y = size_px.x -1 -x;

				char* src_px = (char*)src_pixels +(src_y * size_px.y * pixel_size) +(src_x * pixel_size);
				char* dst_px = (char*)dst_pixels +(y * size_px.y * pixel_size) +(x * pixel_size);

				memcpy(dst_px, src_px, pixel_size);
			}
		}
	}

}
