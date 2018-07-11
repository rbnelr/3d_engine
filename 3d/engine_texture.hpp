#pragma once

#define STB_IMAGE_IMPLEMENTATION

#define STB_IMAGE_ONLY_BMP
#define STB_IMAGE_ONLY_PNG
#define STB_IMAGE_ONLY_JPG
#define STB_IMAGE_ONLY_HDR

#include "deps/stb/stb_image.h"

#include "image_processing.hpp"

namespace engine {
namespace texture_n {
//

enum pixel_format_e {
	PF_SRGB8,
	PF_SRGBA8, // alpha is always linear in opengl
	
	PF_LR8,
	PF_LRGB8,
	PF_LRGBA8,

	PF_LRF,
	PF_LRGBF,
	PF_LRGBAF,
};
enum mipmap_mode_e {
	USE_MIPMAPS=0,
	NO_MIPMAPS,
};
enum minmag_filter_e {
	FILTER_NEAREST=0,
	FILTER_LINEAR,
};
enum border_mode_e {
	BORDER_REPEAT=0,
	BORDER_CLAMP,
	BORDER_COLOR,
};

template <typename FOREACH> int mipmap_chain (iv2 initial_size, FOREACH f) {
	int mip_i = 0;
	iv2 sz = initial_size;

	for (;; ++mip_i) {
		f(mip_i, sz);

		if (all(sz == 1))
			break;

		sz = max(sz / 2, 1);
	}
	return mip_i;
}

// minimal but still useful texture class
// if you need the size of the texture or the mipmap count just create a wrapper class
class Texture {
	MOVE_ONLY_CLASS(Texture)

	GLuint	handle = 0;

public:
	~Texture () {
		if (handle) // maybe this helps to optimize out destructing of unalloced textures
			glDeleteTextures(1, &handle); // would be ok to delete unalloced texture (gpu_handle = 0)
	}
	
	GLuint	get_handle () const {	return handle; }

	struct Options {
		pixel_format_e	pixel_format	;//: 3;
		mipmap_mode_e	mipmap_mode		;//: 1;
		minmag_filter_e	minmag_filter	;//: 1;
		border_mode_e	border_mode		;//: 1;
		lrgba			border_color;

		constexpr Options (pixel_format_e pf, mipmap_mode_e mm=USE_MIPMAPS, minmag_filter_e mf=FILTER_LINEAR, border_mode_e bm=BORDER_REPEAT, lrgba bc=lrgba(0,0,0,1)):
			pixel_format{pf}, mipmap_mode{mm}, minmag_filter{mf}, border_mode{bm}, border_color{bc} {}

		bool operator== (Options r) {
			return	(pixel_format		== r.pixel_format)	&&	
					(mipmap_mode		== r.mipmap_mode)	&&	
					(minmag_filter		== r.minmag_filter)	&&
					(border_mode		== r.border_mode)	&&	
					all(border_color	== r.border_color)	;
		}
	};

	void generate (GLenum target, Options const& o) {
		assert(handle == 0);
		
		glGenTextures(1, &handle);

		glBindTexture(target, handle);
		glBindTexture(target, 0);

		set_minmag_filtering(target, o.minmag_filter, o.mipmap_mode);
		set_border(target, o.border_mode, o.border_color);

		if (o.mipmap_mode == NO_MIPMAPS) // NOTE: If i set the active mips to the one we upload (mip 0) (default is 0,1000) then glGenerateMipmap does not work
			set_active_mips(target, 0,0);
	}

private:
	void set_minmag_filtering (GLenum target, minmag_filter_e mf, mipmap_mode_e mm) {
		glBindTexture(target, handle);

		GLenum min, mag;

		if (		mf == FILTER_LINEAR && mm == USE_MIPMAPS ) {
			min = GL_LINEAR_MIPMAP_LINEAR;	mag = GL_LINEAR;
		} else if (	mf == FILTER_LINEAR && mm == NO_MIPMAPS ) {
			min = mag = GL_LINEAR;
		} else if (	mf == FILTER_NEAREST ) {
			min = mag = GL_NEAREST;
		} else {
			assert(not_implemented);
		}

		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag);

