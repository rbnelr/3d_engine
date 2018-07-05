
#include "engine.hpp"
using namespace engine;

void set_material_albedo(Shader* s, Texture2D const& tex) {
	bind_texture(s,	"albedo_tex", 0, tex);
	set_uniform(s,	"albedo_mult", lrgba(1));
	set_uniform(s,	"albedo_offs", lrgba(0));
}
void set_material_albedo(Shader* s, lrgba col) {
	bind_texture(s, "albedo_tex", 0, *tex_black());
	set_uniform(s,	"albedo_mult", lrgba(0));
	set_uniform(s,	"albedo_offs", col);
}

void set_material_metallic(Shader* s, Texture2D const& tex) {
	bind_texture(s,	"metallic_tex", 1, tex);
	set_uniform(s,	"metallic_mult", 1.0f);
	set_uniform(s,	"metallic_offs", 0.0f);
}
void set_material_metallic(Shader* s, flt val) {
	bind_texture(s,	"metallic_tex", 1, *tex_black());
	set_uniform(s,	"metallic_mult", 0.0f);
	set_uniform(s,	"metallic_offs", val);
}

void set_material_roughness(Shader* s, Texture2D const& tex) {
	bind_texture(s,	"roughness_tex", 2, tex);
	set_uniform(s,	"roughness_mult", 1.0f);
	set_uniform(s,	"roughness_offs", 0.0f);
}
void set_material_roughness(Shader* s, flt val) {
	bind_texture(s, "roughness_tex", 2, *tex_black());
	set_uniform(s,	"roughness_mult", 0.0f);
	set_uniform(s,	"roughness_offs", val);
}

void set_material_normal(Shader* s, Texture2D const& tex) {
	bind_texture(s,	"normal_tex", 3, tex);
}
void set_material_normal_identity(Shader* s) {
	bind_texture(s,	"normal_tex", 3, *tex_identity_normal());
}

std::string shad_default_2d =			"shaders/default_2d";
std::string shad_default_3d =			"shaders/default_3d";

hm calc_cam_to_world (v3 cam_pos_world, v3 cam_altazimuth) {
	quat rot = rotateQ_Z(cam_altazimuth.x) * rotateQ_X(cam_altazimuth.y) * rotateQ_Z(cam_altazimuth.z);
	return translateH(cam_pos_world) * hm(convert_to_m3(rot));
}
hm calc_world_to_cam (v3 cam_pos_world, v3 cam_altazimuth) {
	quat rot = rotateQ_Z(-cam_altazimuth.z) * rotateQ_X(-cam_altazimuth.y) * rotateQ_Z(-cam_altazimuth.x); // rotate world around camera
	return hm(convert_to_m3(rot)) * translateH(-cam_pos_world);
}
m4 calc_perspective_matrix (flt vfov, flt clip_near, flt clip_far, flt aspect_w_over_h) {

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
	flt		vel_cam = 4;

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

		v3 move_vel = inp.buttons[GLFW_KEY_LEFT_SHIFT].is_down ? vel_cam : 1.0f;

		iv3 move_dir = 0;
		if (inp.buttons['S'].is_down)					move_dir.z += 1;
		if (inp.buttons['W'].is_down)					move_dir.z -= 1;
		if (inp.buttons['A'].is_down)					move_dir.x -= 1;
		if (inp.buttons['D'].is_down)					move_dir.x += 1;
		if (inp.buttons[GLFW_KEY_LEFT_CONTROL].is_down)	move_dir.y -= 1;
		if (inp.buttons[' '].is_down)					move_dir.y += 1;

		pos_world += (calc_cam_to_world(pos_world, altazimuth).m3() * (normalize_or_zero((v3)move_dir) * move_vel)) * dt;
	}

	void imgui () {
		ImGui::DragFloat3("cam.pos_world", &pos_world.x, 1.0f / 100);
		ImGui::SliderAngle("cam.vfov", &vfov, 0,180);

		ImGui::DragFloat("cam.vel_cam", &vel_cam, 1.0f / 100);
	}
};

