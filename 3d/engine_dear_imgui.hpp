#pragma once

#include "deps/dear_imgui/imgui.cpp"
#include "deps/dear_imgui/imgui_draw.cpp"
#include "deps/dear_imgui/imgui_demo.cpp"

namespace ImGui {
	IMGUI_API void Value (const char* prefix, engine::fv2 v) {
		Text("%s: %.2f, %.2f", prefix, v.x,v.y);
	}
	IMGUI_API void Value (const char* prefix, engine::s32v2 v) {
		Text("%s: %d, %d", prefix, v.x,v.y);
	}
	IMGUI_API void Value (const char* prefix, engine::u64 i) {
		Text("%s: %lld", prefix, i);
	}
	IMGUI_API void Value_Bytes (const char* prefix, engine::u64 i) {
		using namespace engine;

		cstr unit;
		f32 val = (f32)i;
		if (		i < ((u64)1024) ) {
			val /= (u64)1;
			unit = "B";
		} else if (	i < ((u64)1024*1024) ) {
			val /= (u64)1024;
			unit = "KB";
		} else if (	i < ((u64)1024*1024*1024) ) {
			val /= (u64)1024*1024;
			unit = "MB";
		} else if (	i < ((u64)1024*1024*1024*1024) ) {
			val /= (u64)1024*1024*1024;
			unit = "GB";
		} else if (	i < ((u64)1024*1024*1024*1024*1024) ) {
			val /= (u64)1024*1024*1024*1024;
			unit = "TB";
		} else {
			val /= (u64)1024*1024*1024*1024*1024;
			unit = "PB";
		}
		Text("%s: %.2f %s", prefix, val, unit);
	}

	IMGUI_API bool InputText_str (const char* label, std::string* s, ImGuiInputTextFlags flags=0, ImGuiTextEditCallback callback=nullptr, void* user_data=nullptr) {
		using namespace engine;

		int cur_length = (int)s->size();
		s->resize(1024);
		(*s)[ min(cur_length, (int)s->size()-1) ] = '\0'; // is this guaranteed to work?

		bool ret = InputText(label, &(*s)[0], (int)s->size(), flags, callback, user_data);

		s->resize( strlen(&(*s)[0]) );

		return ret;
	}

	IMGUI_API void TextBox (const char* label, std::string s) { // (pseudo) read-only text box, (can still be copied out of, which is nice, this also allows proper layouting)
		InputText_str(label, &s);
	}
}

#include "image_processing.hpp"

namespace engine {
	Texture2D imgui_atlas;