		set_filtering_anisotropic(target, mf == FILTER_LINEAR);
	}
	void set_filtering_anisotropic (GLenum target, bool enabled) {
		if (GL_ARB_texture_filter_anisotropic) {
			GLfloat aniso;
			if (enabled) {
				GLfloat max_aniso;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_aniso);
				aniso = max_aniso;
			} else {
				aniso = 1;
			}
			glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY, aniso);
		}
	}

	void set_border (GLenum target, border_mode_e bm, lrgba bc) {
		glBindTexture(target, handle);

		if (		target == GL_TEXTURE_2D ) {

			GLenum mode;
		
			switch (bm) {
				case BORDER_REPEAT:	mode = GL_REPEAT;			break;
				case BORDER_CLAMP:	mode = GL_CLAMP_TO_EDGE;	break;
				case BORDER_COLOR:	mode = GL_CLAMP_TO_BORDER;
				default: assert(not_implemented);
			}

			glTexParameteri(target, GL_TEXTURE_WRAP_S, mode);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, mode);

			if (bm == BORDER_COLOR)
				glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, &bc.x); // according to https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_sRGB.txt, the texture border is not converted from srgb to linear even for srgb textures (ie. is treated as linear in a linear pipeline), i could not find other info on this
			

		} else if (	target == GL_TEXTURE_CUBE_MAP ) {
			// wrapping modes are irrelevant for cubemaps

			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // redundant state set

		} else {
			assert(not_implemented);
		}
	}
	
	void set_active_mips (GLenum target, int first, int last) { // i am not sure that just setting abitrary values for these actually works correctly
		glBindTexture(target, handle);

		glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, first);
		glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, last);
	}