template <typename VERTEX> void quad (flt r, VERTEX vert) {
	vert(v2(+r,-r));
	vert(v2(+r,+r));
	vert(v2(-r,-r));
	vert(v2(-r,-r));
	vert(v2(+r,+r));
	vert(v2(-r,+r));
}
template <typename VERTEX> void generate_cube (flt r, VERTEX vert) {
	quad(r, [&] (v2 p) {	vert(												v3(p,r)); });
	quad(r, [&] (v2 p) {	vert(rotate3_Z(deg(180)) *	rotate3_X(deg(180)) *	v3(p,r)); });
	quad(r, [&] (v2 p) {	vert(rotate3_X(deg(90)) *	rotate3_Y(deg(90)) *	v3(p,r)); });
	quad(r, [&] (v2 p) {	vert(rotate3_X(deg(90)) *	rotate3_Y(deg(-90)) *	v3(p,r)); });
	quad(r, [&] (v2 p) {	vert(rotate3_Y(deg(180)) *	rotate3_X(deg(-90)) *	v3(p,r)); });
	quad(r, [&] (v2 p) {	vert(						rotate3_X(deg(90)) *	v3(p,r)); });
}

int main () {
	
	Window wnd;
	wnd.create("3D", iv2(1280,720), VSYNC_ON);
	
	Delta_Time_Measure dt_measure;
	f32 dt = dt_measure.begin();

	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	for (;;) {
		
		auto inp = wnd.poll_input();

		if (wnd.wants_to_close())
			break;

		begin_imgui(inp, dt);

		{
			static f32 frame_time_running_avg = dt;
			flt frame_time_running_avg_alpha = 0.1f;

			frame_time_running_avg = lerp(frame_time_running_avg, dt, frame_time_running_avg_alpha);

			ImGui::Text("dt: %6.2f ms, fps: %6.2f", frame_time_running_avg * 1000, 1.0f / frame_time_running_avg);
		}

		set_shared_uniform("common", "screen_size", (v2)inp.wnd_size_px);
		set_shared_uniform("common", "mcursor_pos", inp.mouse_cursor_pos_screen_buttom_up_pixel_center());

		set_shared_uniform("common", "cubemap_gl_ori_to_z_up", rotate3_X(deg(90)));
		set_shared_uniform("common", "cubemap_z_up_to_gl_ori", rotate3_X(deg(-90)));

		static bool draw_wireframe = false;
		ImGui::Checkbox("draw_wireframe", &draw_wireframe);
		if (inp.buttons['P'].went_down)
			draw_wireframe = !draw_wireframe;

		set_shared_uniform("wireframe", "enable", draw_wireframe);

		static Camera cam;
		//saveable("3d_camera", &cam);

		cam.control(inp, dt);
		cam.imgui();

		hm	world_to_cam = calc_world_to_cam(cam.pos_world, cam.altazimuth);
		hm	cam_to_world = calc_cam_to_world(cam.pos_world, cam.altazimuth);

		m4	cam_to_clip = calc_perspective_matrix(cam.vfov, cam.clip_near, cam.clip_far, (flt)inp.wnd_size_px.x / (flt)inp.wnd_size_px.y);
		
		set_shared_uniform("_3d_view", "world_to_cam", world_to_cam.m4());
		set_shared_uniform("_3d_view", "cam_to_world", cam_to_world.m4());
		set_shared_uniform("_3d_view", "cam_to_clip", cam_to_clip);
		
		// create a skybox via shader
		iv2 cubemap_res = 128;

		static int selected_skybox = 5;
		static cstr skyboxes[] = {
			"generated skybox",
			"assets/ely_nevada/nevada_%s.tga",
			"assets/SkyboxSet1/CloudyLightRays/CloudyLightRays%s2048.png",
			"assets/SkyboxSet1/DarkStormy/DarkStormy%s2048.png",
			"assets/SkyboxSet1/FullMoon/FullMoon%s2048.png",
			"assets/SkyboxSet1/SunSet/SunSet%s2048.png",
			"assets/SkyboxSet1/ThickCloudsWater/ThickCloudsWater%s2048.png",
			"assets/SkyboxSet1/TropicalSunnyDay/TropicalSunnyDay%s2048.png",
		};
		bool selected_skybox_changed = ImGui::Combo("skybox", &selected_skybox, skyboxes, ARRLEN(skyboxes));

		v3 skyboxes_dir_to_sun[] = {
			0,
			rotate3_Z(deg(  8)) * rotate3_X(deg( 32)) * v3(0,1,0),
			rotate3_Z(deg( 53)) * rotate3_X(deg( 41)) * v3(0,1,0),
			rotate3_Z(deg(-89)) * rotate3_X(deg( 12)) * v3(0,1,0),
			rotate3_Z(deg( 70)) * rotate3_X(deg( 17)) * v3(0,1,0),
			rotate3_Z(deg( 65)) * rotate3_X(deg(  3)) * v3(0,1,0),
			rotate3_Z(deg( 34)) * rotate3_X(deg(  6)) * v3(0,1,0),
			rotate3_Z(deg(-20)) * rotate3_X(deg( 19)) * v3(0,1,0),
		};

		static struct {
			srgb8	col;
			flt		intensity;
		} skyboxes_sun_col[] = {
			{ srgb8(0), 0 },
			{ srgb8(255,234,226), 6.00f },
			{ srgb8(248,233,232), 0.31f },
			{ srgb8(216,207,207), 0.14f },
			{ srgb8(255,255,255), 0.37f },
			{ srgb8(243,157, 80), 1.71f },
			{ srgb8(248,191,152), 2.20f },
			{ srgb8(255,248,236), 6.00f },
		};

		static flt rotate_skybox = 0;
		ImGui::SliderAngle("rotate_skybox", &rotate_skybox, -180, +180);
		
		m3 world_to_skybox = rotate3_Z(rotate_skybox);
		m3 skybox_to_world = rotate3_Z(-rotate_skybox);
		
		set_shared_uniform("common", "world_to_skybox", world_to_skybox);

		v3 dir_to_sun_world = skybox_to_world * skyboxes_dir_to_sun[selected_skybox];
		lrgb skybox_light_radiance = skyboxes_sun_col[selected_skybox].col.to_lrgb() * skyboxes_sun_col[selected_skybox].intensity;

		set_shared_uniform("common", "skybox_light_dir_world", dir_to_sun_world);
		set_shared_uniform("common", "skybox_light_radiance", skybox_light_radiance);

		static flt ambient_light = 0.1f;
		ImGui::SliderFloat("ambient_light", &ambient_light, 0,1);
		set_shared_uniform("common", "ambient_light", v3(ambient_light));

		TextureCube* skybox;
		if (selected_skybox > 0) {
			
			static cstr prev_selected_skybox = skyboxes[selected_skybox];
			if (selected_skybox_changed) {
				evict_cubemap(prev_selected_skybox);
				
				prev_selected_skybox = skyboxes[selected_skybox];
			}

			skybox = get_multifile_cubemap(skyboxes[selected_skybox], { PF_SRGB8, USE_MIPMAPS },
					selected_skybox != 1 ?		 std::vector<std::string>{"Right","Left","Down","Up","Back","Front"} :	std::vector<std::string>{"rt","lf","dn","up","bk","ft"},
					selected_skybox != 1 ?		 std::vector<int>{0,0,ROT_180,ROT_180,0,0} :							std::vector<int>{0,0,ROT_90,ROT_90,0,0});

		} else {
			static auto _skybox = alloc_cube_texture(cubemap_res, { PF_LRGBF, NO_MIPMAPS });
			skybox = &_skybox;

			static auto fbo = draw_to_texture(_skybox, cubemap_res);

			glViewport(0,0, cubemap_res.x,cubemap_res.y);

			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glDisable(GL_SCISSOR_TEST);
			
			auto s = use_shader("shaders/skybox_cubemap_gen");
			assert(s);


			struct Vertex {
				v3		pos_world;
			};
			const Data_Vertex_Layout layout = { (int)sizeof(Vertex), {
				{ "pos_world",		FV3,	(int)offsetof(Vertex, pos_world) },
			}};

			static std::vector<Vertex> cube;
			if (cube.size() == 0)
				generate_cube(1, [] (v3 p) { cube.push_back({p}); });

			static auto vbo = stream_vertex_data(cube.data(), (int)cube.size() * layout.vertex_size);
		
			bind_vertex_data(vbo, layout, *s);
			
			static m3 faces_world_to_cam[6] = {
				rotate3_Z(deg(180)) *	rotate3_Y(deg( 90)),	// pos x
				rotate3_Z(deg(180)) *	rotate3_Y(deg(-90)),	// neg x
										rotate3_X(deg(-90)),	// pos y
										rotate3_X(deg( 90)),	// neg y
										rotate3_X(deg(180)),	// pos z
				rotate3_Z(deg(180)),							// neg z
			};

			m4 cam_to_clip = calc_perspective_matrix(deg(90), 1.0f/16, 1024, 1);
			set_uniform(s, "cam_to_clip", cam_to_clip);

			for (auto face=0; face<6; ++face) {
				fbo.bind(face);

				set_uniform(s, "world_to_cam", faces_world_to_cam[face]);

				draw_triangles(*s, 0, (int)cube.size());
			}
		
		}

		auto draw_scene = [&] (Texture2D* prev_framebuffer) {
			
			lrgb clear_color_lrgb = srgb8(30).to_lrgb();

			glClearColor(clear_color_lrgb.x,clear_color_lrgb.y,clear_color_lrgb.z,255);
			//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			glClear(GL_DEPTH_BUFFER_BIT);

			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);

			{
				auto s = use_shader("shaders/skybox");
				assert(s);
				
				struct Vertex {
					v3		pos_world;
				};
				const Data_Vertex_Layout layout = { (int)sizeof(Vertex), {
					{ "pos_world",		FV3,	(int)offsetof(Vertex, pos_world) },
				}};

				static std::vector<Vertex> cube;
				if (cube.size() == 0)
					generate_cube(1, [] (v3 p) { cube.push_back({p}); });

				static auto vbo = stream_vertex_data(cube.data(), (int)cube.size() * layout.vertex_size);

				bind_vertex_data(vbo, layout, *s);

				bind_texture(s, "skybox", 0, *skybox);

				draw_triangles(*s, 0, (int)cube.size());
			}

			glDisable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			glDisable(GL_SCISSOR_TEST);


			if (1) {
				auto cerberus = mesh_manager.get_mesh("assets/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
				
				auto s = use_shader(shad_default_3d);
				assert(s);

				static VBO vbo = stream_vertex_data(cerberus->vbo_data.data(), (int)cerberus->vbo_data.size() * (int)sizeof(Default_Vertex_3d));
				use_vertex_data(*s, Default_Vertex_3d::layout, vbo);

				set_uniform(s, "model_to_world", translate4(v3(+5,0,0)) * scale4(1.0f / 50));

				bind_texture(s, "skybox", 0, *skybox);

				set_material_albedo(s,		*get_texture("assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga", { PF_SRGB8 }));
				set_material_metallic(s,	*get_texture("assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga", { PF_LRGB8 }));
				set_material_roughness(s,	*get_texture("assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga", { PF_LRGB8 }));
				set_material_normal(s,		*get_texture("assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga", { PF_LRGB8 }));

				draw_triangles(*s, 0, (int)cerberus->vbo_data.size());
			}

			if (1) {
				auto mesh = mesh_manager.get_mesh("assets/terrain.fbx");

				auto s = use_shader(shad_default_3d);
				assert(s);

				static VBO vbo = stream_vertex_data(mesh->vbo_data.data(), (int)mesh->vbo_data.size() * (int)sizeof(Default_Vertex_3d));
				use_vertex_data(*s, Default_Vertex_3d::layout, vbo);

				set_uniform(s, "model_to_world", translate4(v3(0,0,0)) * scale4(500));

				bind_texture(s, "skybox", 0, *skybox);

				set_material_albedo(s,		lwhite);
				set_material_metallic(s,	0);
				set_material_roughness(s,	0.8f);
				set_material_normal_identity(s);

				draw_triangles(*s, 0, (int)mesh->vbo_data.size());
			}

			std::vector<Default_Vertex_3d> quad;

			for (auto p : { v2(1,0),v2(1,1),v2(0,0), v2(0,0),v2(1,1),v2(0,1) }) {
				Default_Vertex_3d v;
				v.pos_model = v3(p -0.5f, +0.5f);
				v.normal_model = v3(0,0,1);
				v.tangent_model = v4(1,0,0,1);
				v.uv = p;
				v.col_lrgba = srgba8(230).to_lrgb();
				quad.push_back(v);
			}

			for (int face=0; face<6; ++face) {

				auto s = use_shader(shad_default_3d);
				if (s) {
					hm model_to_world;

					if (		face == 0 ) {
						model_to_world = m3::ident();
					} else if (	face == 1 ) {
						model_to_world = rotateH_Z(deg(180)) * rotateH_X(deg(180));
					} else if (	face == 2 ) {
						model_to_world = rotateH_X(deg(90)) * rotateH_Y(deg(90));
					} else if (	face == 3 ) {
						model_to_world = rotateH_X(deg(90)) * rotateH_Y(deg(-90));
					} else if (	face == 4 ) {
						model_to_world = rotateH_Y(deg(180)) * rotateH_X(deg(-90));
					} else if (	face == 5 ) {
						model_to_world = rotateH_X(deg(90));
					}

					model_to_world = translateH(v3(0,0,0)) * model_to_world;

					set_uniform(s, "model_to_world", model_to_world.m4());

					int tex_select = face / 2;

					Texture2D tmp;

					if (		tex_select == 0 ) {
						srgba8 pixels[16][16];
						for (int y=0; y<16; ++y) {
							for (int x=0; x<16; ++x) {
								pixels[y][x] = BOOL_XOR(BOOL_XOR(x % 2, y % 2), mymod((flt)glfwGetTime(), 1.0f) < 0.5f) ? srgba8(220,150,150,255) : srgba8(255);
							}
						}

						tmp = upload_texture(pixels, iv2(16,16), { PF_SRGBA8, NO_MIPMAPS, FILTER_NEAREST });
						set_material_albedo(s, tmp);
					} else if ( tex_select == 1 ) {
						set_material_albedo(s, *get_texture("assets/dab.png", { PF_SRGBA8, NO_MIPMAPS }));
					} else if ( tex_select == 2 ) {

						if (prev_framebuffer)
							set_material_albedo(s, *prev_framebuffer);
						else
							set_material_albedo(s, upload_texture(&black, iv2(1,1), { PF_SRGBA8, NO_MIPMAPS, FILTER_NEAREST }));
					}

					set_material_metallic(s,	0);
					set_material_roughness(s,	0.5f);
					set_material_normal_identity(s);

					draw_stream_triangles(*s, quad);
				}
			}

		};

		if (1) {
			
			draw_to_screen();

			glViewport(0,0, inp.wnd_size_px.x,inp.wnd_size_px.y);
			glScissor(0,0, inp.wnd_size_px.x,inp.wnd_size_px.y);

			draw_scene(nullptr);

		} else {
			Texture2D* framebuffer;
			{ // draw into scene into framebuffer, and use that framebuffer on the next frame as a texture in the scene
				static Texture2D* prev_framebuffer = nullptr;
			
				struct Framebuffer {
					Texture2D	tex;
					iv2			size = -1;
					FBO			fbo;
				};
				static Framebuffer doublebuffer_framebuffers[2];
				static Framebuffer* _framebuffer = &doublebuffer_framebuffers[0];

				static FBO fbo;

				if (any(inp.wnd_size_px != _framebuffer->size)) {
					_framebuffer->size = inp.wnd_size_px;
					_framebuffer->tex = alloc_texture(_framebuffer->size, { PF_LRGBF, NO_MIPMAPS });
					_framebuffer->fbo = draw_to_texture(_framebuffer->tex, _framebuffer->size);
				}

				_framebuffer->fbo.bind();
			
				glViewport(0,0, inp.wnd_size_px.x,inp.wnd_size_px.y);
				glScissor(0,0, inp.wnd_size_px.x,inp.wnd_size_px.y);

				draw_scene(prev_framebuffer);

				framebuffer = &_framebuffer->tex;
				prev_framebuffer = framebuffer;

				_framebuffer = &doublebuffer_framebuffers[ (_framebuffer -&doublebuffer_framebuffers[0]) ^ 1 ];
			}

			{ // draw framebuffer as fullscreen quad
				draw_to_screen();

				glViewport(0,0, inp.wnd_size_px.x,inp.wnd_size_px.y);
				glScissor(0,0, inp.wnd_size_px.x,inp.wnd_size_px.y);

				glDisable(GL_BLEND);
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
				glDisable(GL_SCISSOR_TEST);
		
				{//draw_fullscreen_quad();
			
					struct Vertex_2d {
						v4		pos_clip;
						v2		uv;
					};
					const Data_Vertex_Layout layout = { (int)sizeof(Vertex_2d), {
						{ "pos_clip",			FV4,	(int)offsetof(Vertex_2d, pos_clip) },
						{ "uv",					FV2,	(int)offsetof(Vertex_2d, uv) },
					}};
			
					auto shad = use_shader("shaders/fullscreen_quad");
					assert(shad);

					static Vertex_2d fullscreen_quad[] = {
						{ v4(+1,-1, 0,1), v2(1,0) },
						{ v4(+1,+1, 0,1), v2(1,1) },
						{ v4(-1,-1, 0,1), v2(0,0) },
						{ v4(-1,-1, 0,1), v2(0,0) },
						{ v4(+1,+1, 0,1), v2(1,1) },
						{ v4(-1,+1, 0,1), v2(0,1) },
					};
					static VBO vbo = stream_vertex_data(fullscreen_quad, sizeof(fullscreen_quad));
					use_vertex_data(*shad, layout, vbo);

					bind_texture(shad, "tex", 0, *framebuffer);
					draw_triangles(*shad, 0, 6);
				}
			}
		}

		end_imgui(inp.wnd_size_px);

		wnd.swap_buffers();
		
		dt = dt_measure.frame();
	}

	return 0;
}
