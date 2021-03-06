
#include "engine.hpp"
using namespace engine;
namespace imgui = ImGui;

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

void set_material_normal(Shader* s, Texture2D const& tex, bool flip_y=false) { // flip_y: unreal engine has y flipped in comparison to my approach (and unity and blender)
	bind_texture(s,	"normal_tex", 3, tex);
	set_uniform(s,	"normal_mult", flip_y ? v3(1,-1,1) : v3(1));
	set_uniform(s,	"normal_offs", flip_y ? v3(0,+1,0) : v3(0));
}
void set_material_normal_identity(Shader* s) {
	bind_texture(s,	"normal_tex", 3, *tex_identity_normal());
}

void set_material_ao(Shader* s, Texture2D const& tex) {
	bind_texture(s,	"ao_tex", 4, tex);
	set_uniform(s,	"ao_mult", 1.0f);
	set_uniform(s,	"ao_offs", 0.0f);
}
void set_material_ao_identity(Shader* s) {
	bind_texture(s,	"ao_tex", 4, *tex_black());
	set_uniform(s,	"ao_mult", 0.0f);
	set_uniform(s,	"ao_offs", 1.0f);
}

void set_material_displacement(Shader* s, Texture2D const& tex, flt displacement_size) {
	bind_texture(s,	"displacement_tex", 5, tex);
	set_uniform(s,	"displacement_size", displacement_size);
}
void set_material_displacement_identity(Shader* s) {
	bind_texture(s,	"displacement_tex", 5, *tex_black());
	set_uniform(s,	"displacement_size", 0.0f);
}

std::string shad_default_2d =			"default_2d";
std::string shad_default_3d =			"default_3d";

hm calc_cam_to_world (v3 cam_pos_world, v3 cam_altazimuth) {
	quat rot = rotateQ_Z(cam_altazimuth.x) * rotateQ_X(cam_altazimuth.y) * rotateQ_Z(cam_altazimuth.z);
	return translateH(cam_pos_world) * hm(convert_to_m3(rot));
}
hm calc_world_to_cam (v3 cam_pos_world, v3 cam_altazimuth) {
	quat rot = rotateQ_Z(-cam_altazimuth.z) * rotateQ_X(-cam_altazimuth.y) * rotateQ_Z(-cam_altazimuth.x); // rotate world around camera
	return hm(convert_to_m3(rot)) * translateH(-cam_pos_world);
}

struct Camera {
	v3		pos_world = v3(0.1f, -2, +1);
	
	v3		altazimuth = v3(0,deg(50),0); // x: azimuth (along horizon) y: altitude (up/down) z: roll
	
	flt		vfov = deg(75);

	flt		clip_near = 1.0f / 16;
	flt		clip_far = 1024;

	// flycam movement
	flt		base_speed = 1;
	flt		fast_mult = 4;

	void control (Input const& inp, flt dt) {
		
		flt roll_vel = deg(45); // ang / sec

		int roll_dir = 0;
		if (inp.is_down('Q')) roll_dir -= 1;
		if (inp.is_down('E')) roll_dir += 1;

		altazimuth.z += (flt)-roll_dir * roll_vel * dt;

		v2 mouselook_sensitivity = deg(0.12f); // ang / mouse_move_px
		
		if (inp.is_down(GLFW_MOUSE_BUTTON_RIGHT) && !inp.block_mouse) {
			altazimuth.x -=  inp.mousecursor.delta_screen.x * mouselook_sensitivity.x;
			altazimuth.y += -inp.mousecursor.delta_screen.y * mouselook_sensitivity.y; // delta_screen is top down
		}

		altazimuth.x = mod_range(altazimuth.x, deg(-180), deg(+180));
		altazimuth.y = clamp(altazimuth.y, deg(0), deg(180));
		altazimuth.z = mymod(altazimuth.z, deg(360));

		// movement
		iv3 move_dir = 0;
		if (inp.is_down('S'))					move_dir.z += 1;
		if (inp.is_down('W'))					move_dir.z -= 1;
		if (inp.is_down('A'))					move_dir.x -= 1;
		if (inp.is_down('D'))					move_dir.x += 1;
		if (inp.is_down(GLFW_KEY_LEFT_CONTROL))	move_dir.y -= 1;
		if (inp.is_down(' '))					move_dir.y += 1;

		flt speed = base_speed * (inp.is_down(GLFW_KEY_LEFT_SHIFT) ? fast_mult : 1);

		pos_world += (calc_cam_to_world(pos_world, altazimuth).m3() * (normalize_or_zero((v3)move_dir) * speed)) * dt;
	}

