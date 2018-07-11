#pragma once

#include <unordered_map>
#include <unordered_set>

#include "parse.hpp"

#include "directory_watcher.hpp"

namespace engine {
//
namespace shader {

	bool recursive_preprocess_shader (std::string const& filepath, std::string* source, std::unordered_set<std::string>* included_files) {
		included_files->insert(filepath);
		
		auto find_path = [] (std::string const& filepath) { // "shaders/blah.vert" -> "shaders/"  "shaders" -> ""  "blah\\" -> "\\"
			auto filename_pos = filepath.find_last_of("/\\");
			return filepath.substr(0, filename_pos +1);
		};
		
		std::string path = find_path(filepath);

		if (!load_text_file(filepath.c_str(), source)) {
			fprintf(stderr, "Could load shader source! \"%s\"\n", filepath.c_str());
			return false;
		}

		using namespace n_parse;

		int		line_number = 0; // in original file, this ignores all replaced lines

		int		cur_line_begin_indx = 0;
		char*	cur = (char*)&source->c_str()[cur_line_begin_indx];

		auto go_to_next_line = [&] () {
			while (!end_of_line(&cur))
				++cur;
		};

		auto replace_line_with = [&] (std::string lines) { // line should have been completed with end_of_line()
			auto cur_line_end_indx = cur -(char*)source->c_str();

			source->replace(	cur_line_begin_indx, cur -&source->c_str()[cur_line_begin_indx],
								lines);
			cur_line_end_indx += lines.size() -(cur_line_end_indx -cur_line_begin_indx);
			
			cur = (char*)&source->c_str()[cur_line_end_indx]; // source->replace invalidates cur
		};
		auto comment_out_cur_line = [&] () { // line should have been completed with end_of_line()
			auto cur_line_end_indx = cur -(char*)source->c_str();

			source->insert(cur_line_begin_indx, "//");
			cur_line_end_indx += 2;

			cur = (char*)&source->c_str()[cur_line_end_indx]; // source->replace invalidates cur
		};

		auto include_file = [&] (std::string include_filepath) -> bool { // line should have been completed with end_of_line()
			
			include_filepath.insert(0, path);

			if (included_files->find(include_filepath) != included_files->end()) {
				// file already include, we prevent double include by default
				replace_line_with("//include \"%s\" (prevented double-include)\n");
			} else {
				std::string included_source;
				if (!recursive_preprocess_shader(include_filepath, &included_source, included_files)) return false;

				replace_line_with(prints(	"//$include \"%s\"\n"
											"%s\n"
											"//$include_end file \"%s\" line %d\n",
											include_filepath.c_str(), included_source.c_str(), filepath.c_str(), line_number));
			}

			return true;
		};

		auto dollar_cmd = [&] (char** pcur) {
			char* cur = *pcur;

			whitespace(&cur);

			if (!character(&cur, '$')) return false;

			whitespace(&cur);

			*pcur = cur;
			return true;
		};

		auto include_cmd = [&] () {
			if (!identifier(&cur, "include")) return false;
			
			whitespace(&cur);
			
			std::string include_filepath;
			if (!quoted_string_copy(&cur, &include_filepath)) return false;
			
			if (!end_of_line(&cur)) return false;
			
			if (!include_file(std::move(include_filepath))) return false;
			
			return true;
		};

		while (!end_of_input(cur)) { // for all lines
			
			if ( dollar_cmd(&cur) ) {
				if (		include_cmd() );
				//else if (	other command );
				else {
					fprintf(stderr, "unknown or invalid $command in shader \"%s\".\n", filepath.c_str());
					
					// ignore invalid line
					go_to_next_line();
					comment_out_cur_line();
				}
			} else {
				go_to_next_line();
			}

			cur_line_begin_indx = (int)(cur -(char*)source->c_str()); // set beginning of next line
			++line_number;
		}

		return true;
	}
	bool preprocess_shader (std::string const& filepath, std::string* source) {
		std::unordered_set<std::string> included_files;
		return recursive_preprocess_shader(filepath, source, &included_files);
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
		if (!preprocess_shader(filepath, &source)) return false;

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
		if (shad == shaders.end()) {
			auto tmp = Shader::load(name);
			if (tmp.get_prog_handle() == 0)
				return nullptr;
			shad = shaders.emplace(name, std::move(tmp)).first;
		}
		return &shad->second;
	}

	Directory_Watcher shaders_changed = Directory_Watcher("shaders/");
	
	void poll_reload_shaders (int frame_i) {
		std::vector<std::string> changed_files;
		shaders_changed.poll_file_changes(&changed_files);

		for (auto& filepath : changed_files) {
			printf("frame %6d: %s changed\n", frame_i, filepath.c_str());
		}
	}
};

Shader_Manager shader_manager;

//
}
