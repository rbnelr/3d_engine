#pragma once

#define STB_IMAGE_IMPLEMENTATION

#define STB_IMAGE_ONLY_BMP
#define STB_IMAGE_ONLY_PNG
#define STB_IMAGE_ONLY_JPG

#include "deps/stb/stb_image.h"

namespace engine {
namespace texture_n {
//

enum pixel_format_e {
	// linear
	PF_LR8,
	PF_LRGBA8,
	PF_SRGBA8, // alpha is always linear in opengl

	PF_LRF,
	PF_LRGBF,
	PF_LRGBAF,
};
enum mipmap_mode_e {
	NO_MIPMAPS,
	GEN_MIPMAPS,
};
enum minmag_filter_e {
	FILTER_NEAREST,
	FILTER_BILINEAR,
	FILTER_TRILINEAR,
};
enum aniso_filter_e {
	FILTER_NO_ANISO,
	FILTER_MAX_ANISO,
};
enum border_e {
	BORDER_CLAMP,
	BORDER_COLOR,
};

// minimal but still useful texture class
// if you need the size of the texture or the mipmap count just create a wrapper class
class Texture2D {
	friend void swap (Texture2D& l, Texture2D& r);

	GLuint	handle = 0;

public:

	// default alloc unalloced texture
	Texture2D () {}
	// auto free texture
	~Texture2D () {
		if (handle) // maybe this helps to optimize out destructing of unalloced textures
			glDeleteTextures(1, &handle); // would be ok to delete unalloced texture (gpu_handle = 0)
	}

	// no implicit copy
	Texture2D& operator= (Texture2D& r) = delete;
	Texture2D (Texture2D& r) = delete;
	// move operators
	Texture2D& operator= (Texture2D&& r) {	swap(*this, r);	return *this; }
	Texture2D (Texture2D&& r) {				swap(*this, r); }

	//
	GLuint	get_handle () const {	return handle; }

	//
	static Texture2D generate () {
		Texture2D tex;
		glGenTextures(1, &tex.handle);
		return tex;
	}

	void upload_mipmap (int mip, pixel_format_e format, void const* pixels, iv2 size_px) {

		GLenum internal_format;
		GLenum cpu_format;
		GLenum type;

		switch (format) {
			case PF_LR8:	internal_format = GL_R8;			cpu_format = GL_RED;	type = GL_UNSIGNED_BYTE;	break;
			case PF_LRGBA8:	internal_format = GL_RGBA8;			cpu_format = GL_RGBA;	type = GL_UNSIGNED_BYTE;	break;
			case PF_SRGBA8:	internal_format = GL_SRGB8_ALPHA8;	cpu_format = GL_RGBA;	type = GL_UNSIGNED_BYTE;	break;

			case PF_LRF:	internal_format = GL_R32F;			cpu_format = GL_RED;	type = GL_FLOAT;			break;
			case PF_LRGBF:	internal_format = GL_RGB32F;		cpu_format = GL_RGB;	type = GL_FLOAT;			break;
			case PF_LRGBAF:	internal_format = GL_RGBA32F;		cpu_format = GL_RGBA;	type = GL_FLOAT;			break;
			default: assert(not_implemented);
		}

		glBindTexture(GL_TEXTURE_2D, handle);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexImage2D(GL_TEXTURE_2D, mip, internal_format, size_px.x,size_px.y, 0, cpu_format, type, pixels);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

private:
	void set_active_mips (int first, int last) { // i am not sure that just setting abitrary values for these actually works correctly
		glBindTexture(GL_TEXTURE_2D, handle);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, first);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, last);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
public:

	// single mip
	void upload (pixel_format_e format, void const* pixels, iv2 size_px) {
		upload_mipmap(0, format, pixels, size_px);

		set_active_mips(0, 0); // no mips
	}
	/* // doesnt work ??
	void gen_mipmaps () {
	glBindTexture(GL_TEXTURE_2D, gpu_handle);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	}
	*/

