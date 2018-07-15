#pragma once

namespace engine {
	//
	void APIENTRY ogl_debuproc (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, cstr message, void const* userParam) {

		//if (source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB) return;

		// hiding irrelevant infos/warnings
		switch (id) {
			case 131185: // Buffer detailed info (where the memory lives which is supposed to depend on the usage hint)
						 //case 1282: // using shader that was not compiled successfully
						 //
						 //case 2: // API_ID_RECOMPILE_FRAGMENT_SHADER performance warning has been generated. Fragment shader recompiled due to state change.
						 //case 131218: // Program/shader state performance warning: Fragment shader in program 3 is being recompiled based on GL state.
						 //
						 //			 //case 131154: // Pixel transfer sync with rendering warning
						 //
						 //			 //case 1282: // Wierd error on notebook when trying to do texture streaming
						 //			 //case 131222: // warning with unused shadow samplers ? (Program undefined behavior warning: Sampler object 0 is bound to non-depth texture 0, yet it is used with a program that uses a shadow sampler . This is undefined behavior.), This might just be unused shadow samplers, which should not be a problem
						 //			 //case 131218: // performance warning, because of shader recompiling based on some 'key'
				return;
		}

		cstr src_str = "<unknown>";
		switch (source) {
			case GL_DEBUG_SOURCE_API_ARB:				src_str = "GL_DEBUG_SOURCE_API_ARB";				break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:		src_str = "GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB";		break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:	src_str = "GL_DEBUG_SOURCE_SHADER_COMPILER_ARB";	break;
			case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:		src_str = "GL_DEBUG_SOURCE_THIRD_PARTY_ARB";		break;
			case GL_DEBUG_SOURCE_APPLICATION_ARB:		src_str = "GL_DEBUG_SOURCE_APPLICATION_ARB";		break;
			case GL_DEBUG_SOURCE_OTHER_ARB:				src_str = "GL_DEBUG_SOURCE_OTHER_ARB";				break;
		}

		cstr type_str = "<unknown>";
		switch (source) {
			case GL_DEBUG_TYPE_ERROR_ARB:				type_str = "GL_DEBUG_TYPE_ERROR_ARB";				break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:	type_str = "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB";	break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:	type_str = "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB";	break;
			case GL_DEBUG_TYPE_PORTABILITY_ARB:			type_str = "GL_DEBUG_TYPE_PORTABILITY_ARB";			break;
			case GL_DEBUG_TYPE_PERFORMANCE_ARB:			type_str = "GL_DEBUG_TYPE_PERFORMANCE_ARB";			break;
			case GL_DEBUG_TYPE_OTHER_ARB:				type_str = "GL_DEBUG_TYPE_OTHER_ARB";				break;
		}

		cstr severity_str = "<unknown>";
		switch (severity) {
			case GL_DEBUG_SEVERITY_HIGH_ARB:			severity_str = "GL_DEBUG_SEVERITY_HIGH_ARB";		break;
			case GL_DEBUG_SEVERITY_MEDIUM_ARB:			severity_str = "GL_DEBUG_SEVERITY_MEDIUM_ARB";		break;
			case GL_DEBUG_SEVERITY_LOW_ARB:				severity_str = "GL_DEBUG_SEVERITY_LOW_ARB";			break;
		}

		fprintf(stderr, "OpenGL debug proc: severity: %s src: %s type: %s id: %d  %s\n",
				severity_str, src_str, type_str, id, message);
	}

	struct RectI {
		iv2	low;
		iv2	high;

		inline iv2 get_size () {
			return high -low;
		}
	};

	RectI to_rect (RECT win32_rect) {
		return {	iv2(win32_rect.left,	win32_rect.top),
			iv2(win32_rect.right,	win32_rect.bottom) };
	}

	enum e_vsync_mode {
		VSYNC_ON,
		VSYNC_OFF,
	};

	struct Input {
		
		bool block_mouse = false;
		bool block_keyboard = false;
		bool blocked_by_typing = false;

		iv2	wnd_size_px;

		struct Mousecursor {
			v2		pos_screen; // top-down
			v2		delta_screen;
		} mousecursor;

		v2 mouse_cursor_pos_screen_buttom_up () {
			v2 p = mousecursor.pos_screen;
			return v2(p.x, (flt)wnd_size_px.y -1 -p.y);
		}
		v2 mouse_cursor_pos_screen_buttom_up_pixel_center () {
			return mouse_cursor_pos_screen_buttom_up() + 0.5f;
		}