public:
	
	void generate_mipmaps (GLenum target) {
		glBindTexture(target, handle);

		glGenerateMipmap(target); // NOTE: If i set the active mips to the one we upload (mip 0) (default is 0,1000) then glGenerateMipmap does not work
	}

	struct GL_Format {
		GLenum internal_format;
		GLenum cpu_format;
		GLenum type;
	} _format (pixel_format_e pf) {
		switch (pf) {
			case PF_SRGB8:	return { GL_SRGB8,			GL_RGB,		GL_UNSIGNED_BYTE };
			case PF_SRGBA8:	return { GL_SRGB8_ALPHA8,	GL_RGBA,	GL_UNSIGNED_BYTE };
			
			case PF_LR8:	return { GL_R8,				GL_RED,		GL_UNSIGNED_BYTE };
			case PF_LRGB8:	return { GL_RGB8,			GL_RGB,		GL_UNSIGNED_BYTE };
			case PF_LRGBA8:	return { GL_RGBA8,			GL_RGBA,	GL_UNSIGNED_BYTE };
			
			case PF_LRF:	return { GL_R32F,			GL_RED,		GL_FLOAT };		
			case PF_LRGBF:	return { GL_RGB32F,			GL_RGB,		GL_FLOAT };		
			case PF_LRGBAF:	return { GL_RGBA32F,		GL_RGBA,	GL_FLOAT };		
			default: assert(not_implemented);
				return {};
		}
	}
	void reupload_mipmap_2d (void const* pixels, iv2 size_px, int mip, Options o) {
		auto f = _format(o.pixel_format);
		glTexImage2D(GL_TEXTURE_2D, mip, f.internal_format, size_px.x,size_px.y, 0, f.cpu_format, f.type, pixels);
	}
	void reupload_mipmap_cube_face (void const* pixels, iv2 size_px, int cube_face, int mip, Options o) {
		auto f = _format(o.pixel_format);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +cube_face, mip, f.internal_format, size_px.x,size_px.y, 0, f.cpu_format, f.type, pixels);
	}

	void reupload_mipmap (GLenum target, void const* pixels, iv2 size_px, int mip, Options o) { // pixels for cubemap faces are contiguous
		glBindTexture(target, handle);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (		target == GL_TEXTURE_2D ) {
			reupload_mipmap_2d(pixels, size_px, mip, o);
		} else if (	target == GL_TEXTURE_CUBE_MAP ) {
			for (int face=0; face<6; ++face)
				reupload_mipmap_cube_face(pixels, size_px, face, mip, o);
		} else {
			assert(not_implemented);
		}

		glBindTexture(target, 0);
	}
	void reupload_mipmap_cube (std::vector<void const*> face_pixels, iv2 size_px, int mip, Options o) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		for (int face=0; face<6; ++face)
			reupload_mipmap_cube_face(face_pixels[face], size_px, face, mip, o);
	}

	void alloc_mipmap (GLenum target, iv2 size_px, int mip, Options o) {
		reupload_mipmap(target, nullptr, size_px, mip, o);
	}

	void reupload (GLenum target, void const* pixels, iv2 size_px, Options o) { // pixels for cubemap faces are contiguous
		reupload_mipmap(target, pixels, size_px, 0, o);

		if (o.mipmap_mode == USE_MIPMAPS) {
			generate_mipmaps(target);
		}
	}
	void reupload_cube (std::vector<void const*> face_pixels, iv2 size_px, Options o) {
		reupload_mipmap_cube(face_pixels, size_px, 0, o);

		if (o.mipmap_mode == USE_MIPMAPS) {
			generate_mipmaps(GL_TEXTURE_CUBE_MAP);
		}
	}

	void reupload (GLenum target, std::vector<void const*> mipmap_pixels, iv2 size_px, Options o) {
		assert(o.mipmap_mode == USE_MIPMAPS);
		
		int mips = mipmap_chain(size_px, [&] (int mip, iv2 mip_size_px) {
			reupload_mipmap(target, mipmap_pixels[mip], mip_size_px, mip, o);
		});
		set_active_mips(target, 0, mips-1);
	}
	void reupload_cube (std::vector< std::vector<void const*> > mipmap_face_pixels, iv2 size_px, Options o) {
		assert(o.mipmap_mode == USE_MIPMAPS);

		int mips = mipmap_chain(size_px, [&] (int mip, iv2 mip_size_px) {
			reupload_mipmap_cube(mipmap_face_pixels[mip], mip_size_px, mip, o);
		});
		set_active_mips(GL_TEXTURE_CUBE_MAP, 0, mips-1);
	}

	void alloc (GLenum target, iv2 size_px, Options o) {
		if (o.mipmap_mode == USE_MIPMAPS) {
			int mips = mipmap_chain(size_px, [&] (int mip, iv2 mip_size_px) {
				alloc_mipmap(target, size_px, 0, o);
			});
			set_active_mips(target, 0, mips-1);
		} else {
			alloc_mipmap(target, size_px, 0, o);
		}
	}
	
};
void swap (Texture& l, Texture& r) {
	std::swap(l.handle, r.handle);
}

enum texture_type_e {
	TEXTURE_2D,
	TEXTURE_CUBE,
};

class Texture2D : public Texture {
	using Texture::generate;
public:
	static Texture2D generate (Texture::Options o) {
		Texture2D tex;
		tex.generate(GL_TEXTURE_2D, o);
		return tex;
	}
};
class TextureCube : public Texture {
	using Texture::generate;
public:
	static TextureCube generate (Texture::Options o) {
		TextureCube tex;
		tex.generate(GL_TEXTURE_CUBE_MAP, o);
		return tex;
	}
};

Texture2D upload_texture (void const* pixels, iv2 size_px, Texture::Options o) {
	auto tex = Texture2D::generate(o);
	tex.reupload(GL_TEXTURE_2D, pixels, size_px, o);
	return tex;
}

