#pragma once

namespace engine {
//

class FBO {
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

	static FBO create (Texture2D const& color_target, iv2 size_px) {
		FBO fbo;
		glGenFramebuffers(1, &fbo.handle);

		glBindFramebuffer(GL_FRAMEBUFFER, fbo.handle);

		//
		glGenRenderbuffers(1, &fbo.depth_renderbuffer);

		glBindRenderbuffer(GL_RENDERBUFFER, fbo.depth_renderbuffer);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size_px.x,size_px.y);
		
		//
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_target.get_handle(), 0);

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

FBO draw_to_texture (Texture2D const& color_target, iv2 size_px) {
	return FBO::create(color_target, size_px);
}

void draw_to_screen () {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//
}