	void begin_imgui (Input* inp, flt dt, bool gui_input_enabled, bool imgui_enabled) { // imgui_enabled: hide imgui, we still call imgui calls, so it still needs to work like usually
		auto init = [] () {
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();

			for (int i=0; i<ARRLEN(io.KeyMap); ++i)
				io.KeyMap[i] = i;

			{
				typedef srgba8 pixel;

				s32v2	size;
				u8*		pixels; // Imgui mallocs and frees the pixels
				io.Fonts->GetTexDataAsRGBA32(&pixels, &size.x,&size.y);

				//flip_vertical_inplace(pixels, size.x * sizeof(pixel), size.y);

				imgui_atlas = upload_texture(pixels,size, { PF_SRGBA8, NO_MIPMAPS, FILTER_LINEAR });

				io.Fonts->TexID = (void*)&imgui_atlas;
			}

			return true;
		};
		static bool dummy = init();

		ImGuiIO& io = ImGui::GetIO();
		
		inp->block_keyboard =		io.WantCaptureKeyboard; // when window is in focus (actived by clicking)
		inp->block_mouse =			io.WantCaptureMouse; // correctly handles dragging onto and off of the imgui windows
		inp->blocked_by_typing =	io.WantTextInput;

		bool use_input = gui_input_enabled && imgui_enabled;

		if (!use_input) {
			for (auto& kd : io.KeysDown)
				kd = false;

			io.KeyCtrl =	false;
			io.KeyShift =	false;
			io.KeyAlt =		false;
			io.KeySuper =	false;

			io.MousePos.x =		-1;
			io.MousePos.y =		-1;
			io.MouseDown[0] =	false;
			io.MouseDown[1] =	false;
			io.MouseWheel =		0;

		} else {

			io.KeysDown[ ImGuiKey_Tab			] = inp->buttons[ GLFW_KEY_TAB			].is_down;
			io.KeysDown[ ImGuiKey_LeftArrow		] = inp->buttons[ GLFW_KEY_LEFT			].is_down;
			io.KeysDown[ ImGuiKey_RightArrow	] = inp->buttons[ GLFW_KEY_RIGHT		].is_down;
			io.KeysDown[ ImGuiKey_UpArrow		] = inp->buttons[ GLFW_KEY_UP			].is_down;
			io.KeysDown[ ImGuiKey_DownArrow		] = inp->buttons[ GLFW_KEY_DOWN			].is_down;
			io.KeysDown[ ImGuiKey_PageUp		] = inp->buttons[ GLFW_KEY_PAGE_UP		].is_down;
			io.KeysDown[ ImGuiKey_PageDown		] = inp->buttons[ GLFW_KEY_PAGE_DOWN	].is_down;
			io.KeysDown[ ImGuiKey_Home			] = inp->buttons[ GLFW_KEY_HOME			].is_down;
			io.KeysDown[ ImGuiKey_End			] = inp->buttons[ GLFW_KEY_END			].is_down;
			io.KeysDown[ ImGuiKey_Insert		] = inp->buttons[ GLFW_KEY_INSERT		].is_down;
			io.KeysDown[ ImGuiKey_Delete		] = inp->buttons[ GLFW_KEY_DELETE		].is_down;
			io.KeysDown[ ImGuiKey_Backspace		] = inp->buttons[ GLFW_KEY_BACKSPACE	].is_down;
			io.KeysDown[ ImGuiKey_Space			] = inp->buttons[ GLFW_KEY_SPACE		].is_down;
			io.KeysDown[ ImGuiKey_Enter			] = inp->buttons[ GLFW_KEY_ENTER		].is_down;
			io.KeysDown[ ImGuiKey_Escape		] = inp->buttons[ GLFW_KEY_ESCAPE		].is_down;
			io.KeysDown[ ImGuiKey_A				] = inp->buttons[ GLFW_KEY_A			].is_down;
			io.KeysDown[ ImGuiKey_C				] = inp->buttons[ GLFW_KEY_C			].is_down;
			io.KeysDown[ ImGuiKey_V				] = inp->buttons[ GLFW_KEY_V			].is_down;
			io.KeysDown[ ImGuiKey_X				] = inp->buttons[ GLFW_KEY_X			].is_down;
			io.KeysDown[ ImGuiKey_Y				] = inp->buttons[ GLFW_KEY_Y			].is_down;
			io.KeysDown[ ImGuiKey_Z				] = inp->buttons[ GLFW_KEY_Z			].is_down;

			io.KeyCtrl =	inp->buttons[ GLFW_KEY_LEFT_CONTROL	].is_down	|| inp->buttons[ GLFW_KEY_RIGHT_CONTROL	].is_down;
			io.KeyShift =	inp->buttons[ GLFW_KEY_LEFT_SHIFT	].is_down	|| inp->buttons[ GLFW_KEY_RIGHT_SHIFT	].is_down;
			io.KeyAlt =		inp->buttons[ GLFW_KEY_LEFT_ALT		].is_down	|| inp->buttons[ GLFW_KEY_RIGHT_ALT		].is_down;
			io.KeySuper =	inp->buttons[ GLFW_KEY_LEFT_SUPER	].is_down	|| inp->buttons[ GLFW_KEY_RIGHT_SUPER	].is_down;

			for (auto& e : inp->events) {
				if (e.type == Input::Event::TYPING)
					io.AddInputCharacter((ImWchar)e.Typing.codepoint);
			}

			io.MousePos.x =		inp->mousecursor.pos_screen.x;
			io.MousePos.y =		inp->mousecursor.pos_screen.y;
			io.MouseDown[0] =	inp->buttons[GLFW_MOUSE_BUTTON_LEFT].is_down;
			io.MouseDown[1] =	inp->buttons[GLFW_MOUSE_BUTTON_RIGHT].is_down;
			io.MouseWheel =		inp->mousewheel.delta;
		}

		io.DisplaySize.x = (flt)inp->wnd_size_px.x;
		io.DisplaySize.y = (flt)inp->wnd_size_px.y;
		io.DeltaTime =		dt;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::NewFrame();
	}

