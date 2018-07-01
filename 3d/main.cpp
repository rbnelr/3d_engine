
#include "engine.hpp"
using namespace engine;

std::string shad_default_2d =			"shaders/default_2d";
std::string shad_default_3d =			"shaders/default_3d";

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

		begin_imgui(inp, dt);

		lrgb clear_color_lrgb = srgb8(40).to_lrgb();

		glViewport(0,0, inp.wnd_size_px.x,inp.wnd_size_px.y);
		glScissor(0,0, inp.wnd_size_px.x,inp.wnd_size_px.y);

		#if 1
		glClearColor(clear_color_lrgb.x,clear_color_lrgb.y,clear_color_lrgb.z,255);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		#endif

		set_shared_uniform("common", "screen_size", (v2)inp.wnd_size_px);
		set_shared_uniform("common", "mcursor_pos", inp.mouse_cursor_pos_screen_buttom_up_pixel_center());

		static bool draw_wireframe = false;
		ImGui::Checkbox("draw_wireframe", &draw_wireframe);
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
		
		//
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_SCISSOR_TEST);

		std::vector<Default_Vertex_3d> quad;

		for (auto p : { v2(1,0),v2(1,1),v2(0,0), v2(0,0),v2(1,1),v2(0,1) }) {
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
						srgba8 pixels[16][16];
						for (int y=0; y<16; ++y) {
							for (int x=0; x<16; ++x) {
								pixels[y][x] = BOOL_XOR(BOOL_XOR(x % 2, y % 2), mymod((flt)glfwGetTime(), 1.0f) < 0.5f) ? srgba8(220,150,150,255) : srgba8(255);
							}
						}
				
						tmp = upload_texture(pixels, iv2(16,16), PF_SRGBA8, NO_MIPMAPS, FILTER_NEAREST);
						bind_texture(s, "tex", 0, tmp);
					} else if ( cycle == 1 ) {
						bind_texture(s, "tex", 0, *get_texture("dab.png", PF_SRGBA8, NO_MIPMAPS));
					} else if ( cycle == 2 ) {
						static Texture2D white_tex = upload_texture(&white, iv2(1,1), PF_SRGBA8, NO_MIPMAPS, FILTER_NEAREST);
			
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

		end_imgui(inp.wnd_size_px);

		wnd.swap_buffers();
		
		dt = dt_measure.frame();
	}

	return 0;
}
