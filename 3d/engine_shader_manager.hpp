#pragma once

#include "parse.hpp"

#include "directory_watcher.hpp"

template <typename T> bool contains (std::vector<T> const& c, T const& val) {
	return std::find(c.begin(), c.end(), val) != c.end();
}
template <typename T> bool any_contains (std::vector<T> const& as, std::vector<T> const& bs) {
	for (auto& a : as) {
		if (contains(bs, a))
			return true;
	}
	return false;
}

namespace engine {
//

class Shader {
	MOVE_ONLY_CLASS(Shader)

	GLuint prog_handle = 0;

public:
	~Shader () {
		if (prog_handle) // maybe this helps to optimize out destructing of unalloced shaders
			glDeleteProgram(prog_handle); // would be ok to delete unalloced shaders (handle = 0)
	}
	static Shader take_handle (GLuint h) {
		Shader shad;
		shad.prog_handle = h;
		return shad;
	}

	GLuint	get_prog_handle () const {	return prog_handle; }
};
inline void swap (Shader& l, Shader& r) {
	::std::swap(l.prog_handle, r.prog_handle);
}

#define INLINE_SHADER_RELOADING 1

struct Shader_Manager {
	
	bool recursive_preprocess_shader (std::string const& filepath, std::string* source, std::vector<std::string>* included_files, std::vector<std::string>* file_dependencies=nullptr) {
		included_files->push_back(filepath);

		if (file_dependencies && !contains(*file_dependencies, filepath))
			file_dependencies->push_back(filepath);

		auto find_path = [] (std::string const& filepath) { // "shaders/blah.vert" -> "shaders/"  "shaders" -> ""  "blah\\" -> "\\"
			auto filename_pos = filepath.find_last_of("/\\");
			return filepath.substr(0, filename_pos +1);
		};

		std::string path = find_path(filepath);

		if (!(get_inline_shader_source(filepath, source) || load_text_file(filepath.c_str(), source))) {
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

			if (contains(*included_files, include_filepath)) {
				// file already include, we prevent double include by default
				replace_line_with("//include \"%s\" (prevented double-include)\n");
			} else {
				std::string included_source;
				if (!recursive_preprocess_shader(include_filepath, &included_source, included_files, file_dependencies)) return false;

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
	bool preprocess_shader (std::string const& filepath, std::string* source, std::vector<std::string>* file_dependencies=nullptr) {
		std::vector<std::string> included_files; // included_files in this shader (file_dependencies are for entire shader program, so i can't use the same list of files here)
		return recursive_preprocess_shader(filepath, source, &included_files, file_dependencies);
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

	bool load_gl_shader (GLenum type, std::string const& filepath, GLuint* shad, std::string* preprocessed, std::vector<std::string>* file_dependencies=nullptr) {
		*shad = glCreateShader(type);

		std::string source;
		if (!preprocess_shader(filepath, &source, file_dependencies)) return false;

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
	GLuint load_gl_shader_program (std::string const& vert_filepath, std::string const& frag_filepath, std::vector<std::string>* file_dependencies=nullptr) {
		GLuint prog_handle = glCreateProgram();

		GLuint vert;
		GLuint frag;

		bool compile_success = true;

		std::string vert_pp_src, frag_pp_src;

		bool vert_success = load_gl_shader(GL_VERTEX_SHADER,		vert_filepath, &vert, &vert_pp_src, file_dependencies);
		bool frag_success = load_gl_shader(GL_FRAGMENT_SHADER,		frag_filepath, &frag, &frag_pp_src, file_dependencies);

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

	Shader load_shader (std::string const& name, std::vector<std::string>* file_dependencies=nullptr) {
		auto h = load_gl_shader_program(	prints("shaders/%s.vert", name.c_str()),
											prints("shaders/%s.frag", name.c_str()), file_dependencies );
		return Shader::take_handle(h);
	}

	//
	struct Cached_Shader {
		Shader	shad;

		std::vector<std::string> file_dependencies; // if any file this shader is made out of is changed reload the shader
	};

	std::unordered_map<std::string, Cached_Shader> shaders;

	Shader* get_shader (std::string const& name) {
		auto shad = shaders.find(name);
		if (shad == shaders.end()) {
			Cached_Shader s;

			s.shad = load_shader(name, &s.file_dependencies);
			if (s.shad.get_prog_handle() == 0)
				return nullptr;

			shad = shaders.emplace(name, std::move(s)).first;
		}
		return &shad->second.shad;
	}

	struct Inline_Shader_File {
		std::string	source;

		#if INLINE_SHADER_RELOADING
		bool		was_changed = false;
		#endif

		Inline_Shader_File (std::string const& source): source{source} {}
	};

	std::unordered_map<std::string, Inline_Shader_File> inline_shader_files;

	bool get_inline_shader_source (std::string const& virtual_filepath, std::string* source) {
		auto is = inline_shader_files.find(virtual_filepath);
		if (is == inline_shader_files.end())
			return false;

		*source = is->second.source;
		return true;
	}
	void inline_shader (std::string virtual_filepath, std::string const& source) {
		virtual_filepath.insert(0, "shaders/");

		auto is = inline_shader_files.find(virtual_filepath);
		if (is == inline_shader_files.end()) {
			
			is = inline_shader_files.emplace(virtual_filepath, source).first;

		} else {
			
			#if INLINE_SHADER_RELOADING
			is->second.was_changed = is->second.source.compare(source) != 0;
			if (is->second.was_changed)
				is->second.source = source;
			#endif
		}
	}

	Directory_Watcher dir_watcher = Directory_Watcher("shaders/");
	
	void poll_reload_shaders (int frame_i) {
		std::vector<std::string> changed_files;
		
		dir_watcher.poll_file_changes(&changed_files);

		for (auto& is : inline_shader_files) {
			if (is.second.was_changed) {
				changed_files.push_back( is.first );
				is.second.was_changed = false;
			}
		}

		for (auto& filepath : changed_files) {
			printf("frame %6d: \"%s\" changed\n", frame_i, filepath.c_str());
		}

		for (auto& shad : shaders) {
			auto& filepath = shad.first;

			if (any_contains(shad.second.file_dependencies, changed_files)) {
				// dependency changed

				Cached_Shader s;

				s.shad = load_shader(filepath, &s.file_dependencies);

				if (s.shad.get_prog_handle() == 0) {
					// new shader could not be loaded, keep the old shader
					printf("  shader \"%s\" could not be loaded, keeping the old shader!\n", filepath.c_str());
				} else {
					printf("  reloading shader \"%s\".\n", filepath.c_str());
					shad.second = std::move(s); // overwrite old shader with new
				}
			}

		}
	}
};

Shader_Manager shader_manager;

void inline_shader (std::string const& virtual_filepath, std::string const& source) {
	shader_manager.inline_shader(virtual_filepath, source);
}

//
}
