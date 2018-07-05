#pragma once

#define _CRT_SECURE_NO_WARNINGS

// language includes
#include <string>
#include <vector>
#include <cassert>

#include <memory>
using std::unique_ptr;
using std::make_unique;
using std::shared_ptr;
using std::make_shared;
//

// library includes
#include "deps/glad/glad.c"

#include "deps/dear_imgui/imgui.h"

#include "deps/glfw-3.2.1.bin.WIN64/include/GLFW/glfw3.h"

#pragma push_macro("APIENTRY")

#define GLFW_EXPOSE_NATIVE_WIN32 1
#include "deps/glfw-3.2.1.bin.WIN64/include/GLFW/glfw3native.h"

#pragma pop_macro("APIENTRY")

#include "windows.h"
#include "Shlobj.h"
//

// my includes
#include "basic_typedefs.hpp"
#include "vector.hpp"
#include "simple_file_io.hpp"
#include "prints.hpp"

static constexpr bool not_implemented = false; // use like: assert(not_implemented)
//

// Use 32bit float as float type by default, if everything goes well you could switch to doubles by simply changing the typedef here, but i never tested this, NOTE that OpenGL does not support doubles
namespace engine {
	typedef f32 flt;

	typedef fv2 v2;
	typedef fv3 v3;
	typedef fv4 v4;

	typedef s32v2 iv2;
	typedef s32v3 iv3;
	typedef s32v4 iv4;

	typedef fm2 m2;
	typedef fm3 m3;
	typedef fm4 m4;
	typedef fhm hm;

	typedef fquat quat;
}

#include "colors.hpp"

namespace engine {
	constexpr srgba8 black = 0;
	constexpr lrgba lblack = 0;

	constexpr srgba8 white = 255;
	constexpr lrgba lwhite = 1;

	constexpr v3	identity_normal = v3(0.5f,0.5f,1);
}

#include "engine_window.hpp"

#include "engine_vertex_layout.hpp"

#include "engine_mesh_manager.hpp"
#include "engine_shader_manager.hpp"
#include "engine_texture.hpp"

#include "engine_draw.hpp"
#include "engine_draw_to_texture.hpp"

#include "engine_dear_imgui.hpp"