Texture2D alloc_texture (iv2 size_px, Texture::Options o) {
	auto tex = Texture2D::generate(o);
	tex.alloc(GL_TEXTURE_2D, size_px, o);
	return tex;
}

TextureCube alloc_cube_texture (iv2 size_px, Texture::Options o) {
	auto tex = TextureCube::generate(o);
	tex.alloc(GL_TEXTURE_CUBE_MAP, size_px, o);
	return tex;
}

void* get_image2d_file_pixel (std::string const& filepath, Texture::Options o, iv2* size_px, int* sizeof_pixel=nullptr) {
	int channels;
	int got_channels;
	bool hdr = false;

	switch (o.pixel_format) {
		case PF_SRGB8:	channels = 3;	break;
		case PF_SRGBA8:	channels = 4;	break;

		case PF_LR8:	channels = 1;	break;
		case PF_LRGB8:	channels = 3;	break;
		case PF_LRGBA8:	channels = 4;	break;

		case PF_LRF:	channels = 1;	hdr = true;	break;
		case PF_LRGBF:	channels = 3;	hdr = true;	break;
		case PF_LRGBAF:	channels = 4;	hdr = true;	break;

		default: assert(not_implemented);
	}

	assert((stbi_is_hdr(filepath.c_str()) != 0) == hdr); // could load hdr as 8 bit ldr and vice versa, but just enforce fitting pixel formats for now

	stbi_set_flip_vertically_on_load(true);

	void* pixels;

	if (!hdr) {

		pixels = stbi_load(filepath.c_str(), &size_px->x,&size_px->y, &got_channels, channels);
		if (!pixels) {
			fprintf(stderr, "Texture \"%s\" could not be loaded!\n", filepath.c_str());
			return nullptr;
		}

		if (sizeof_pixel) *sizeof_pixel = channels * sizeof(u8);

	} else {
		
		pixels = stbi_loadf(filepath.c_str(), &size_px->x,&size_px->y, &got_channels, channels);
		if (!pixels) {
			fprintf(stderr, "Texture \"%s\" could not be loaded!\n", filepath.c_str());
			return nullptr;
		}

		if (sizeof_pixel) *sizeof_pixel = channels * sizeof(u8);

	}
	return pixels;
}

Texture2D upload_texture_from_file (std::string const& filepath, Texture::Options o, iv2* out_size_px=nullptr) {
	
	iv2 size_px;
	auto* pixels = get_image2d_file_pixel(filepath, o, &size_px);
	if (!pixels) {
		return Texture2D();
	}

	auto tex = upload_texture(pixels, size_px, o);

	free(pixels);

	*out_size_px = size_px;
	return tex;
}

enum _90_deg_rotation {
	ROT_0=0,
	ROT_90,
	ROT_180,
	ROT_270,
};

TextureCube upload_cube_texture_from_multifile (std::string const& name_format, Texture::Options o, std::vector<std::string> const& face_names, iv2* out_size_px=nullptr) {
	
	auto tex = TextureCube::generate(o);

	int sizeof_pixel;
	iv2 prev_size_px = -1;
	
	for (int face=0; face<6; ++face) {
		iv2 size_px;
		auto* pixels = get_image2d_file_pixel(prints(name_format.c_str(), face_names[face].c_str()), o, &size_px, &sizeof_pixel);
		if (!pixels || !(prev_size_px.x < 0 || all(prev_size_px == size_px))) {
			return TextureCube();
		}

		prev_size_px = size_px;

		tex.reupload_mipmap_cube_face(pixels, size_px, face, 0, o);

		free(pixels);
	}

	if (o.mipmap_mode == USE_MIPMAPS) {
		tex.generate_mipmaps(GL_TEXTURE_CUBE_MAP);
	}

	*out_size_px = prev_size_px;
	return tex;
}

void imgui_texture_window_register_texture (std::string const& name);