		struct Mousewheel {
			flt		delta;
		} mousewheel;

		struct Button {
			bool	is_down		: 1;
			bool	went_down	: 1;
			bool	went_up		: 1;
			bool	os_repeat	: 1;
		};

		Button buttons[GLFW_KEY_LAST +1] = {}; // lower 8 indecies are used as mouse button (GLFW_MOUSE_BUTTON_1 - GLFW_MOUSE_BUTTON_8), glfw does not seem to have anything assigned to them

		bool is_down (int glfw_button) const {
			if (block_keyboard) return false;
			return buttons[glfw_button].is_down;
		}
		bool went_down (int glfw_button) const {
			if (block_keyboard) return false;
			return buttons[glfw_button].went_down;
		}
		bool went_up (int glfw_button) const {
			if (block_keyboard) return false;
			return buttons[glfw_button].went_up;
		}
		bool os_repeat (int glfw_button) const {
			if (block_keyboard) return false;
			return buttons[glfw_button].os_repeat;
		}

		bool key_combo (int glfw_mod_key_l, int glfw_mod_key_r, int glfw_key) {
			if (blocked_by_typing) return false; // still trigger ctrl+key shortcut if keyboard is blocked, since key does not interfere with imgui (don't trigger during text typing)

			auto& lc = buttons[glfw_mod_key_l];
			auto& rc = buttons[glfw_mod_key_r];
			auto& key = buttons[glfw_key];

			return	( (lc.is_down ||   rc.is_down) &&  key.is_down ) && // all keys in combo must be down
				(  lc.went_down || rc.went_down || key.went_down ); // but only trigger if one of them went down
		}
		bool ctrl_combo (int glfw_key) {	return key_combo(GLFW_KEY_LEFT_CONTROL, GLFW_KEY_RIGHT_CONTROL, glfw_key); }
		bool alt_combo (int glfw_key) {		return key_combo(GLFW_KEY_LEFT_ALT, GLFW_KEY_RIGHT_ALT, glfw_key); }

		struct Event {

			enum type_e {
				MOUSEMOVE,
				MOUSEWHEEL,
				BUTTON, // Button transition, no repeats
				TYPING,
			};

			type_e	type;

			union {
				struct {
					v2			delta_screen;
				} Mousemove;
				struct {
					v2			delta;
				} Mousewheel;
				struct {
					int			glfw_button;
					bool		went_down;
				} Button;
				struct {
					utf32		codepoint;
				} Typing;
			};

			Event () {} 
			Event (type_e t): type{t} {} 
		};

		std::vector<Event>	events;
	};

	struct Window {

		GLFWwindow*			window = nullptr;

		e_vsync_mode		vsync_mode;

		WINDOWPLACEMENT		win32_windowplacement = {}; // Zeroed
		void get_win32_windowplacement () {
			GetWindowPlacement(glfwGetWin32Window(window), &win32_windowplacement);
		}
		void set_win32_windowplacement () {
			SetWindowPlacement(glfwGetWin32Window(window), &win32_windowplacement);
		}

		bool				is_fullscreen = false;
		iv2					fullscreen_resolution = -1; // -1 => native res

