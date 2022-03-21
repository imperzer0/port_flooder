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
				std::string command;
				bool substitution = false;
				while (::fgets(line.data(), line.size(), input))
				{
					actual_size = (int)::strlen(line.data());
					int prev_end = 0, i = 0, j;
					while (i < actual_size)
					{
						bool local_substitution = substitution;
						
						if (!substitution)
						{
							++i;
							for (; i < actual_size; ++i)
								if ((i < 2 || line[i - 2] != '\\') && line[i - 1] == '$' && line[i] == ' ')
								{
									local_substitution = true;
									break;
								}
						}
						else
						{
							--i;
						}
						
						
						for (j = i; j < actual_size; ++j)
							if (line[j - 1] == ' ' && line[j] == '$')
							{
								local_substitution = false;
								break;
							}
						
						if (!substitution)
						{
							std::string line_str = std::string(line.data()).substr(prev_end, i - (i != j) - prev_end);
							boost::replace_all(line_str, "\\$", "$");
							boost::replace_all(line_str, "\\\\", "\\");
							if (::fwrite(line_str.c_str(), sizeof(char), line_str.size(), output) != line_str.size())
							{
								::fprintf(stderr, WRITE_FAILED_FMT);
								return;
							}
						}
						
						if (j - i > 2)
							command += std::string(line.data() + i + 1, line.data() + j - (j < actual_size));
						
						if (j < actual_size && !local_substitution)
						{
							boost::replace_all(command, "\\$", "$");
							boost::replace_all(command, "\\\\", "\\");
							argv[PLACEHOLDER_INDEX] = command.data();
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
							bool lastendl = false;
							while (::fgets(stdoutline.data(), stdoutline.size(), stdout_stream))
							{
								size_t size = ::strlen(stdoutline.data());
								lastendl = stdoutline[size - 1] == '\n';
								if (::fwrite(stdoutline.data(), sizeof(char), size, output) != size)
								{
									::fprintf(stderr, WRITE_FAILED_FMT);
									return;
								}
							}
							if (lastendl)
								::fseek(output, -1, SEEK_CUR);
							::close(p[exec::pipe::read]);
							command.clear();
						}
						
						i = prev_end = j + 1;
						substitution = local_substitution;
					}
				}
				::fclose(input);
				::fclose(output);
			}
		}
		
		bool check()
		{
			if (input)
			{
				std::array<char, 128> line{ };
				int actual_size;
				bool substitution = false;
				while (::fgets(line.data(), line.size(), input))
				{
					actual_size = (int)::strlen(line.data());
					int i = 0, j;
					while (i < actual_size)
					{
						if (!substitution)
						{
							++i;
							for (; i < actual_size; ++i)
								if ((i < 2 || line[i - 2] != '\\') && line[i - 1] == '$' && line[i] == ' ')
								{
									substitution = true;
									break;
								}
						}
						else
						{
							--i;
						}
						
						for (j = i; j < actual_size; ++j)
							if (line[j - 1] == ' ' && line[j] == '$')
							{
								substitution = false;
								break;
							}
						
						i = j + 1;
					}
				}
				::fclose(input);
				return !substitution;
			}
			return false;
		}
	
	private:
		FILE* input;
		FILE* output;
	};
}

#endif //PORT_FLOODER_HMLEXECUTOR_HPP