struct Any_Texture {
	texture_type_e		type;
	union {
		Texture2D		tex2d;
		TextureCube		tex_cube;
	};
	iv2					size_px;
	Texture::Options	o;

	Any_Texture (Texture2D t, iv2 sz, Texture::Options o):		type{TEXTURE_2D},	tex2d{std::move(t)},	size_px{sz}, o{o} {}
	Any_Texture (TextureCube t, iv2 sz, Texture::Options o):	type{TEXTURE_CUBE},	tex_cube{std::move(t)},	size_px{sz}, o{o} {}

	~Any_Texture () {
		switch (type) { // destruct active element in union
		case TEXTURE_2D:	std::move(tex2d);		break;
		case TEXTURE_CUBE:	std::move(tex_cube);	break;
		default:	assert(not_implemented);
		}
	}
};

struct Texture_Manager {
	
	std::unordered_map<std::string, unique_ptr<Any_Texture>> textures;

	Any_Texture* find_texture (std::string const& name) {
		auto tex = textures.find(name);
		return tex == textures.end() ? nullptr : tex->second.get();
	}

	Texture2D* get_texture (std::string const& name, Texture::Options o) {
		auto tex = find_texture(name);
		if (!tex) {
			iv2 size_px;
			auto tmp = upload_texture_from_file(name, o, &size_px);
			tex = textures.emplace(name, make_unique<Any_Texture>(std::move(tmp), size_px, o)).first->second.get();
		}

		assert(tex->type == TEXTURE_2D && tex->o == o);
		return &tex->tex2d;
	}
	
	TextureCube* get_multifile_cubemap (std::string const& name, Texture::Options o, std::vector<std::string> const& face_names) {
		auto tex = find_texture(name);
		if (!tex) {
			iv2 size_px;
			auto tmp = upload_cube_texture_from_multifile(name, o, face_names, &size_px);
			tex = textures.emplace(name, make_unique<Any_Texture>(std::move(tmp), size_px, o)).first->second.get();
		}

		assert(tex->type == TEXTURE_CUBE && tex->o == o);
		return &tex->tex_cube;
	}

	bool evict_texture (std::string const& name) {
		auto shad = textures.find(name);
		if (shad == textures.end()) {
			return false;
		}
		
		textures.erase(shad);
		return true;
	}
};

Texture_Manager texture_manager;

Texture2D* get_texture (std::string const& name, Texture::Options o) {
	return texture_manager.get_texture(name, o);
}

TextureCube* get_multifile_cubemap (std::string const& name_format, Texture::Options o, std::vector<std::string> const& face_names) {
	return texture_manager.get_multifile_cubemap(name_format, o, face_names);
}
bool evict_texture (std::string const& name) {
	return texture_manager.evict_texture(name);
}


Texture2D upload_texture (lrgba color) {
	Texture::Options o = { PF_LRGBAF, NO_MIPMAPS, FILTER_NEAREST, BORDER_CLAMP };
	auto tex = Texture2D::generate(o);
	tex.reupload(GL_TEXTURE_2D, &color, 1, o);
	return tex;
}
Texture2D upload_texture (srgba8 color) {
	Texture::Options o = { PF_SRGBA8, NO_MIPMAPS, FILTER_NEAREST, BORDER_CLAMP };
	auto tex = Texture2D::generate(o);
	tex.reupload(GL_TEXTURE_2D, &color, 1, o);
	return tex;
}

Texture2D* tex_black () {			static Texture2D tex = upload_texture(lrgba(0,0,0,1));			return &tex; }
Texture2D* tex_grey () {			static Texture2D tex = upload_texture(lrgba(0.5f,0.5f,0.5f,1));	return &tex; }
Texture2D* tex_white () {			static Texture2D tex = upload_texture(lrgba(1));				return &tex; }
Texture2D* tex_identity_normal () {	static Texture2D tex = upload_texture(lrgba(0.5f,0.5f,1,1));	return &tex; }

//
}
using namespace texture_n;
}