	void imgui (Save* save, cstr name) {
		if (imgui::TreeNodeEx("cam", ImGuiTreeNodeFlags_CollapsingHeader)) {

			imgui::DragFloat3("pos_world", &pos_world.x, 1.0f / 100);
			imgui::DragFloat3("altazimuth", &altazimuth.x, 1.0f / 100);

			imgui::SliderAngle("vfov", &vfov, 0,180);

			imgui::DragFloat("clip_near", &clip_near, 1.0f / 100);
			imgui::DragFloat("clip_far", &clip_far, 1.0f / 100);

			imgui::DragFloat("base_speed", &base_speed, 1.0f / 10);
			imgui::DragFloat("fast_mult", &fast_mult, 1.0f / 10);

			imgui::TreePop();
		}

		save->begin("cam");
		{
			save->value("pos_world", &pos_world);
			save->angle("altazimuth", &altazimuth);
		
			save->angle("vfov", &vfov);
		
			save->value("clip_near", &clip_near);
			save->value("clip_far", &clip_far);
		
			save->value("base_speed", &base_speed);
			save->value("fast_mult", &fast_mult);
		}
		save->end();
	}
};

std::vector<Default_Vertex_3d> gen_sphere (flt radius=1, iv2 lonlat_steps=iv2(64,32)) {
	std::vector<Default_Vertex_3d> mesh;

	struct Vert { v3 pos; v3 tangent; v2 uv; };
	
	auto vert = [&] (Vert v) {
		Default_Vertex_3d out;
		out.pos_model =			v.pos;
		out.tangent_model =		v4(v.tangent, 1);
		out.normal_model =		normalize(v.pos);
		out.uv =				v.uv;
		mesh.push_back(out);
	};
	auto quad = [&] (Vert a, Vert b, Vert c, Vert d) {
		vert(b);
		vert(c);
		vert(a);
		vert(a);
		vert(c);
		vert(d);
	};

	for (int lat=0; lat<lonlat_steps.y; ++lat) {

		flt lat0 = (flt)(lat +0) / (flt)lonlat_steps.y;
		flt lat1 = (flt)(lat +1) / (flt)lonlat_steps.y;

		m3 lat_rot0 = rotate3_Y(-lat0 * deg(180)); // rotate from -z to +x
		m3 lat_rot1 = rotate3_Y(-lat1 * deg(180));

		for (int lon=0; lon<lonlat_steps.x; ++lon) {

			flt lon0 = (flt)(lon +0) / (flt)lonlat_steps.x;
			flt lon1 = (flt)(lon +1) / (flt)lonlat_steps.x;

			m3 lon_rot0 = rotate3_Z(lon0 * deg(360));
			m3 lon_rot1 = rotate3_Z(lon1 * deg(360));

			quad({ lon_rot0 * (lat_rot0 * v3(0,0,-radius)), lon_rot0 * v3(0,1,0), v2(lon0,lat0) },
				 { lon_rot1 * (lat_rot0 * v3(0,0,-radius)), lon_rot1 * v3(0,1,0), v2(lon1,lat0) },
				 { lon_rot1 * (lat_rot1 * v3(0,0,-radius)), lon_rot1 * v3(0,1,0), v2(lon1,lat1) },
				 { lon_rot0 * (lat_rot1 * v3(0,0,-radius)), lon_rot0 * v3(0,1,0), v2(lon0,lat1) } );
		}
	}

	return mesh;
}

