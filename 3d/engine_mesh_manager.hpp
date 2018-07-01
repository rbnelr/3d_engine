#pragma once

namespace engine {
//

class VBO {
	MOVE_ONLY_CLASS(VBO)
	
	GLuint handle = 0;

public:
	~VBO () {
		if (handle) // maybe this helps to optimize out destructing of unalloced vbos
			glDeleteBuffers(1, &handle); // would be ok to delete unalloced vbo (handle = 0)
	}

	static VBO generate () {
		VBO vbo;
		glGenBuffers(1, &vbo.handle);
		return vbo;
	}
	static VBO gen_and_upload (void const* vertex_data, int vertex_size, bool stream=false) {
		auto vbo = generate();
		glBindBuffer(GL_ARRAY_BUFFER, vbo.handle);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertex_size, vertex_data, stream ? GL_STREAM_DRAW : GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return vbo;
	}
	void reupload (void const* vertex_data, int vertex_size) {
		glBindBuffer(GL_ARRAY_BUFFER, handle);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertex_size, NULL, GL_DYNAMIC_DRAW); // Buffer orphan on reupload
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vertex_size, vertex_data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	GLuint get_handle () const {	return handle; }
};
inline void swap (VBO& l, VBO& r) {
	::std::swap(l.handle, r.handle);
}


struct Mesh_Manager {
	
	VBO stream_vertex_data (void const* vertex_data, int vertex_size) {
		return VBO::gen_and_upload(vertex_data, vertex_size);
	}
};

Mesh_Manager mesh_manager;

//
}
