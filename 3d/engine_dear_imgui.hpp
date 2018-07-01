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

	//begin_user_texture_callback_fp begin_user_texture_callback = nullptr;
	//end_user_texture_callback_fp end_user_texture_callback = nullptr;
	//
	//unique_ptr<Texture2D> imgui_make_user_texture (pixel_format_e format, void const* pixels, s32v2 size_px, minmag_filter_e minmag, aniso_filter_e aniso, border_e border, rgba8 border_color) {
	//	return make_unique<Texture2D>( single_mip_texture(format, pixels, size_px, minmag, aniso, border, border_color) );
	//}

	void begin_imgui (Input const& inp, flt dt) {
		auto init = [] () {
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();

			{
				io.KeyMap[ ImGuiKey_Tab			 ] = GLFW_KEY_TAB;
				io.KeyMap[ ImGuiKey_LeftArrow	 ] = GLFW_KEY_LEFT;
				io.KeyMap[ ImGuiKey_RightArrow	 ] = GLFW_KEY_RIGHT;
				io.KeyMap[ ImGuiKey_UpArrow		 ] = GLFW_KEY_UP;
				io.KeyMap[ ImGuiKey_DownArrow	 ] = GLFW_KEY_DOWN;
				io.KeyMap[ ImGuiKey_PageUp		 ] = GLFW_KEY_PAGE_UP;
				io.KeyMap[ ImGuiKey_PageDown	 ] = GLFW_KEY_PAGE_DOWN;
				io.KeyMap[ ImGuiKey_Home		 ] = GLFW_KEY_HOME;
				io.KeyMap[ ImGuiKey_End			 ] = GLFW_KEY_END;
				io.KeyMap[ ImGuiKey_Insert		 ] = GLFW_KEY_INSERT;
				io.KeyMap[ ImGuiKey_Delete		 ] = GLFW_KEY_DELETE;
				io.KeyMap[ ImGuiKey_Backspace	 ] = GLFW_KEY_BACKSPACE;
				io.KeyMap[ ImGuiKey_Space		 ] = GLFW_KEY_SPACE;
				io.KeyMap[ ImGuiKey_Enter		 ] = GLFW_KEY_ENTER;
				io.KeyMap[ ImGuiKey_Escape		 ] = GLFW_KEY_ESCAPE;
				io.KeyMap[ ImGuiKey_A			 ] = GLFW_KEY_A;
				io.KeyMap[ ImGuiKey_C			 ] = GLFW_KEY_C;
				io.KeyMap[ ImGuiKey_V			 ] = GLFW_KEY_V;
				io.KeyMap[ ImGuiKey_X			 ] = GLFW_KEY_X;
				io.KeyMap[ ImGuiKey_Y			 ] = GLFW_KEY_Y;
				io.KeyMap[ ImGuiKey_Z			 ] = GLFW_KEY_Z;
			}

			{
				typedef srgba8 pixel;

				s32v2	size;
				u8*		pixels; // Imgui mallocs and frees the pixels
				io.Fonts->GetTexDataAsRGBA32(&pixels, &size.x,&size.y);

				//flip_vertical_inplace(pixels, size.x * sizeof(pixel), size.y);

				imgui_atlas = upload_texture(pixels,size, PF_SRGBA8, NO_MIPMAPS, FILTER_BILINEAR, FILTER_NO_ANISO);

				io.Fonts->TexID = (void*)&imgui_atlas;
			}

			return true;
		};
		static bool initialized = init();

		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize.x = (flt)inp.wnd_size_px.x;
		io.DisplaySize.y = (flt)inp.wnd_size_px.y;
		io.DeltaTime = dt;
		io.MousePos.x = inp.mousecursor.pos_screen.x;
		io.MousePos.y = inp.mousecursor.pos_screen.y;
		io.MouseDown[0] = inp.buttons[GLFW_MOUSE_BUTTON_LEFT].is_down;
		io.MouseDown[1] = inp.buttons[GLFW_MOUSE_BUTTON_RIGHT].is_down;
		io.MouseWheel = inp.mousewheel.delta;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::NewFrame();
	}

	void end_imgui (iv2 wnd_size_px) {
		ImGui::Render();

		glViewport(0,0, wnd_size_px.x,wnd_size_px.y);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_SCISSOR_TEST);

		ImDrawData* draw_data = ImGui::GetDrawData();

		static VBO vbo = VBO::generate();
		static EBO ebo = EBO::generate();

		auto shad = use_shader("shaders/imgui");
		assert(shad);

		set_uniform(shad, "screen_size", (v2)wnd_size_px);

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

		use_vertex_data(*shad, layout, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo.get_handle());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.get_handle());

		for (int n = 0; n < draw_data->CmdListsCount; n++) {
			auto* cmd_list = draw_data->CmdLists[n];

			ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;  // vertex buffer generated by ImGui
			ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;   // index buffer generated by ImGui

			auto vertex_size = cmd_list->VtxBuffer.size() * sizeof(ImDrawVert);
			auto index_size = cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx);

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
					unique_ptr<Texture2D> user_texture = nullptr;

					//if (pcmd->TextureId != &imgui_atlas && begin_user_texture_callback) {
					//	user_texture = begin_user_texture_callback(pcmd->TextureId);
					//}
					
					flt y0 = (flt)wnd_size_px.y -pcmd->ClipRect.w;
					flt y1 = (flt)wnd_size_px.y -pcmd->ClipRect.y;

					glScissor((int)pcmd->ClipRect.x, (int)y0, (int)(pcmd->ClipRect.z -pcmd->ClipRect.x), (int)(y1 -y0));

					bind_texture(shad, "tex", 0, user_texture ? *user_texture : imgui_atlas);

					auto* ptr = (GLvoid const*)((u8 const*)cur_idx_buffer -(u8 const*)idx_buffer);
					glDrawElements(GL_TRIANGLES, pcmd->ElemCount, GL_UNSIGNED_SHORT, ptr);

					//if (pcmd->TextureId != &imgui_atlas && end_user_texture_callback) {
					//	end_user_texture_callback(pcmd->TextureId, std::move(user_texture));
					//}
				}
				cur_idx_buffer += pcmd->ElemCount;
			}
		}
	}

	// ImGui::DestroyContext();
}
