
#include "engine.hpp"
using namespace engine;

std::string shad_default_2d =			"shaders/default_2d";
std::string shad_default_3d =			"shaders/default_3d";

Shader const* _current_used_shader = nullptr; // for debugging

void gl_set_uniform (GLint loc, flt val) {		glUniform1f(	loc, val); }
void gl_set_uniform (GLint loc, fv2 val) {		glUniform2fv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, fv3 val) {		glUniform3fv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, fv4 val) {		glUniform4fv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, s32 val) {		glUniform1i(	loc, val); }
void gl_set_uniform (GLint loc, s32v2 val) {	glUniform2iv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, s32v3 val) {	glUniform3iv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, s32v4 val) {	glUniform4iv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, fm2 val) {		glUniformMatrix2fv(	loc, 1, GL_FALSE, &val.arr[0].x); }
void gl_set_uniform (GLint loc, fm3 val) {		glUniformMatrix3fv(	loc, 1, GL_FALSE, &val.arr[0].x); }
void gl_set_uniform (GLint loc, fm4 val) {		glUniformMatrix4fv(	loc, 1, GL_FALSE, &val.arr[0].x); }
void gl_set_uniform (GLint loc, bool val) {		glUniform1i(	loc, val ? 1 : 0); }


template <typename T> void set_uniform (Shader* shad, std::string const& name, T val) {
	assert(_current_used_shader == shad);

	GLint loc = glGetUniformLocation(shad->get_prog_handle(), name.c_str());
	if (loc >= 0) gl_set_uniform(loc, val);
}

void use_shader (Shader* shad) {
	_current_used_shader = shad;
	glUseProgram(shad->get_prog_handle());
}

struct Uniform_Sharer {
	enum type_e {
		FLT, FV2, FV3, FV4,
		INT_, IV2, IV3, IV4,

		U8V4,

		MAT2, MAT3, MAT4,

		BOOL,
	};
	struct Uniform_Val {
		type_e	type;
		union {
			f32		flt_;
			fv2		fv2_;
			fv3		fv3_;
			fv4		fv4_;

			s32		int_;
			s32v2	iv2_;
			s32v3	iv3_;
			s32v4	iv4_;

			fm2		fm2_;
			fm3		fm3_;
			fm4		fm4_;

			bool	bool_;
		};

		Uniform_Val () {}

		void set (f32	val) { type = FLT ;	flt_  = val;	}
		void set (fv2	val) { type = FV2 ;	fv2_  = val;	}
		void set (fv3	val) { type = FV3 ;	fv3_  = val;	}
		void set (fv4	val) { type = FV4 ;	fv4_  = val;	}
		void set (s32	val) { type = INT_;	int_  = val;	}
		void set (s32v2	val) { type = IV2 ;	iv2_  = val;	}
		void set (s32v3	val) { type = IV3 ;	iv3_  = val;	}
		void set (s32v4	val) { type = IV4 ;	iv4_  = val;	}
		void set (m2	val) { type = MAT2;	fm2_  = val;	}
		void set (m3	val) { type = MAT3;	fm3_  = val;	}
		void set (m4	val) { type = MAT4;	fm4_  = val;	}
		void set (bool	val) { type = BOOL;	bool_ = val;	}
	};
	static void gl_set_uniform (GLint loc, Uniform_Val const& val) {
		switch (val.type) {
			case FLT :	::gl_set_uniform(loc, val.flt_ );	break;
			case FV2 :	::gl_set_uniform(loc, val.fv2_ );	break;
			case FV3 :	::gl_set_uniform(loc, val.fv3_ );	break;
			case FV4 :	::gl_set_uniform(loc, val.fv4_ );	break;
			case INT_:	::gl_set_uniform(loc, val.int_ );	break;
			case IV2 :	::gl_set_uniform(loc, val.iv2_ );	break;
			case IV3 :	::gl_set_uniform(loc, val.iv3_ );	break;
			case IV4 :	::gl_set_uniform(loc, val.iv4_ );	break;
			case MAT2:	::gl_set_uniform(loc, val.fm2_ );	break;
			case MAT3:	::gl_set_uniform(loc, val.fm3_ );	break;
			case MAT4:	::gl_set_uniform(loc, val.fm4_ );	break;
			case BOOL:	::gl_set_uniform(loc, val.bool_);	break;
			default: assert(not_implemented);
		}
	}

	std::unordered_map<std::string, Uniform_Val>	shared_uniforms;