		void set_fullscreen (bool want_fullscreen) {
			if (!is_fullscreen && !want_fullscreen) return; // going from windowed to windowed

			bool was_windowed = !is_fullscreen;

			if (want_fullscreen) {

				if (was_windowed)
					get_win32_windowplacement();

				GLFWmonitor* fullscreen_monitor = nullptr;
				GLFWvidmode const* fullscreen_vidmode = nullptr;
				{
					int count;
					GLFWmonitor** monitors = glfwGetMonitors(&count);

					flt				min_dist = INF;

					iv2 wnd_pos;
					iv2 wnd_sz;
					glfwGetWindowPos(window, &wnd_pos.x,&wnd_pos.y);
					glfwGetWindowSize(window, &wnd_sz.x,&wnd_sz.y);

					v2 wnd_center = ((v2)wnd_pos +(v2)wnd_sz) / 2;

					for (int i=0; i<count; ++i) {
						
						iv2 pos;
						glfwGetMonitorPos(monitors[i], &pos.x,&pos.y);

						auto* mode = glfwGetVideoMode(monitors[i]);

						v2 monitor_center = ((v2)pos +(v2)iv2(mode->width,mode->height)) / 2;
						
						v2 offs = wnd_center -monitor_center;
						flt dist = length(offs);

						if (dist < min_dist) {
							fullscreen_monitor = monitors[i];
							min_dist = dist;

							fullscreen_vidmode = mode;
						}
					}

					assert(fullscreen_monitor);
				}

				iv2 res = fullscreen_resolution;
				if (res.x < 0)
					res = iv2(fullscreen_vidmode->width, fullscreen_vidmode->height);

				glfwSetWindowMonitor(window, fullscreen_monitor, 0, 0, res.x,res.y, GLFW_DONT_CARE);

			} else { // want windowed
				assert(!was_windowed);

				auto r = to_rect(win32_windowplacement.rcNormalPosition);
				auto sz = r.get_size();
				glfwSetWindowMonitor(window, NULL, r.low.x,r.low.y, sz.x,sz.y, GLFW_DONT_CARE);

				set_win32_windowplacement(); // Still refuses to return to maximized mode ??

				//if (win32_windowplacement.showCmd == SW_MAXIMIZE) {
				//	glfwMaximizeWindow(window); // Still refuses to return to maximized mode ????
				//}
			}

			set_vsync(vsync_mode); // seemingly need to reset vsync sometimes when toggling fullscreen mode

			is_fullscreen = want_fullscreen;
		}
		void toggle_fullscreen () {
			set_fullscreen(!is_fullscreen);
		}

		void save_window_positioning () {

			if (is_fullscreen) {
				// keep window positioning that we got when we switched to fullscreen
			} else {
				get_win32_windowplacement();
			}

			if (!write_fixed_size_binary_file("saves/window_placement.bin", &win32_windowplacement, sizeof(win32_windowplacement))) {
				fprintf(stderr, "Could not save window_placement to saves/window_placement.bin, window position and size won't be restored on the next launch of this app.");
			}
		}
		bool load_window_positioning () {
			return load_fixed_size_binary_file("saves/window_placement.bin", &win32_windowplacement, sizeof(win32_windowplacement));
		}

		Input inp;

		static void glfw_error_proc (int err, cstr msg) {
			fprintf(stderr, "GLFW Error! 0x%x '%s'\n", err, msg);
		}

		void create (std::string const& title, iv2 default_size, e_vsync_mode vsync_mode) {
			assert(window == nullptr);
			
			this->vsync_mode = vsync_mode;

			glfwSetErrorCallback(glfw_error_proc);

			assert(glfwInit() != 0); // do not support multiple windows at the moment

			bool placement_loaded = load_window_positioning();

			is_fullscreen = false; // always start in windowed mode

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

			bool gl_vaos_required = true;

			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

			s32v2 size = placement_loaded ? to_rect(win32_windowplacement.rcNormalPosition).get_size() : default_size;

			window = glfwCreateWindow(size.x,size.y, title.c_str(), NULL, NULL);

			glfwSetWindowUserPointer(window, this);

			if (placement_loaded) {
				set_win32_windowplacement();
			}

			glfwSetCursorPosCallback(window,	glfw_mouse_pos_event);
			glfwSetMouseButtonCallback(window,	glfw_mouse_button_event);
			glfwSetScrollCallback(window,		glfw_mouse_scroll);
			glfwSetKeyCallback(window,			glfw_key_event);
			glfwSetCharModsCallback(window,		glfw_char_event);

			glfwMakeContextCurrent(window);

			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

			if (GLAD_GL_ARB_debug_output) {
				glDebugMessageCallbackARB(ogl_debuproc, 0);
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

				// without GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB ogl_debuproc needs to be thread safe
			}

			GLuint vao; // one global vao for everything

			if (gl_vaos_required) {
				glGenVertexArrays(1, &vao);
				glBindVertexArray(vao);
			}

			set_vsync(vsync_mode);

		}
		void close () {
			save_window_positioning();

			glfwDestroyWindow(window);
			glfwTerminate();

			window = nullptr;
		}

		~Window () {
			if (window)
				close();
		}

