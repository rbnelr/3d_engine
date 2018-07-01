#pragma once

#include <unordered_map>

namespace engine {
//
namespace shader {
	bool preprocess_shader (std::string const& filepath, std::string* source) {
		return load_text_file(filepath.c_str(), source);
	}
	
	bool get_shader_compile_log (GLuint shad, std::string* log) {
		GLsizei log_len;
		{
			GLint temp = 0;
			glGetShaderiv(shad, GL_INFO_LOG_LENGTH, &temp);
			log_len = (GLsizei)temp;
		}

		if (log_len <= 1) {
			return false; // no log available
		} else {
			// GL_INFO_LOG_LENGTH includes the null terminator, but it is not allowed to write the null terminator in str, so we have to allocate one additional char and then resize it away at the end

			log->resize(log_len);

			GLsizei written_len = 0;
			glGetShaderInfoLog(shad, log_len, &written_len, &(*log)[0]);
			assert(written_len == (log_len -1));

			log->resize(written_len);

			return true;
		}
	}
	bool get_program_link_log (GLuint prog, std::string* log) {
		GLsizei log_len;
		{
			GLint temp = 0;
			glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &temp);
			log_len = (GLsizei)temp;
		}

		if (log_len <= 1) {
			return false; // no log available
		} else {
			// GL_INFO_LOG_LENGTH includes the null terminator, but it is not allowed to write the null terminator in str, so we have to allocate one additional char and then resize it away at the end

			log->resize(log_len);

			GLsizei written_len = 0;
			glGetProgramInfoLog(prog, log_len, &written_len, &(*log)[0]);
			assert(written_len == (log_len -1));

			log->resize(written_len);

			return true;
		}
	}

	bool load_shader (GLenum type, std::string const& filepath, GLuint* shad, std::string* preprocessed) {
		*shad = glCreateShader(type);

		std::string source;
		if (!preprocess_shader(filepath, &source)) {
			fprintf(stderr, "Could produce load shader source! \"%s\"\n", filepath.c_str());
			return false;
		}

		{
			cstr ptr = source.c_str();
			glShaderSource(*shad, 1, &ptr, NULL);
		}

		glCompileShader(*shad);

		bool success;
		{
			GLint status;
			glGetShaderiv(*shad, GL_COMPILE_STATUS, &status);

			std::string log_str;
			bool log_avail = get_shader_compile_log(*shad, &log_str);

			success = status == GL_TRUE;
			if (!success) {
				// compilation failed
				fprintf(stderr, "OpenGL error in shader compilation \"%s\"!\n>>>\n%s\n<<<\n", filepath.c_str(), log_avail ? log_str.c_str() : "<no log available>");
			} else {
				// compilation success
				if (log_avail) {
					fprintf(stderr, "OpenGL shader compilation log \"%s\":\n>>>\n%s\n<<<\n", filepath.c_str(), log_str.c_str());
				}
			}
		}

		return success;
	}
	GLuint load_program (std::string const& vert_filepath, std::string const& frag_filepath) {
		GLuint prog_handle = glCreateProgram();

		GLuint vert;
		GLuint frag;

		bool compile_success = true;

		std::string vert_pp_src, frag_pp_src;

		bool vert_success = load_shader(GL_VERTEX_SHADER,		vert_filepath, &vert, &vert_pp_src);
		bool frag_success = load_shader(GL_FRAGMENT_SHADER,		frag_filepath, &frag, &frag_pp_src);

		if (!(vert_success && frag_success)) {
			glDeleteProgram(prog_handle);
			prog_handle = 0;
			return 0;
		}

		glAttachShader(prog_handle, vert);
		glAttachShader(prog_handle, frag);

		glLinkProgram(prog_handle);

		bool success;
		{
			GLint status;
			glGetProgramiv(prog_handle, GL_LINK_STATUS, &status);

			std::string log_str;
			bool log_avail = get_program_link_log(prog_handle, &log_str);

			success = status == GL_TRUE;
			if (!success) {
				// linking failed
				fprintf(stderr, "OpenGL error in shader linkage \"%s\"|\"%s\"!\n>>>\n%s\n<<<\n", vert_filepath.c_str(), frag_filepath.c_str(), log_avail ? log_str.c_str() : "<no log available>");
			} else {
				// linking success
				if (log_avail) {
					fprintf(stderr, "OpenGL shader linkage log \"%s\"|\"%s\":\n>>>\n%s\n<<<\n", vert_filepath.c_str(), frag_filepath.c_str(), log_str.c_str());
				}
			}
		}

		glDetachShader(prog_handle, vert);
		glDetachShader(prog_handle, frag);

		glDeleteShader(vert);
		glDeleteShader(frag);

		return prog_handle;
	}
}
using namespace shader;

class Shader {
	MOVE_ONLY_CLASS(Shader)

	GLuint prog_handle = 0;

public:
	~Shader () {
		if (prog_handle) // maybe this helps to optimize out destructing of unalloced shaders
			glDeleteProgram(prog_handle); // would be ok to delete unalloced shaders (handle = 0)
	}

	static Shader load (std::string const& name) {
		Shader shad;
		shad.prog_handle = load_program(name +".vert", name +".frag");
		return shad;
	}

	GLuint	get_prog_handle () const {	return prog_handle; }
};
inline void swap (Shader& l, Shader& r) {
	::std::swap(l.prog_handle, r.prog_handle);
}

struct Shader_Manager {
	
	std::unordered_map<std::string, Shader> shaders;

	Shader* get_shader (std::string const& name) {
		auto shad = shaders.find(name);
		if (shad == shaders.end())
			shad = shaders.emplace(name, Shader::load(name) ).first;
		
		return &shad->second;
	}
};

Shader_Manager shader_manager;

//
}