	void set_shared_uniform (std::string const& share_name, std::string const& uniform_name, Uniform_Val val) {
		shared_uniforms[share_name+ '_' +uniform_name] = val;
	}

	void set_shared_uniforms_for_shader (Shader* shad) {
		assert(_current_used_shader == shad);
		
		for (auto& su : shared_uniforms) {
			GLint loc = glGetUniformLocation(shad->get_prog_handle(), su.first.c_str());
			if (loc != -1) gl_set_uniform(loc, su.second);
		}
	}
};
Uniform_Sharer uniform_sharer;

/*
	Set uniform that all shaders can access
	This uniform even applies if the shader does not exist yet (i do lazy shader loading)

	Could be optimized to use Uniform Buffer Object (share_name would be the name of a UBO and uniform_name would be a member of that structure)
*/
template <typename T> void set_shared_uniform (std::string const& share_name, std::string const& uniform_name, T val) {
	Uniform_Sharer::Uniform_Val unionized;
	unionized.set(val);
	uniform_sharer.set_shared_uniform(share_name, uniform_name, unionized);
}

// gl abstraction utility functions
Shader* _get_shader (std::string const& shader) {
	Shader*	shad = shader_manager.get_shader(shader);
	return shad;
}
Shader* use_shader (std::string const& shader) {
	Shader*	shad = _get_shader(shader);
	if (shad) {
		use_shader(shad);
		uniform_sharer.set_shared_uniforms_for_shader(shad);
	}
	return shad;
}