		Input& poll_input (bool mouse_cursor_enabled) {

			inp.mousecursor.delta_screen = 0;
			inp.mousewheel.delta = 0;
			for (auto& b : inp.buttons) {
				b.went_down = false;
				b.went_up = false;
				b.os_repeat = false;
			}

			inp.events.clear();

			glfwSetInputMode(window, GLFW_CURSOR, mouse_cursor_enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

			glfwPollEvents();

			glfwGetFramebufferSize(window, &inp.wnd_size_px.x,&inp.wnd_size_px.y);

			{
				double x,y;
				glfwGetCursorPos(window, &x, &y);
				inp.mousecursor.pos_screen = v2((flt)x,(flt)y);
			}

			if (inp.buttons[GLFW_KEY_F11].went_down) {
				toggle_fullscreen();
			}

			return inp;
		}

		bool wants_to_close () {
			return glfwWindowShouldClose(window) != 0; // also handles ALT+F4
		}

		void set_vsync (e_vsync_mode vsync_mode) {

			int interval = 1;

			if (vsync_mode == VSYNC_ON)
				interval = 1; // -1 swap interval technically requires a extension check which requires wgl loading in addition to gl loading
			else if (vsync_mode == VSYNC_OFF)
				interval = 0;
			else assert(not_implemented);

			glfwSwapInterval(interval);

			this->vsync_mode = vsync_mode;
		}

		void swap_buffers () {
			glfwSwapBuffers(window);
		}

		// input
		static void button_event (GLFWwindow* window, int button, int action, int mods) {
			Input& inp = ((Window*)glfwGetWindowUserPointer(window))->inp;

			bool went_down = action == GLFW_PRESS;
			bool went_up = action == GLFW_RELEASE;
			bool repeat = action == GLFW_REPEAT;

			if (!(button >= 0 && button < ARRLEN(Input::buttons)))
				return;

			assert(went_down || went_up || repeat);
			if (!(went_down || went_up || repeat))
				return;

			inp.events.push_back({ Input::Event::BUTTON });
			inp.events.back().Button.glfw_button = button;
			inp.events.back().Button.went_down = went_down;

			inp.buttons[button].is_down =	went_down || repeat;
			inp.buttons[button].went_down =	went_down;
			inp.buttons[button].went_up =	went_up;
			inp.buttons[button].os_repeat =	repeat;
		}

		static void glfw_mouse_pos_event (GLFWwindow* window, double xpos, double ypos) {
			Input& inp = ((Window*)glfwGetWindowUserPointer(window))->inp;
			
			v2 new_pos = v2((flt)xpos,(flt)ypos);
			static v2 prev_pos;
			
			static bool first_call = true;
			if (!first_call) {
				
				v2 diff = new_pos -prev_pos;
				inp.mousecursor.delta_screen += diff;

			}
			first_call = false;

			prev_pos = new_pos;
		}
		static void glfw_mouse_button_event (GLFWwindow* window, int button, int action, int mods) {
			button_event(window, button, action, mods);
		}
		static void glfw_mouse_scroll (GLFWwindow* window, double xoffset, double yoffset) {
			Input& inp = ((Window*)glfwGetWindowUserPointer(window))->inp;

			inp.mousewheel.delta += (flt)yoffset;

			inp.events.push_back({ Input::Event::MOUSEWHEEL });
			inp.events.back().Mousewheel.delta = (flt)yoffset;
		}
		static void glfw_key_event (GLFWwindow* window, int key, int scancode, int action, int mods) {
			button_event(window, key, action, mods);
		}
		static void glfw_char_event (GLFWwindow* window, unsigned int codepoint, int mods) {
			Input& inp = ((Window*)glfwGetWindowUserPointer(window))->inp;

			inp.events.push_back({ Input::Event::TYPING });
			inp.events.back().Typing.codepoint = (utf32)codepoint;
		}

	};

	struct Delta_Time_Measure {

		u64 QPC_freq;
		u64 QPC_prev_frame_end;

		flt begin () {
			assert(QueryPerformanceCounter((LARGE_INTEGER*)&QPC_prev_frame_end));
			assert(QueryPerformanceFrequency((LARGE_INTEGER*)&QPC_freq));

			return 0; // zero dt on first frame, timestep calc should be able to handle this
		}

		flt frame () {
			u64 now;
			assert(QueryPerformanceCounter((LARGE_INTEGER*)&now));

			u64 dt_ticks = now -QPC_prev_frame_end;

			flt dt = (flt)( (f64)dt_ticks / (f64)QPC_freq );

			QPC_prev_frame_end = now;
			return dt;
		}

	};

	//
}
