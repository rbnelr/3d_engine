#pragma once

namespace engine {
//

Shader const* _current_used_shader = nullptr; // for debugging

void gl_set_uniform (GLint loc, flt val) {		glUniform1f(	loc, val); }
void gl_set_uniform (GLint loc, fv2 val) {		glUniform2fv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, fv3 val) {		glUniform3fv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, fv4 val) {		glUniform4fv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, s32 val) {		glUniform1i(	loc, val); }
void gl_set_uniform (GLint loc, s32v2 val) {	glUniform2iv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, s32v3 val) {	glUniform3iv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, s32v4 val) {	glUniform4iv(	loc, 1, &val.x); }
void gl_set_uniform (GLint loc, fm2 val) {		glUniformMatrix2fv(	loc, 1, GL_FALSE, &val.arr[0].x); }
void gl_set_uniform (GLint loc, fm3 val) {		glUniformMatrix3fv(	loc, 1, GL_FALSE, &val.arr[0].x); }
void gl_set_uniform (GLint loc, fm4 val) {		glUniformMatrix4fv(	loc, 1, GL_FALSE, &val.arr[0].x); }
void gl_set_uniform (GLint loc, bool val) {		glUniform1i(	loc, (int)val); }
void gl_set_uniform (GLint loc, bv2 val) {		glUniform2i(	loc, (int)val.x,(int)val.y); }
void gl_set_uniform (GLint loc, bv3 val) {		glUniform3i(	loc, (int)val.x,(int)val.y,(int)val.z); }
void gl_set_uniform (GLint loc, bv4 val) {		glUniform4i(	loc, (int)val.x,(int)val.y,(int)val.z,(int)val.w); }


template <typename T> void set_uniform (Shader* shad, std::string const& name, T val) {
	assert(_current_used_shader == shad);

	GLint loc = glGetUniformLocation(shad->get_prog_handle(), name.c_str());
	if (loc >= 0) gl_set_uniform(loc, val);
}

void use_shader (Shader* shad) {
	_current_used_shader = shad;
	glUseProgram(shad->get_prog_handle());
}

struct Uniform_Sharer {
	enum type_e {
		FLT, FV2, FV3, FV4,
		INT_, IV2, IV3, IV4,

		U8V4,

		MAT2, MAT3, MAT4,

		BOOL, BV2, BV3, BV4,
	};
	struct Uniform_Val {
		type_e	type;
		union {
			f32		flt_;
			fv2		fv2_;
			fv3		fv3_;
			fv4		fv4_;

			s32		int_;
			s32v2	iv2_;
			s32v3	iv3_;
			s32v4	iv4_;

			fm2		fm2_;
			fm3		fm3_;
			fm4		fm4_;

			bool	bool_;
			bv2		bv2_;
			bv3		bv3_;
			bv4		bv4_;
		};

		Uniform_Val () {}

		void set (f32	val) { type = FLT ;	flt_  = val;	}
		void set (fv2	val) { type = FV2 ;	fv2_  = val;	}
		void set (fv3	val) { type = FV3 ;	fv3_  = val;	}
		void set (fv4	val) { type = FV4 ;	fv4_  = val;	}
		void set (s32	val) { type = INT_;	int_  = val;	}
		void set (s32v2	val) { type = IV2 ;	iv2_  = val;	}
		void set (s32v3	val) { type = IV3 ;	iv3_  = val;	}
		void set (s32v4	val) { type = IV4 ;	iv4_  = val;	}
		void set (m2	val) { type = MAT2;	fm2_  = val;	}
		void set (m3	val) { type = MAT3;	fm3_  = val;	}
		void set (m4	val) { type = MAT4;	fm4_  = val;	}
		void set (bool	val) { type = BOOL;	bool_ = val;	}
		void set (bv2	val) { type = BV2;	bv2_  = val;	}
		void set (bv3	val) { type = BV3;	bv3_  = val;	}
		void set (bv4	val) { type = BV4;	bv4_  = val;	}
	};
	static void gl_set_uniform (GLint loc, Uniform_Val const& val) {
		switch (val.type) {
			case FLT :	engine::gl_set_uniform(loc, val.flt_ );	break;
			case FV2 :	engine::gl_set_uniform(loc, val.fv2_ );	break;
			case FV3 :	engine::gl_set_uniform(loc, val.fv3_ );	break;
			case FV4 :	engine::gl_set_uniform(loc, val.fv4_ );	break;
			case INT_:	engine::gl_set_uniform(loc, val.int_ );	break;
			case IV2 :	engine::gl_set_uniform(loc, val.iv2_ );	break;
			case IV3 :	engine::gl_set_uniform(loc, val.iv3_ );	break;
			case IV4 :	engine::gl_set_uniform(loc, val.iv4_ );	break;
			case MAT2:	engine::gl_set_uniform(loc, val.fm2_ );	break;
			case MAT3:	engine::gl_set_uniform(loc, val.fm3_ );	break;
			case MAT4:	engine::gl_set_uniform(loc, val.fm4_ );	break;
			case BOOL:	engine::gl_set_uniform(loc, val.bool_);	break;
			case BV2:	engine::gl_set_uniform(loc, val.bv2_ );	break;
			case BV3:	engine::gl_set_uniform(loc, val.bv3_ );	break;
			case BV4:	engine::gl_set_uniform(loc, val.bv4_ );	break;
			default: assert(not_implemented);
		}
	}