	void set_minmag_filtering (minmag_filter_e minmag_filter) {
		GLenum min, mag;

		switch (minmag_filter) {
			case FILTER_NEAREST:	min = mag = GL_NEAREST;								break;
			case FILTER_BILINEAR:	min = mag = GL_LINEAR;								break;
			case FILTER_TRILINEAR:	min = GL_LINEAR_MIPMAP_LINEAR;	mag = GL_LINEAR;	break;
			default: assert(not_implemented);
		}

		glBindTexture(GL_TEXTURE_2D, handle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void enable_filtering_anisotropic (aniso_filter_e aniso_filter) {
		if (GL_ARB_texture_filter_anisotropic) {
			GLfloat aniso;
			if (aniso_filter == FILTER_MAX_ANISO) {
				GLfloat max_aniso;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_aniso);
				aniso = max_aniso;
			} else {
				aniso = 1;
			}
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);
		}
	}

	void set_border (border_e border, rgba8 border_color=0) {
		GLenum mode;
		rgbaf border_colorf;

		switch (border) {
			case BORDER_CLAMP:	mode = GL_CLAMP_TO_EDGE; break;
			case BORDER_COLOR:	mode = GL_CLAMP_TO_BORDER;
				border_colorf = (rgbaf)border_color / 255;	break;
			default: assert(not_implemented);
		}

		glBindTexture(GL_TEXTURE_2D, handle);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);

		if (border == BORDER_COLOR)
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &border_colorf.x);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

};
void swap (Texture2D& l, Texture2D& r) {
	std::swap(l.handle, r.handle);
}

Texture2D stream_texture (void const* pixels, iv2 size_px, pixel_format_e format, mipmap_mode_e mips=GEN_MIPMAPS, minmag_filter_e minmag=FILTER_TRILINEAR, aniso_filter_e aniso=FILTER_MAX_ANISO, border_e border=BORDER_CLAMP, rgba8 border_color=0) {
	auto tex = Texture2D::generate();
	tex.upload(format, pixels, size_px);

	if (mips == GEN_MIPMAPS) {
		assert(not_implemented);
	}

	tex.set_minmag_filtering(minmag);
	tex.enable_filtering_anisotropic(aniso);
	tex.set_border(border, border_color);

	return tex;
}

Texture2D upload_texture_from_file (std::string const& filepath, pixel_format_e format, mipmap_mode_e mips=GEN_MIPMAPS, minmag_filter_e minmag=FILTER_TRILINEAR, aniso_filter_e aniso=FILTER_MAX_ANISO, border_e border=BORDER_CLAMP, rgba8 border_color=0) {
	Texture2D tex;

	iv2 size;
	int channels;
	int got_channels;

	switch (format) {
		case PF_LR8:	channels = 1;	break;
		case PF_LRGBA8:	channels = 4;	break;
		case PF_SRGBA8:	channels = 4;	break;
		
		case PF_LRF:	channels = 1;	break;
		case PF_LRGBF:	channels = 3;	break;
		case PF_LRGBAF:	channels = 4;	break;
		default: assert(not_implemented);
	}

	stbi_set_flip_vertically_on_load(true);

	auto* pixels = stbi_load(filepath.c_str(), &size.x,&size.y, &got_channels, channels);
	if (!pixels) {
		fprintf(stderr, "Texture \"%s\" could not be loaded!\n", filepath.c_str());
		return Texture2D(); // return error code ?
	}

	tex = stream_texture(pixels, size, format, mips, minmag, aniso, border, border_color);

	return tex;
}

struct Texture_Manager {
	std::unordered_map<std::string, Texture2D> textures;
	
	Texture2D* get_texture (std::string const& name, pixel_format_e format, mipmap_mode_e mips, minmag_filter_e minmag, aniso_filter_e aniso, border_e border, rgba8 border_color) {
		auto shad = textures.find(name);
		if (shad == textures.end())
			shad = textures.emplace(name, upload_texture_from_file(name, format, mips, minmag, aniso, border, border_color)).first;

		return &shad->second;
	}
};

Texture_Manager texture_manager;

Texture2D* get_texture (std::string const& name, pixel_format_e format, mipmap_mode_e mips=GEN_MIPMAPS, minmag_filter_e minmag=FILTER_TRILINEAR, aniso_filter_e aniso=FILTER_MAX_ANISO, border_e border=BORDER_CLAMP, rgba8 border_color=0) {
	return texture_manager.get_texture(name, format, mips, minmag, aniso, border, border_color);
}

//
}
using namespace texture_n;
}
