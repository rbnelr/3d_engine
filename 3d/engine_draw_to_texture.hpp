#pragma once

namespace engine {
//

class FBO {
	friend void draw_to_texture (FBO const& fbo, iv2 texture_size);

	MOVE_ONLY_CLASS(FBO)

	GLuint	handle = 0;
	GLuint	depth_renderbuffer = 0;

public:
	~FBO () {
		if (handle) // maybe this helps to optimize out destructing of unalloced textures
			glDeleteFramebuffers(1, &handle); // would be ok to delete unalloced texture (gpu_handle = 0)
		if (depth_renderbuffer)
			glDeleteRenderbuffers(1, &depth_renderbuffer);
	}

	static FBO create (Texture const& color_target, iv2 size_px, int mip=0, int cubemap_face=-1) {
		FBO fbo;
		glGenFramebuffers(1, &fbo.handle);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo.handle);

		//
		glGenRenderbuffers(1, &fbo.depth_renderbuffer);

		glBindRenderbuffer(GL_RENDERBUFFER, fbo.depth_renderbuffer);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size_px.x,size_px.y);
		
		//
		if (cubemap_face == -1)
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_target.get_handle(), mip);
		else
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X +cubemap_face, color_target.get_handle(), mip);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo.depth_renderbuffer);

		/*
		GLenum bufs = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &bufs);
		*/

		//assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glCheckFramebufferStatus(GL_FRAMEBUFFER);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return fbo;
	}

	void bind () {
		glBindFramebuffer(GL_FRAMEBUFFER, handle);
	}
};
void swap (FBO& l, FBO& r) {
	std::swap(l.handle, r.handle);
	std::swap(l.depth_renderbuffer, r.depth_renderbuffer);
}

class FBO_Cube {
	FBO	faces[6];
public:
	static FBO_Cube create (TextureCube const& color_target, iv2 size_px, int mip=0) {
		FBO_Cube fbo;

		for (int face=0; face<6; ++face) {
			fbo.faces[face] = FBO::create(color_target, size_px, mip, face);
		}

		return fbo;
	}

	void draw_to_face (int face, iv2 cubemap_size) {
		glViewport(0,0, cubemap_size.x,cubemap_size.y);
		glScissor(0,0, cubemap_size.x,cubemap_size.y);

		set_shared_uniform("common", "viewport_size", (v2)cubemap_size);

		faces[face].bind();
	}
};

FBO create_fbo (Texture2D const& color_target, iv2 size_px) {
	return FBO::create(color_target, size_px);
}
FBO_Cube create_fbo (TextureCube const& color_target, iv2 size_px, int mip=0) {
	return FBO_Cube::create(color_target, size_px, mip);
}

void draw_to_screen (iv2 screen_size) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0,0, screen_size.x,screen_size.y);
	glScissor(0,0, screen_size.x,screen_size.y);

	set_shared_uniform("common", "viewport_size", (v2)screen_size);
}
void draw_to_texture (FBO const& fbo, iv2 texture_size) {

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.handle);

	glViewport(0,0, texture_size.x,texture_size.y);
	glScissor(0,0, texture_size.x,texture_size.y);

	set_shared_uniform("common", "viewport_size", (v2)texture_size);
}

//
void draw_entire_cubemap (Shader* shad, FBO_Cube* fbo, iv2 cubemap_res) {
	
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);

	struct Vertex {
		v3		pos;
	};
	const Data_Vertex_Layout layout = { (int)sizeof(Vertex), {
		{ "pos",		FV3,	(int)offsetof(Vertex, pos) },
	}};

	static std::vector<Vertex> cube;
	if (cube.size() == 0)
		generate_cube(1, [] (v3 p) { cube.push_back({p}); });

	static auto vbo = stream_vertex_data(cube.data(), (int)cube.size() * layout.vertex_size);

	bind_vertex_data(vbo, layout, *shad);

	static m3 faces_to_cam[6] = {
		rotate3_X(deg(-90)) *	rotate3_Z(deg(+90)),	// pos x , our right
		rotate3_X(deg(-90)) *	rotate3_Z(deg(-90)),	// neg x , our left
		m3::ident(),									// pos y , our down
		rotate3_X(deg(180)),							// neg y , our up
		rotate3_X(deg(-90)),							// pos z , our front
		rotate3_X(deg(-90)) *	rotate3_Z(deg(180)),	// neg z , our back
	};

	m4 cam_to_clip = calc_perspective_matrix(deg(90), 1.0f/16, 1024, 1);
	set_uniform(shad, "cam_to_clip", cam_to_clip);

	for (auto face=0; face<6; ++face) {
		fbo->draw_to_face(face, cubemap_res);

		set_uniform(shad, "to_cam", faces_to_cam[face]);

		draw_triangles(*shad, 0, (int)cube.size());
	}
}

//
}
