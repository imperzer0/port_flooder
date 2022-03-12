//
// Created by imper on 3/12/22.
//

#ifndef PORT_FLOODER_HMLEXECUTOR_HPP
#define PORT_FLOODER_HMLEXECUTOR_HPP

#include "color.hpp"
#include <execute-process-linux>
#include <execute-process-linux-defs>
#include <boost/algorithm/string.hpp>

#define PLACEHOLDER nullptr
#define PLACEHOLDER_INDEX 2
#define CMD_LINE(shell_var) {::strdup(shell_var), "-c", PLACEHOLDER, nullptr}

#define WRITE_FAILED_FMT COLOR_RED "Failed write to (FILE*) output stream!!!\n" COLOR_RESET

namespace hml
{
	class compiler
	{
	public:
		inline compiler(FILE* input, FILE* output) : input(input), output(output)
		{ }
		
		/// Compiles input file to output file
		/// !!! No debil-expression protection
		inline void compile(const char* shell = "bash")
		{
			if (input && output)
			{
				std::array<char, 128> line{ };
				int actual_size;
				char* argv[]CMD_LINE(shell);
				while (::fgets(line.data(), line.size(), input))
				{
					actual_size = (int)::strlen(line.data());
					int prev_end = 0, i = 0, j;
					while (i < actual_size)
					{
						++i;
						for (; i < actual_size && !((i < 2 || line[i - 2] != '\\') && line[i - 1] == '$' && line[i] == ' '); ++i);
						for (j = i; j < actual_size && !(line[j - 1] == ' ' && line[j] == '$'); ++j);
						
						std::string line_str = std::string(line.data()).substr(prev_end, i - (i != j) - prev_end);
						boost::replace_all(line_str, "\\$", "$");
						boost::replace_all(line_str, "\\\\", "\\");
						
						if (::fwrite(line_str.c_str(), sizeof(char), line_str.size(), output) != line_str.size())
						{
							::fprintf(stderr, WRITE_FAILED_FMT);
							return;
						}
						if (j - i > 2)
						{
							auto tmp = std::string(j - i - 1, 0);
							::strncpy(tmp.data(), line.data() + i + 1, j - i - 2);
							boost::replace_all(tmp, "\\&", "&");
							boost::replace_all(tmp, "\\\\", "\\");
							argv[PLACEHOLDER_INDEX] = tmp.data();
							int p[2];
							::pipe(p);
							auto pid = exec::execute_program(argv, nullptr, exec::std_out.redirect_to(p[exec::pipe::write]));
							if (exec::wait_for_program(pid))
							{
								::close(p[exec::pipe::write]);
								continue;
							}
							::close(p[exec::pipe::write]);
							
							FILE* stdout_stream = ::fdopen(p[exec::pipe::read], "rb");
							std::array<char, 128> stdoutline{ };
							while (::fgets(stdoutline.data(), stdoutline.size(), stdout_stream))
							{
								size_t size = ::strlen(stdoutline.data());
								if (stdoutline[size - 1] == '\n')
									--size;
								if (::fwrite(stdoutline.data(), sizeof(char), size, output) != size)
								{
									::fprintf(stderr, WRITE_FAILED_FMT);
									return;
								}
							}
							::close(p[exec::pipe::read]);
						}
						i = prev_end = j + 1;
					}
				}
				::fclose(input);
				::fclose(output);
			}
		}
	
	private:
		FILE* input;
		FILE* output;
	};
}

#endif //PORT_FLOODER_HMLEXECUTOR_HPP