int main () {
	
	Window wnd;
	wnd.create("3D", iv2(1280,720), VSYNC_ON);
	
	Delta_Time_Measure dt_measure;
	f32 dt = dt_measure.begin();

	glEnable(GL_FRAMEBUFFER_SRGB);

	for (int frame_i=0;; ++frame_i) { // frame_i just for debugging
		
		static bool gui_input_enabled = true; // true: mouse visible, mouse can click on guis, keyboard input for imgui enabled,  false: mouselook mouse

		auto inp = wnd.poll_input(gui_input_enabled);

		if (wnd.wants_to_close())
			break;

		static bool imgui_enabled = true;
		if (inp.went_down(GLFW_KEY_F2))
			imgui_enabled = !imgui_enabled;
		if (inp.went_down(GLFW_KEY_F1))
			gui_input_enabled = !gui_input_enabled;
		
		begin_imgui(&inp, dt, gui_input_enabled, imgui_enabled);

		if (	(inp.buttons[GLFW_MOUSE_BUTTON_RIGHT].went_down && !imgui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) // unfocus imgui windows when right clicking outside of them
				|| frame_i < 3 || !imgui_enabled || !gui_input_enabled ) { // imgui steals focus on the third frame for some reason
			imgui::ClearActiveID();
			imgui::FocusWindow(NULL);
		}

		imgui::Begin("Debug");

		Save* save;
		{
			bool trigger_load = imgui::Button("Load") || inp.alt_combo(GLFW_KEY_L) || frame_i == 0;
			imgui::SameLine(0, 50);
			bool trigger_save = imgui::Button("Save") || inp.alt_combo(GLFW_KEY_S);
		
			save = save_file("saves/saves", trigger_load, trigger_save);
		}

		{
			static f32 frame_time_running_avg = dt;
			flt frame_time_running_avg_alpha = 0.1f;

			frame_time_running_avg = lerp(frame_time_running_avg, dt, frame_time_running_avg_alpha);

			imgui::Text("dt: %6.2f ms, fps: %6.2f", frame_time_running_avg * 1000, 1.0f / frame_time_running_avg);
		}

		shader_manager.poll_reload_shaders(frame_i);

		set_shared_uniform("common", "screen_size", (v2)inp.wnd_size_px);
		set_shared_uniform("common", "mcursor_pos", inp.mouse_cursor_pos_screen_buttom_up_pixel_center());

		// OpenGL debuggers like nsight consider y to be the up/down axis, so i store my skyboxes that way so they get displayed correctly in the debugger
		//  but since i use a z-up convention i convert between them
		set_shared_uniform("common", "cubemap_gl_ori_to_z_up", rotate3_X(-deg(90)));
		set_shared_uniform("common", "cubemap_z_up_to_gl_ori", rotate3_X(+deg(90)));

		static bool draw_wireframe = false;
		save->value("draw_wireframe", &draw_wireframe);
		imgui::Checkbox("draw_wireframe", &draw_wireframe);
		if (inp.went_down('P'))
			draw_wireframe = !draw_wireframe;

		set_shared_uniform("wireframe", "enable", draw_wireframe);

		// Camera
		static Camera cam;

		cam.control(inp, dt);
		cam.imgui(save, "cam");

		hm	world_to_cam = calc_world_to_cam(cam.pos_world, cam.altazimuth);
		hm	cam_to_world = calc_cam_to_world(cam.pos_world, cam.altazimuth);

		m4	cam_to_clip = calc_perspective_matrix(cam.vfov, cam.clip_near, cam.clip_far, (flt)inp.wnd_size_px.x / (flt)inp.wnd_size_px.y);
		
		set_shared_uniform("cam", "world_to_cam", world_to_cam.m4());
		set_shared_uniform("cam", "cam_to_world", cam_to_world.m4());
		set_shared_uniform("cam", "cam_to_clip", cam_to_clip);
		
		// Skybox
		static int selected_skybox = 7;
		static cstr skyboxes[] = {
			"generated skybox",
			"assets/ely_nevada/nevada_%s.tga",
			"assets/SkyboxSet1/CloudyLightRays/CloudyLightRays%s2048.png",
			"assets/SkyboxSet1/DarkStormy/DarkStormy%s2048.png",
			"assets/SkyboxSet1/FullMoon/FullMoon%s2048.png",
			"assets/SkyboxSet1/SunSet/SunSet%s2048.png",
			"assets/SkyboxSet1/ThickCloudsWater/ThickCloudsWater%s2048.png",
			"assets/SkyboxSet1/TropicalSunnyDay/TropicalSunnyDay%s2048.png",

			"assets/sIBL_Archive/Arches_E_PineTree/Arches_E_PineTree_3k.hdr",
			"assets/sIBL_Archive/Chiricahua_Plaza/GravelPlaza_REF.hdr",
		};
		
		static flt rotate_skybox = 0;
		
		static int target_skybox_res = 512;
		
		bool update_skybox = frame_i == 0;
		if (imgui::TreeNodeEx("skybox", ImGuiTreeNodeFlags_CollapsingHeader|ImGuiTreeNodeFlags_DefaultOpen)) {
			
			update_skybox = imgui::Combo("skybox", &selected_skybox, skyboxes, ARRLEN(skyboxes)) || update_skybox;
			save->value("selected_skybox", &selected_skybox);

			imgui::SliderAngle("rotate_skybox", &rotate_skybox, +180, -180);
			save->angle("rotate_skybox", &rotate_skybox);

			imgui::InputInt("skybox_res", &target_skybox_res) || update_skybox;
			save->value("skybox_res", &target_skybox_res);

			update_skybox = imgui::Button("update_skybox") || update_skybox;
			
			imgui::TreePop();
		}
		imgui::Separator();

		m3 world_to_skybox = rotate3_Z(-rotate_skybox);
		m3 skybox_to_world = rotate3_Z(rotate_skybox);

		v3 skyboxes_dir_to_sun_skybox[] = {
			rotate3_Z(deg(  85)) * rotate3_X(deg( 30)) * v3(0,1,0),
			rotate3_Z(deg(  98)) * rotate3_X(deg( 32)) * v3(0,1,0),
			rotate3_Z(deg(-127)) * rotate3_X(deg( 41)) * v3(0,1,0),
			rotate3_Z(deg(  91)) * rotate3_X(deg( 12)) * v3(0,1,0),
			rotate3_Z(deg(-110)) * rotate3_X(deg( 17)) * v3(0,1,0),
			rotate3_Z(deg(-115)) * rotate3_X(deg(  3)) * v3(0,1,0),
			rotate3_Z(deg(-146)) * rotate3_X(deg(  6)) * v3(0,1,0),
			rotate3_Z(deg( 160)) * rotate3_X(deg( 19)) * v3(0,1,0),
		};

		static struct {
			srgb8	col;
			flt		intensity;
		} skyboxes_sun_col[] = {
			{ srgb8(255,247,207), 0.50f },
			{ srgb8(255,234,226), 6.00f },
			{ srgb8(248,233,232), 0.31f },
			{ srgb8(216,207,207), 0.14f },
			{ srgb8(255,255,255), 0.37f },
			{ srgb8(243,157, 80), 1.71f },
			{ srgb8(248,191,152), 2.20f },
			{ srgb8(255,248,236), 6.00f },
		};

		set_shared_uniform("common", "world_to_skybox", world_to_skybox);

		v3 dir_to_sun_world = skybox_to_world * skyboxes_dir_to_sun_skybox[selected_skybox];
		lrgb skybox_light_radiance = skyboxes_sun_col[selected_skybox].col.to_lrgb() * skyboxes_sun_col[selected_skybox].intensity;

		set_shared_uniform("common", "skybox_light_dir_world", dir_to_sun_world);
		set_shared_uniform("common", "skybox_light_radiance", skybox_light_radiance);

		static TextureCube skybox;
		static iv2 skybox_res;

		if (std::string( skyboxes[selected_skybox] ).find("sIBL_Archive/") != std::string::npos) {

			if (update_skybox) {
				
				skybox = alloc_cube_texture(target_skybox_res, { PF_LRGBF, USE_MIPMAPS });

				auto fbo = create_fbo(skybox, target_skybox_res);

				auto s = use_shader("equirectangular_to_cubemap");
				assert(s);

				auto equirectangular_hdr = upload_texture_from_file(skyboxes[selected_skybox], { PF_LRGBF, USE_MIPMAPS });

				//
				bind_texture(s, "equirectangular", 0, equirectangular_hdr);

				draw_entire_cubemap(s, &fbo, target_skybox_res);

				skybox.generate_mipmaps();

			}
			
			skybox_res = target_skybox_res;

		} else if (selected_skybox > 0) {
			
			if (update_skybox) {
				skybox = upload_cube_texture_from_multifile(skyboxes[selected_skybox], { PF_SRGB8, USE_MIPMAPS },
						selected_skybox != 1 ?	std::vector<std::string>{"Left","Right","Down","Up","Front","Back"} :	std::vector<std::string>{"ft","bk","dn","up","rt","lf"},
						&skybox_res);
			}
		} else {
			// create a skybox via shader
			
			static FBO_Cube fbo;

			if (update_skybox) {
				skybox = alloc_cube_texture(target_skybox_res, { PF_LRGBF, USE_MIPMAPS });
				fbo = create_fbo(skybox, target_skybox_res);

			}

			{
				auto s = use_shader("skybox_cubemap_gen");
				assert(s);

				//
				set_uniform(s, "dir_to_sun", skyboxes_dir_to_sun_skybox[0]);

				draw_entire_cubemap(s, &fbo, target_skybox_res);

				skybox.generate_mipmaps();
			}
			
			skybox_res = target_skybox_res;
		}

		// PBR
		static int irradiance_res = 32;
		static int prefilter_res = 128;
		static int brdf_LUT_res = 512;
		
		static TextureCube irradiance;
		static TextureCube prefilter;
		static Texture2D brdf_LUT;
		{
			bool update_irradiance = frame_i == 0 || update_skybox;
			bool update_prefilter = frame_i == 0 || update_skybox;
			bool update_brdf_LUT = frame_i == 0;

			static int prefilter_sample_count = 1024;
			static int desired_prefilter_levels = 5;
			static int brdf_LUT_sample_count = 1024;

			static flt displacement_min_layers_count = 10;
			static flt displacement_max_layers_count = 10;

			if (imgui::TreeNodeEx("PBR", ImGuiTreeNodeFlags_CollapsingHeader|ImGuiTreeNodeFlags_DefaultOpen)) {
				
				imgui::InputInt("irradiance_res", &irradiance_res);
				imgui::InputInt("prefilter_res", &prefilter_res);
				imgui::InputInt("brdf_LUT_res", &brdf_LUT_res);

				imgui::InputInt("prefilter_sample_count", &prefilter_sample_count);
				imgui::InputInt("desired_prefilter_levels", &desired_prefilter_levels);
				imgui::InputInt("brdf_LUT_sample_count", &brdf_LUT_sample_count);

				update_irradiance =	imgui::Button("update irradiance_res") || update_irradiance;
				imgui::SameLine();
				update_prefilter =	imgui::Button("update prefilter_res") || update_prefilter;
				imgui::SameLine();
				update_brdf_LUT =	imgui::Button("update brdf_LUT_res") || update_brdf_LUT;

				imgui::DragFloat("displacement_min_layers_count", &displacement_min_layers_count, 1.0f / 20, 0);
				imgui::DragFloat("displacement_max_layers_count", &displacement_max_layers_count, 1.0f / 20, 0);

				imgui::TreePop();
			}
			{
				save->begin("PBR");

				save->value("irradiance_res", &irradiance_res);
				save->value("prefilter_res", &prefilter_res);
				save->value("brdf_LUT_res", &brdf_LUT_res);

				save->value("prefilter_sample_count", &prefilter_sample_count);
				save->value("desired_prefilter_levels", &desired_prefilter_levels);
				save->value("brdf_LUT_sample_count", &brdf_LUT_sample_count);

				save->value("displacement_min_layers_count", &displacement_min_layers_count);
				save->value("displacement_max_layers_count", &displacement_max_layers_count);

				save->end();
			}

			set_shared_uniform("displacement", "min_layers_count", displacement_min_layers_count);
			set_shared_uniform("displacement", "max_layers_count", displacement_max_layers_count);

			if (update_irradiance) {
				irradiance = alloc_cube_texture(irradiance_res, { PF_LRGBF, USE_MIPMAPS });

				auto fbo = create_fbo(irradiance, irradiance_res);

				auto s = use_shader("pbr_irradiance_convolve");
				assert(s);

				bind_texture(s, "source_radiance", 0, skybox);

				draw_entire_cubemap(s, &fbo, irradiance_res);

				irradiance.generate_mipmaps();
			}

			if (update_prefilter) {
				Texture::Options cubemap_opt = { PF_LRGBF, USE_MIPMAPS };

				prefilter = TextureCube::generate(cubemap_opt);
			
				auto s = use_shader("pbr_prefilter_specular");
				assert(s);

				set_uniform(s, "sample_count", prefilter_sample_count);
				
				std::vector<iv2> mip_sizes;
				mipmap_chain(prefilter_res, [&] (int mip, iv2 sz) { mip_sizes.push_back(sz); });

				int prefilter_levels = min(desired_prefilter_levels, (int)mip_sizes.size());

				for (int mip=0; mip<prefilter_levels; ++mip) {
					iv2 res = mip_sizes[mip];

					prefilter.alloc_mipmap(GL_TEXTURE_CUBE_MAP, res, mip, cubemap_opt);
				}

				prefilter.set_active_mips(GL_TEXTURE_CUBE_MAP, 0, prefilter_levels -1);

				set_shared_uniform("pbr", "prefilter_levels", (flt)prefilter_levels);

				for (int mip=0; mip<prefilter_levels; ++mip) {
					iv2 res = mip_sizes[mip];

					auto fbo = create_fbo(prefilter, res, mip);

					flt roughness = (flt)mip / (flt)(max(prefilter_levels -1, 1));

					set_uniform(s, "roughness", roughness);

					assert(skybox_res.x == skybox_res.y);

					bind_texture(s, "source_radiance", 0, skybox);
					set_uniform(s, "source_radiance_res", (flt)skybox_res.x);

					draw_entire_cubemap(s, &fbo, res);
				}

			}

			if (update_brdf_LUT) {
				brdf_LUT = alloc_texture(brdf_LUT_res, { PF_LRGBF, USE_MIPMAPS, FILTER_LINEAR, BORDER_CLAMP });

				auto fbo = create_fbo(brdf_LUT, brdf_LUT_res);

				inline_shader("pbr_brdf_LUT_precompute.vert", R"_SHAD(
					$include "common.vert"

					in		vec4	pos_clip;
					in		vec2	uv;

					out		vec2	vs_uv;

					void vert () {
						gl_Position =		pos_clip;
						vs_uv =				uv;
					}
				)_SHAD");

				auto s = use_shader("pbr_brdf_LUT_precompute");
				assert(s);

				set_uniform(s, "sample_count", brdf_LUT_sample_count);

				draw_to_texture(fbo, brdf_LUT_res);
				draw_fullscreen_quad(s);

				brdf_LUT.generate_mipmaps();
			}
		}

		//
		auto draw_scene = [&] (Texture2D* prev_framebuffer) {
			
			if (draw_wireframe) {
				lrgb clear_color_lrgb = srgb8(30).to_lrgb();

				glClearColor(clear_color_lrgb.x,clear_color_lrgb.y,clear_color_lrgb.z,255);

				glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			} else {
				glClear(GL_DEPTH_BUFFER_BIT);
			}

			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
			glEnable(GL_DEPTH_CLAMP);

			{
				auto s = use_shader("skybox");
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

				bind_texture(s, "skybox", 4, skybox);

				draw_triangles(*s, 0, (int)cube.size());
			}
			glDisable(GL_DEPTH_CLAMP);

			glDisable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			glDisable(GL_SCISSOR_TEST);

			static auto sphere_mesh = gen_sphere();
			
			{
				auto shad = use_shader(shad_default_3d);
				if (shad) {
					
					static auto vbo = stream_vertex_data(sphere_mesh.data(), (int)sphere_mesh.size() * Default_Vertex_3d::layout.vertex_size);
					
					bind_texture(shad, "skybox", 4, skybox);
					bind_texture(shad, "irradiance", 5, irradiance);

					{
						static v3 srgb_albedo = (v3(1.000f, 0.766f, 0.336f));
						imgui::ColorEdit3("spheres.albedo", &srgb_albedo.x);
						save->value("spheres.albedo", &srgb_albedo);

						static int metallic_steps = 3;
						static int roughness_steps = 7;
						imgui::DragInt("spheres.metallic_steps", &metallic_steps, 1.0f / 25);
						imgui::DragInt("spheres.roughness_steps", &roughness_steps, 1.0f / 25);
						save->value("metallic_steps", &metallic_steps);
						save->value("roughness_steps", &roughness_steps);

						for (int m=0; m<metallic_steps; ++m) {
							for (int r=0; r<roughness_steps; ++r) {

								hm model_to_world = translateH(v3(-5,0,0)) * rotateH_Z(deg(30)) * translateH(v3((flt)r * 1.5f, 0, (flt)m * 1.5f)) * scaleH(0.5f);

								set_uniform(shad, "model_to_world", model_to_world.m4());

								set_uniform(shad, "uv_scale", v2(1));

								set_material_albedo(shad, lrgba(to_linear(srgb_albedo), 1));

								flt roughness = (flt)r / (flt)(roughness_steps -1);
								roughness = roughness * roughness;

								set_material_metallic(shad,	(flt)m / (flt)(metallic_steps -1));
								set_material_roughness(shad, roughness);
								set_material_normal_identity(shad);
								set_material_ao_identity(shad);
								set_material_displacement_identity(shad);

								bind_texture(shad, "irradiance", 6, irradiance);
								bind_texture(shad, "prefilter", 7, prefilter);
								bind_texture(shad, "brdf_LUT", 8, brdf_LUT);

								use_vertex_data(*shad, Default_Vertex_3d::layout, vbo);

								draw_triangles(*shad, 0,(int)sphere_mesh.size());
							}
						}
					}

					struct Sphere {
						std::string	name;

						v3			pos = 0;
						flt			ori = 0;
						flt			radius = 0.5f;


						std::string	folder = "assets/";
						// if empty use values below (identity for normal)
						std::string	albedo_tex;
						std::string	metallic_tex;
						std::string	roughness_tex;
						std::string	normal_tex;
						std::string	ao_tex;
						std::string	height_tex;
						bool		normal_flip_y = true;

						flt			height_depth = 0.1f;

						v3			albedo_srgb = 1;
						flt			metallic;
						flt			roughness = 0.2f;

						v2			uv_scale = v2(4,2);

						bool		imgui_open = false;

						bool imgui (uptr spheres_count) {
							bool delete_sphere = false;

							imgui::SetNextTreeNodeOpen(imgui_open);
							imgui_open = imgui::TreeNode((void*)(((uptr)this << 16) + spheres_count), name.c_str());
							if (!imgui_open)
								return delete_sphere;

							if (imgui::BeginPopupContextItem("context menu")) {
								delete_sphere = imgui::Button("Delete");
								ImGui::EndPopup();
							}

							imgui::InputText_str("name", &name);

							flt w = imgui::GetContentRegionAvailWidth();

							imgui::DragFloat3("pos", &pos.x, 1.0f / 25);

							flt ori_deg = to_deg(ori);
							imgui::DragFloat("ori", &ori_deg, 360.0f / 200);
							ori = to_rad(ori_deg);

							imgui::DragFloat("radius", &radius, 1.0f / 25);

							imgui::InputText_str("folder", &folder);

							imgui::InputText_str("albedo_tex", &albedo_tex);
							imgui::InputText_str("metallic_tex", &metallic_tex);
							imgui::InputText_str("roughness_tex", &roughness_tex);
							imgui::InputText_str("normal_tex", &normal_tex);
							imgui::SameLine();
							imgui::Checkbox("normal_flip_y", &normal_flip_y);
							imgui::InputText_str("ao_tex", &ao_tex);
							imgui::InputText_str("height_tex",		&height_tex);

							imgui::DragFloat("height_depth", &height_depth, 0.1f / 100);

							imgui::ColorEdit3("albedo", &albedo_srgb.x);
							imgui::SliderFloat("metallic", &metallic, 0,1);
							imgui::SliderFloat("roughness", &roughness, 0,1);

							imgui::DragFloat2("uv_scale", &uv_scale.x, 1.0f / 200);

							imgui::TreePop();

							return delete_sphere;
						}
						void save (Save* save) {
							
							save->value("name",				&name);
							save->value("pos",				&pos);
							save->angle("ori",				&ori);
							save->value("radius",			&radius);
							
							save->value("folder",			&folder);
							save->value("albedo_tex",		&albedo_tex);
							save->value("metallic_tex",		&metallic_tex);
							save->value("roughness_tex",	&roughness_tex);
							save->value("normal_tex",		&normal_tex);
							save->value("normal_flip_y",	&normal_flip_y);
							save->value("ao_tex",			&ao_tex);
							save->value("height_tex",		&height_tex);

							save->value("height_depth",		&height_depth);
							
							save->value("albedo_srgb",		&albedo_srgb);
							save->value("metallic",			&metallic);
							save->value("roughness",		&roughness);

							save->value("uv_scale",			&uv_scale);

						}
					};
					static std::vector<Sphere> spheres;

					if (imgui::TreeNodeEx("spheres", ImGuiTreeNodeFlags_DefaultOpen)) {

						for (auto it=spheres.begin(); it!=spheres.end();) {

							bool delete_it = it->imgui(spheres.size());
							
							if (delete_it)
								it = spheres.erase(it);
							else
								++it;
						}

						if (imgui::Button("Add"))
							spheres.emplace_back();

						imgui::TreePop();
					}

					save->begin_array("spheres", &spheres);
					for (auto& s : spheres) {
						save->begin("sphere");
						s.save(save);
						save->end();
					}
					save->end_array();

					for (auto& s : spheres) {
						
						hm model_to_world = translateH(s.pos) * rotateH_Z(s.ori) * scaleH(s.radius);

						set_uniform(shad, "model_to_world", model_to_world.m4());

						set_uniform(shad, "uv_scale", s.uv_scale);

						if (s.albedo_tex.size() > 0)	set_material_albedo(shad, *get_texture(		s.folder + s.albedo_tex, { PF_SRGB8 }));
						else							set_material_albedo(shad, lrgba(to_linear(s.albedo_srgb), 1));

						if (s.metallic_tex.size() > 0)	set_material_metallic(shad, *get_texture(	s.folder + s.metallic_tex, { PF_LRGB8 }));
						else							set_material_metallic(shad, s.metallic);

						if (s.roughness_tex.size() > 0)	set_material_roughness(shad, *get_texture(	s.folder + s.roughness_tex, { PF_LRGB8 }));
						else							set_material_roughness(shad, s.roughness);

						if (s.normal_tex.size() > 0)	set_material_normal(shad, *get_texture(		s.folder + s.normal_tex, { PF_LRGB8 }), s.normal_flip_y);
						else							set_material_normal_identity(shad);

						if (s.ao_tex.size() > 0)		set_material_ao(shad, *get_texture(			s.folder + s.ao_tex, { PF_LRGB8 }));
						else							set_material_ao_identity(shad);

						if (s.height_tex.size() > 0)	set_material_displacement(shad, *get_texture(		s.folder + s.height_tex, { PF_LRGB8 }), s.height_depth);
						else							set_material_displacement_identity(shad);

						bind_texture(shad, "irradiance", 6, irradiance);
						bind_texture(shad, "prefilter", 7, prefilter);
						bind_texture(shad, "brdf_LUT", 8, brdf_LUT);

						use_vertex_data(*shad, Default_Vertex_3d::layout, vbo);
						
						draw_triangles(*shad, 0,(int)sphere_mesh.size());

						if (s.height_tex.size() > 0) {
							hm model_to_world = translateH(s.pos +v3(0,0,1.5f)) * rotate3_Z(deg(-90) +s.ori) * rotate3_X(deg(90)) * scaleH(1);
						
							set_uniform(shad, "model_to_world", model_to_world.m4());

							set_uniform(shad, "uv_scale", s.uv_scale * v2(0.5f,1));

							std::vector<Default_Vertex_3d> quad;
							for (auto p : quad_verts) {
								Default_Vertex_3d v;
								v.pos_model = v3(p -0.5f, +0.5f);
								v.normal_model = v3(0,0,1);
								v.tangent_model = v4(1,0,0,1);
								v.uv = p;
								quad.push_back(v);
							}

							draw_stream_triangles(*shad, quad);
						}
					}
				}
			}
			imgui::Separator();

			static bool draw_cerberus = true;
			imgui::Checkbox("draw_cerberus", &draw_cerberus);
			save->value("draw_cerberus", &draw_cerberus);
			if (draw_cerberus) {
				auto cerberus = mesh_manager.get_mesh("assets/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");

				auto s = use_shader(shad_default_3d);
				assert(s);

				static VBO vbo = stream_vertex_data(cerberus->vbo_data.data(), (int)cerberus->vbo_data.size() * (int)sizeof(Default_Vertex_3d));
				use_vertex_data(*s, Default_Vertex_3d::layout, vbo);

				set_uniform(s, "model_to_world", translate4(v3(+5,0,0)) * scale4(1.0f / 50));

				set_material_albedo(s,		*get_texture("assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga", { PF_SRGB8 }));
				set_material_metallic(s,	*get_texture("assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga", { PF_LRGB8 }));
				set_material_roughness(s,	*get_texture("assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga", { PF_LRGB8 }));
				set_material_normal(s,		*get_texture("assets/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga", { PF_LRGB8 }));
				set_material_ao_identity(s);
				set_material_displacement_identity(s);

				bind_texture(s, "irradiance", 6, irradiance);
				bind_texture(s, "prefilter", 7, prefilter);
				bind_texture(s, "brdf_LUT", 8, brdf_LUT);

				set_uniform(s, "uv_scale", v2(1));

				draw_triangles(*s, 0, (int)cerberus->vbo_data.size());
			}

			static bool draw_terrain = 0;
			imgui::Checkbox("draw_terrain", &draw_terrain);
			if (draw_terrain) {
				auto mesh = mesh_manager.get_mesh("assets/terrain.fbx");

				auto s = use_shader(shad_default_3d);
				assert(s);

				static VBO vbo = stream_vertex_data(mesh->vbo_data.data(), (int)mesh->vbo_data.size() * (int)sizeof(Default_Vertex_3d));
				use_vertex_data(*s, Default_Vertex_3d::layout, vbo);

				set_uniform(s, "model_to_world", translate4(v3(0,0,0)) * scale4(500));

				set_material_albedo(s,		lwhite);
				set_material_metallic(s,	0);
				set_material_roughness(s,	0.8f);
				set_material_normal_identity(s);
				set_material_ao_identity(s);
				set_material_displacement_identity(s);

				bind_texture(s, "irradiance", 6, irradiance);
				bind_texture(s, "prefilter", 7, prefilter);
				bind_texture(s, "brdf_LUT", 8, brdf_LUT);

				set_uniform(s, "uv_scale", v2(1));

				draw_triangles(*s, 0, (int)mesh->vbo_data.size());
			}

			for (int face=0; face<6; ++face) {

				std::vector<Default_Vertex_3d> quad;
				for (auto p : quad_verts) {
					Default_Vertex_3d v;
					v.pos_model = v3(p -0.5f, +0.5f);
					v.normal_model = v3(0,0,1);
					v.tangent_model = v4(1,0,0,1);
					v.uv = p;
					v.col_lrgba = srgba8(230).to_lrgb();
					quad.push_back(v);
				}

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
\
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
					set_material_ao_identity(s);
					set_material_displacement_identity(s);

					bind_texture(s, "irradiance", 6, irradiance);
					bind_texture(s, "prefilter", 7, prefilter);
					bind_texture(s, "brdf_LUT", 8, brdf_LUT);

					set_uniform(s, "uv_scale", v2(1));

					draw_stream_triangles(*s, quad);
				}
			}

		};

		static bool render_to_framebuffer_and_use_as_texture = false;
		imgui::Checkbox("render_to_framebuffer_and_use_as_texture", &render_to_framebuffer_and_use_as_texture);

		if (!render_to_framebuffer_and_use_as_texture) {
			
			draw_to_screen(inp.wnd_size_px);

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
					_framebuffer->fbo = create_fbo(_framebuffer->tex, _framebuffer->size);
				}

				draw_to_texture(_framebuffer->fbo, inp.wnd_size_px);

				draw_scene(prev_framebuffer);

				framebuffer = &_framebuffer->tex;
				prev_framebuffer = framebuffer;

				_framebuffer = &doublebuffer_framebuffers[ (_framebuffer -&doublebuffer_framebuffers[0]) ^ 1 ];
			}

			{ // draw framebuffer as fullscreen quad
				draw_to_screen(inp.wnd_size_px);

				{
					static bool test_inline_shader_reloading = false;
					imgui::Checkbox("test_inline_shader_reloading", &test_inline_shader_reloading);

					inline_shader("fullscreen_quad.vert", R"_SHAD(
						$include "common.vert"
					
						in		vec4	pos_clip;
						in		vec2	uv;
					
						out		vec2	vs_uv;
					
						void vert () {
							gl_Position =		pos_clip;
							vs_uv =				uv;
						}
					)_SHAD");
					inline_shader("fullscreen_quad.frag", prints(R"_SHAD(
						$include "common.frag"
					
						in		vec2	vs_uv;
					
						uniform sampler2D tex;
					
						vec4 frag () {
							return %d != 0 ? vec4(0,1,0,1) : texture(tex, vs_uv);
						}
					)_SHAD", test_inline_shader_reloading && (frame_i / 60) % 2)); // That this just works is crazy

					auto s = use_shader("fullscreen_quad");
					assert(s);

					bind_texture(s, "tex", 0, *framebuffer);

					draw_fullscreen_quad(s);
				}
			}

		}
		
		imgui_texture_windows();

		draw_to_screen(inp.wnd_size_px);

		save->end_frame();

		imgui::End();
		//imgui::ShowDemoWindow();
		end_imgui(inp.wnd_size_px, imgui_enabled);

		wnd.swap_buffers();
		
		dt = dt_measure.frame();
	}

	return 0;
}