	std::unordered_map<std::string, Uniform_Val>	shared_uniforms;

	void set_shared_uniform (std::string const& share_name, std::string const& uniform_name, Uniform_Val val) {
		shared_uniforms[share_name+ '_' +uniform_name] = val;
	}

	void set_shared_uniforms_for_shader (Shader* shad) {
		assert(_current_used_shader == shad);

		for (auto& su : shared_uniforms) {
			GLint loc = glGetUniformLocation(shad->get_prog_handle(), su.first.c_str());
			if (loc != -1) gl_set_uniform(loc, su.second);
		}
	}
};
Uniform_Sharer uniform_sharer;

/*
Set uniform that all shaders can access
This uniform even applies if the shader does not exist yet (i do lazy shader loading)

Could be optimized to use Uniform Buffer Object (share_name would be the name of a UBO and uniform_name would be a member of that structure)
*/
template <typename T> void set_shared_uniform (std::string const& share_name, std::string const& uniform_name, T val) {
	Uniform_Sharer::Uniform_Val unionized;
	unionized.set(val);
	uniform_sharer.set_shared_uniform(share_name, uniform_name, unionized);
}

// gl abstraction utility functions
Shader* _get_shader (std::string const& shader) {
	Shader*	shad = shader_manager.get_shader(shader);
	return shad;
}
Shader* use_shader (std::string const& shader) {
	Shader*	shad = _get_shader(shader);
	if (shad) {
		use_shader(shad);
		uniform_sharer.set_shared_uniforms_for_shader(shad);
	}
	return shad;
}

void bind_vertex_data (VBO const& vbo, Data_Vertex_Layout const& vertex_layout, Shader const& shad) {

	glBindBuffer(GL_ARRAY_BUFFER, vbo.get_handle());

	for (auto& attr : vertex_layout.attributes) {

		auto loc = glGetAttribLocation(shad.get_prog_handle(), attr.name.c_str());
		if (loc < 0)
			continue;

		glEnableVertexAttribArray(loc);

		switch (attr.type) {
			case FLT:			glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE,			vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case FV2:			glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE,			vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case FV3:			glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE,			vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case FV4:			glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE,			vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;

			case INT_:			glVertexAttribIPointer(loc, 1, GL_INT,						vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case IV2:			glVertexAttribIPointer(loc, 2, GL_INT,						vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case IV3:			glVertexAttribIPointer(loc, 3, GL_INT,						vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
			case IV4:			glVertexAttribIPointer(loc, 4, GL_INT,						vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;

			case U8V4_AS_FV4:	glVertexAttribPointer(loc, 4, GL_UNSIGNED_BYTE,	GL_TRUE,	vertex_layout.vertex_size, (void*)(uptr)attr.offset);	break;
		}

	}

}

void use_vertex_data (Shader const& shad, Data_Vertex_Layout const& vertex_layout, VBO const& vbo) {
	bind_vertex_data(vbo, vertex_layout, shad);
}
template <typename T> VBO use_stream_vertex_data (Shader const& shad, std::vector<T> const& vertex_data) {
	assert((int)sizeof(T) == T::layout.vertex_size);

	VBO vbo = stream_vertex_data(vertex_data.data(), (int)vertex_data.size() * T::layout.vertex_size);
	use_vertex_data(shad, T::layout, vbo);
	return vbo;
}

void draw_triangles (Shader const& shad, int first_vertex, int vertex_count) {
	glDrawArrays(GL_TRIANGLES, first_vertex, (GLsizei)vertex_count);
}

template <typename T> void draw_stream_triangles (Shader const& shad, std::vector<T> const& vertex_data, int first_vertex=0, int vertex_count=-1) {
	auto vbo = use_stream_vertex_data(shad, vertex_data);
	draw_triangles(shad, first_vertex, vertex_count < 0 ? (int)vertex_data.size() : vertex_count);
}

template <typename T> void draw_stream_triangles (std::string const& shader, std::vector<T> const& vertex_data, int first_vertex=0, int vertex_count=-1) {
	auto shad = use_shader(shader);
	if (shad)
		draw_stream_triangles(*shad, vertex_data, first_vertex, vertex_count);
}

void bind_texture (Shader* shad, std::string const& uniform_name, int tex_unit, Texture const& tex, GLenum target) {
	assert(_current_used_shader == shad);

	auto loc = glGetUniformLocation(shad->get_prog_handle(), uniform_name.c_str());
	if (loc >= 0) {
		glUniform1i(loc, tex_unit);

		glActiveTexture(GL_TEXTURE0 +tex_unit);
		glBindTexture(target, tex.get_handle());
	}
}
void bind_texture (Shader* shad, std::string const& uniform_name, int tex_unit, Texture2D const& tex) {
	bind_texture(shad, uniform_name, tex_unit, tex, GL_TEXTURE_2D);
}
void bind_texture (Shader* shad, std::string const& uniform_name, int tex_unit, TextureCube const& tex) {
	bind_texture(shad, uniform_name, tex_unit, tex, GL_TEXTURE_CUBE_MAP);
}

constexpr v2 quad_verts[] = { v2(1,0),v2(1,1),v2(0,0), v2(0,0),v2(1,1),v2(0,1) };

//
}