void bind_vertex_data (VBO const& vbo, Data_Vertex_Layout const& vertex_layout, Shader const& shad) {
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo.get_handle());

	for (auto& attr : vertex_layout.attributes) {
		
		auto loc = glGetAttribLocation(shad.get_prog_handle(), attr.name.c_str());
		if (loc < 0)
			continue;

		glEnableVertexAttribArray(loc);

		switch (attr.type) {
			case FLT:			glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE,			vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case FV2:			glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE,			vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case FV3:			glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE,			vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case FV4:			glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE,			vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;

			case INT_:			glVertexAttribIPointer(loc, 1, GL_INT,						vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case IV2:			glVertexAttribIPointer(loc, 2, GL_INT,						vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case IV3:			glVertexAttribIPointer(loc, 3, GL_INT,						vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case IV4:			glVertexAttribIPointer(loc, 4, GL_INT,						vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;

			case U8V4_AS_FV4:	glVertexAttribPointer(loc, 4, GL_UNSIGNED_BYTE,	GL_TRUE,	vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
		}

	}

}

VBO stream_vertex_data (void const* vertex_data, int vertex_size) {
	VBO vbo = mesh_manager.stream_vertex_data(vertex_data, vertex_size);
	return vbo;
}

void use_vertex_data (Shader const& shad, Data_Vertex_Layout const& vertex_layout, VBO const& vbo) {
	bind_vertex_data(vbo, vertex_layout, shad);
}
template <typename T> VBO use_stream_vertex_data (Shader const& shad, std::vector<T> const& vertex_data) {
	assert((int)sizeof(T) == T::layout.vertex_size);
	
	VBO vbo = stream_vertex_data(vertex_data.data(), (int)vertex_data.size() * T::layout.vertex_size);
	use_vertex_data(shad, T::layout, vbo);
	return vbo;
}

void draw_triangles (Shader const& shad, Data_Vertex_Layout const& vertex_layout, VBO const& vbo, int first_vertex, int vertex_count) {
	glDrawArrays(GL_TRIANGLES, first_vertex, (GLsizei)vertex_count);
}

template <typename T> void draw_stream_triangles (Shader const& shad, std::vector<T> const& vertex_data, int first_vertex=0, int vertex_count=-1) {
	auto vbo = use_stream_vertex_data(shad, vertex_data);
	draw_triangles(shad, T::layout, vbo, first_vertex, vertex_count < 0 ? (int)vertex_data.size() : vertex_count);
}

template <typename T> void draw_stream_triangles (std::string const& shader, std::vector<T> const& vertex_data, int first_vertex=0, int vertex_count=-1) {
	auto shad = use_shader(shader);
	if (shad)
		draw_stream_triangles(*shad, vertex_data, first_vertex, vertex_count);
}

void bind_texture (Shader* shad, std::string const& uniform_name, int tex_unit, Texture2D const& tex) {
	assert(_current_used_shader == shad);
	
	auto loc = glGetUniformLocation(shad->get_prog_handle(), uniform_name.c_str());
	if (loc >= 0) {
		glUniform1i(loc, tex_unit);

		glActiveTexture(GL_TEXTURE0 +tex_unit);
		glBindTexture(GL_TEXTURE_2D, tex.get_handle());
	}
}

NOINLINE hm calc_cam_to_world (v3 cam_pos_world, v3 cam_altazimuth) {
	quat rot = rotateQ_Z(cam_altazimuth.x) * rotateQ_X(cam_altazimuth.y) * rotateQ_Z(cam_altazimuth.z);
	return translateH(cam_pos_world) * hm(convert_to_m3(rot));
}
NOINLINE hm calc_world_to_cam (v3 cam_pos_world, v3 cam_altazimuth) {
	quat rot = rotateQ_Z(-cam_altazimuth.z) * rotateQ_X(-cam_altazimuth.y) * rotateQ_Z(-cam_altazimuth.x); // rotate world around camera
	return hm(convert_to_m3(rot)) * translateH(-cam_pos_world);
}
m4 calc_perspective_matrix (flt vfov, flt clip_near, flt clip_far, v2 screen_size) {

	f32 aspect_w_over_h = screen_size.x / screen_size.y;

	v2 frustrum_scale = tan(vfov * 0.5f);
	frustrum_scale.x *= aspect_w_over_h;

	v2 frustrum_scale_inv = 1 / frustrum_scale;

	f32 temp = clip_near -clip_far;

	f32 x = frustrum_scale_inv.x;
	f32 y = frustrum_scale_inv.y;
	f32 a = (clip_far +clip_near) / temp;
	f32 b = (2 * clip_far * clip_near) / temp;

	return m4::rows(	x, 0, 0, 0,
						0, y, 0, 0,
						0, 0, a, b,
						0, 0,-1, 0 );
}

struct Camera {
	v3		pos_world = v3(0.1f, -2, +1);

	v3		altazimuth = v3(0,deg(50),0); // x: azimuth (along horizon) y: altitude (up/down) z: roll
	
	flt		vfov = deg(75);

	flt		clip_near = 1.0f / 16;
	flt		clip_far = 1024;

	void control (Input const& inp, flt dt) {
		
		flt roll_vel = deg(45); // ang / sec

		int roll_dir = 0;
		if (inp.buttons['Q'].is_down) roll_dir -= 1;
		if (inp.buttons['E'].is_down) roll_dir += 1;

		altazimuth.z += (flt)-roll_dir * roll_vel * dt;

		v2 mouselook_sensitivity = deg(0.12f); // ang / mouse_move_px
		
		if (inp.buttons[GLFW_MOUSE_BUTTON_RIGHT].is_down) {
			altazimuth.x -=  inp.mousecursor.delta_screen.x * mouselook_sensitivity.x;
			altazimuth.y += -inp.mousecursor.delta_screen.y * mouselook_sensitivity.y; // delta_screen is top down
		}

		altazimuth.x = mod_range(altazimuth.x, deg(-180), deg(+180));
		altazimuth.y = clamp(altazimuth.y, deg(0), deg(180));
		altazimuth.z = mymod(altazimuth.z, deg(360));

		v3 move_vel = inp.buttons[GLFW_KEY_LEFT_SHIFT].is_down ? 3.0f : 1.0f;

		iv3 move_dir = 0;
		if (inp.buttons['S'].is_down)					move_dir.z += 1;
		if (inp.buttons['W'].is_down)					move_dir.z -= 1;
		if (inp.buttons['A'].is_down)					move_dir.x -= 1;
		if (inp.buttons['D'].is_down)					move_dir.x += 1;
		if (inp.buttons[GLFW_KEY_LEFT_CONTROL].is_down)	move_dir.y -= 1;
		if (inp.buttons[' '].is_down)					move_dir.y += 1;

		pos_world += (calc_cam_to_world(pos_world, altazimuth).m3() * (normalize_or_zero((v3)move_dir) * move_vel)) * dt;
	}
};

int main () {
	
	Window wnd;
	wnd.create("3D", iv2(1280,720), VSYNC_ON);
	
	Delta_Time_Measure dt_measure;
	f32 dt = dt_measure.begin();

	glEnable(GL_FRAMEBUFFER_SRGB);

	for (;;) {
		
		auto inp = wnd.poll_input();

		if (wnd.wants_to_close())
			break;

		rgbf clear_color_lrgb = to_linear(col8_to_colf( rgb8(40,40,40) ));

		glViewport(0,0, inp.wnd_size_px.x,inp.wnd_size_px.y);

		#if 1
		glClearColor(clear_color_lrgb.x,clear_color_lrgb.y,clear_color_lrgb.z,255);
		glClear(GL_COLOR_BUFFER_BIT);
		#endif

		set_shared_uniform("common", "screen_size", (v2)inp.wnd_size_px);
		set_shared_uniform("common", "mcursor_pos", inp.mouse_cursor_pos_screen_buttom_up_pixel_center());

		static bool draw_wireframe = false;
		if (inp.buttons['P'].went_down)
			draw_wireframe = !draw_wireframe;

		set_shared_uniform("wireframe", "enable", draw_wireframe);

		static Camera cam;
		//saveable("3d_camera", &cam);

		cam.control(inp, dt);

		hm	world_to_cam = calc_world_to_cam(cam.pos_world, cam.altazimuth);
		m4	cam_to_clip = calc_perspective_matrix(cam.vfov, cam.clip_near, cam.clip_far, (v2)inp.wnd_size_px);
		
		set_shared_uniform("_3d_view", "world_to_cam", world_to_cam.m4());
		set_shared_uniform("_3d_view", "cam_to_clip", cam_to_clip);
		
		std::vector<Default_Vertex_3d> quad;

		for (auto p : { v2(0,0),v2(1,0),v2(1,1), v2(1,1),v2(0,0),v2(0,1) }) {
			Default_Vertex_3d v;
			v.pos_model = v3(p -0.5f, 0);
			v.uv = p;
			quad.push_back(v);
		}

		for (int y=-5; y<5; ++y) {
			for (int x=-5; x<5; ++x) {
				
				auto s = use_shader(shad_default_3d);
				if (s) {
					set_uniform(s, "model_to_world", translate4(v3((v2)iv2(y,x) * 1.2f, 0)));
					
					static int cycle = 0;
					//if (inp.buttons['C'].went_down)
					//	cycle++;
					cycle = (int)(glfwGetTime() / 2);

					cycle %= 3;

					Texture2D tmp;

					if (		cycle == 0 ) {
						rgba8 pixels[16][16];
						for (int y=0; y<16; ++y) {
							for (int x=0; x<16; ++x) {
								pixels[y][x] = BOOL_XOR(BOOL_XOR(x % 2, y % 2), mymod((flt)glfwGetTime(), 1.0f) < 0.5f) ? rgba8(220,150,150,255) : rgba8(255);
							}
						}
				
						tmp = stream_texture(pixels, iv2(16,16), PF_SRGBA8, NO_MIPMAPS, FILTER_NEAREST);
						bind_texture(s, "tex", 0, tmp);
					} else if ( cycle == 1 ) {
						bind_texture(s, "tex", 0, *get_texture("dab.png", PF_SRGBA8, NO_MIPMAPS));
					} else if ( cycle == 2 ) {
						static Texture2D white_tex = stream_texture(&white, iv2(1,1), PF_SRGBA8, NO_MIPMAPS, FILTER_NEAREST);
			
						bind_texture(s, "tex", 0, white_tex);
					}

					draw_stream_triangles(*s, quad);
				}
			}
		}

		#if 0
		auto s = use_shader("skybox_cubemap_gen");

		auto render_cubemap = [&] () {
			struct Vertex {
				v2		pos_screen;
				v3		dir_cubemap;
			};
			const Data_Vertex_Layout layout = { (int)sizeof(Vertex), {
				{ "pos_screen",		FV2,	(int)offsetof(Vertex, pos_screen) },
				{ "dir_cubemap",	FV3,	(int)offsetof(Vertex, dir_cubemap) },
			}};

			static TextureCube skybox = alloc_texture_cube(iv2(256,256), PF_LRGBF16, NO_MIPMAPS);
			static auto fbo = draw_to_texture(&skybox);

			static std::vector<Vertex> faces = {
				// face 1
				{ v2(-1,-1), v3(+1,+1,+1) },
				{ v2(+1,-1), v3(+1,+1,+1) },
				{ v2(+1,+1), v3(+1,+1,+1) },
				{ v2(+1,+1), v3(+1,+1,+1) },
				{ v2(-1,-1), v3(+1,+1,+1) },
				{ v2(-1,+1), v3(+1,+1,+1) },
				// ...
			};


		}
		#endif
	
		wnd.swap_buffers();
		
		dt = dt_measure.frame();
	}

	return 0;
}