	void end_imgui (iv2 wnd_size_px, bool imgui_enabled) { // expect viewport to be set
		if (!imgui_enabled) {
			ImGui::EndFrame();
			return;
		}

		ImGui::Render();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_SCISSOR_TEST);

		ImDrawData* draw_data = ImGui::GetDrawData();

		static VBO vbo = VBO::generate();
		static EBO ebo = EBO::generate();

		struct Imgui_Vertex_2d {
			v2		pos_screen; // top down coords
			v2		uv				= 0.5f;
			srgba8	col_srgba		= white;
		};
		const Data_Vertex_Layout layout = { (int)sizeof(Imgui_Vertex_2d), {
			{ "pos_screen",			FV2,	(int)offsetof(Imgui_Vertex_2d, pos_screen) },
			{ "uv",					FV2,	(int)offsetof(Imgui_Vertex_2d, uv) },
			{ "col_srgba",			RGBA8,	(int)offsetof(Imgui_Vertex_2d, col_srgba) }
		}};


		for (int n = 0; n < draw_data->CmdListsCount; n++) {
			auto* cmd_list = draw_data->CmdLists[n];

			ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;  // vertex buffer generated by ImGui
			ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;   // index buffer generated by ImGui

			auto vertex_size = cmd_list->VtxBuffer.size() * sizeof(ImDrawVert);
			auto index_size = cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx);

			glBindBuffer(GL_ARRAY_BUFFER, vbo.get_handle());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.get_handle());

			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertex_size, NULL, GL_DYNAMIC_DRAW); // Buffer orphan on reupload
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertex_size, vtx_buffer, GL_DYNAMIC_DRAW);

			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)index_size, NULL, GL_DYNAMIC_DRAW); // Buffer orphan on reupload
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)index_size, idx_buffer, GL_DYNAMIC_DRAW);

			ImDrawIdx* cur_idx_buffer = idx_buffer;

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback) {
					pcmd->UserCallback(cmd_list, pcmd);
				} else {
					
					if (pcmd->TextureId != (ImTextureID)&imgui_atlas) {
						assert(not_implemented);
					}

					auto shad = use_shader("imgui");
					assert(shad);

					use_vertex_data(*shad, layout, vbo);

					flt y0 = (flt)wnd_size_px.y -pcmd->ClipRect.w;
					flt y1 = (flt)wnd_size_px.y -pcmd->ClipRect.y;

					glScissor((int)pcmd->ClipRect.x, (int)y0, (int)(pcmd->ClipRect.z -pcmd->ClipRect.x), (int)(y1 -y0));

					bind_texture(shad, "tex", 0, imgui_atlas);

					auto* ptr = (GLvoid const*)((u8 const*)cur_idx_buffer -(u8 const*)idx_buffer);
					glDrawElements(GL_TRIANGLES, pcmd->ElemCount, GL_UNSIGNED_SHORT, ptr);

				}
				cur_idx_buffer += pcmd->ElemCount;
			}
		}
	}

	// ImGui::DestroyContext();

	struct Imgui_Texture_Window {
		std::string	tex_name = "";

		bool open = false;

		bv4		show_channels = true;
		float	show_lod = 0;
		bool	nearest_filter = false;

		// where to display texture
		v2 pos_screen;
		v2 size_screen;
	};

	std::vector< unique_ptr<Imgui_Texture_Window> > texture_windows;

	struct Engine_Draw_Texture_Vertex {
		v2		pos_screen; // top down coords
		v2		uv;

		static const Data_Vertex_Layout layout;
	};
	const Data_Vertex_Layout Engine_Draw_Texture_Vertex::layout = { (int)sizeof(Engine_Draw_Texture_Vertex), {
		{ "pos_screen",			FV2,	(int)offsetof(Engine_Draw_Texture_Vertex, pos_screen) },
		{ "uv",					FV2,	(int)offsetof(Engine_Draw_Texture_Vertex, uv) },
	}};

	void draw_tex_2d (ImDrawList const* parent_list, ImDrawCmd const* cmd) {
		auto* w = (Imgui_Texture_Window*)cmd->UserCallbackData;

		Any_Texture* tex = texture_manager.find_texture(w->tex_name);
		assert(tex->type == TEXTURE_2D);

		auto* s = use_shader("imgui_texture_window_2d");
		assert(s);

		set_uniform(s, "show_channels", w->show_channels);
		set_uniform(s, "show_lod", w->show_lod);

		GLuint sampler_normal, sampler_nearest;
		{
			glGenSamplers(1, &sampler_normal);
			glGenSamplers(1, &sampler_nearest);

			glSamplerParameteri(sampler_normal, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glSamplerParameteri(sampler_normal, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glSamplerParameteri(sampler_normal, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glSamplerParameteri(sampler_normal, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glSamplerParameteri(sampler_nearest, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glSamplerParameteri(sampler_nearest, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glSamplerParameteri(sampler_nearest, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glSamplerParameteri(sampler_nearest, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		std::vector<Engine_Draw_Texture_Vertex> vbo_data;
		for (auto p : quad_verts) {
			Engine_Draw_Texture_Vertex v;
			v.pos_screen = w->pos_screen + p * w->size_screen;
			v.uv = v2(p.x, 1 -p.y);
			vbo_data.push_back(v);
		}

		glBindSampler(0, w->nearest_filter ? sampler_nearest : sampler_normal);

		bind_texture(s, "tex", 0, tex->tex2d);

		draw_stream_triangles(*s, vbo_data);

		glBindSampler(0, 0);
	}
	
	void imgui_texture_windows () {

		if (ImGui::Button("Texture Window")) {
			auto first_not_open_window = std::find_if(texture_windows.begin(), texture_windows.end(), [] (unique_ptr<Imgui_Texture_Window> const& w) { return !w->open; });
			if (first_not_open_window == texture_windows.end()) {
				texture_windows.emplace_back( make_unique<Imgui_Texture_Window>() );
				first_not_open_window = texture_windows.end() -1;
			}

			(*first_not_open_window)->open = true;
		}

		for (int i=0; i<(int)texture_windows.size(); ++i) {
			auto& w = texture_windows[i];
			if (!w->open) continue;

			if (ImGui::Begin(prints("Texture Window %d", i).c_str(), &w->open)) {
				
				if (ImGui::BeginCombo("tex_name", w->tex_name.c_str())) {
					for (auto& t : texture_manager.textures) {
						bool is_selected = t.first.compare(w->tex_name) == 0;
						if (ImGui::Selectable(t.first.c_str(), is_selected))
							w->tex_name = t.first;
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				Any_Texture* tex = texture_manager.find_texture(w->tex_name);

				if (tex && tex->type == TEXTURE_2D) {
					
					static std::vector<iv2> mip_sizes;
					mip_sizes.clear();

					if (tex->o.mipmap_mode == USE_MIPMAPS) {
						mipmap_chain(tex->size_px, [&] (int mip, iv2 sz) {
								mip_sizes.push_back(sz);
							});
					} else {
						mip_sizes.push_back(tex->size_px);
					}

					ImGui::Text("size: %4d x %4d  mips: %2d", tex->size_px.x,tex->size_px.y, (int)mip_sizes.size());

					for (int i=0; i<4; ++i) {
						cstr labels[4] = {"R","G","B","A"};
						ImGui::Checkbox(labels[i], &w->show_channels[i]);

						ImGui::SameLine();
					}

					ImGui::Checkbox("Nearest", &w->nearest_filter);

					iv2 mip_sz = mip_sizes[ (int)roundf(w->show_lod) ];
					ImGui::SameLine();
					ImGui::SliderFloat("##show_lod", &w->show_lod, 0,(flt)mip_sizes.size() -1, prints("show_lod: %4.1f  %4d x %4d", w->show_lod, mip_sz.x,mip_sz.y).c_str());

					v2 display_size = ImGui::GetContentRegionAvailWidth();
					display_size.y *= tex->size_px.y / tex->size_px.x;

					auto pos_screen = ImGui::GetCursorScreenPos();

					w->pos_screen = v2(pos_screen.x,pos_screen.y);
					w->size_screen = display_size;

					ImGui::GetWindowDrawList()->AddCallback(draw_tex_2d, w.get());
				}

			}
			ImGui::End();
		}
	}

}
