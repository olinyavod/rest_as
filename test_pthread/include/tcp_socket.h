#ifndef TCP_SOCKET_HPP
#define TCP_SOCKET_HPP

#include <cstring> // For memset
#include <expected>
#include <iostream>
#include <memory>
#include <string>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace net
{

	class tcp_socket
	{
	  public:
		tcp_socket();
		tcp_socket(int port, const std::string &host);
		~tcp_socket();

		std::expected<void, std::string> start_listening();
		std::expected<std::unique_ptr<tcp_socket>, std::string> accept_connection();
		std::expected<void, std::string> connect(const std::string &host, int port);
		void disconnect();
		void stop_listening();
		std::string get_address() const;
		std::expected<void, std::string> send(std::istream &data);
		std::expected<void, std::string> receive(std::ostream &output);

	  private:
		int port_;
		std::string host_;

#ifdef _WIN32
		SOCKET socket_fd_;
		struct sockaddr_in address_;
		void initialize_winsock();
#else
		int socket_fd_;
		struct sockaddr_in address_;
#endif

		bool is_listening_;
		bool is_server_socket_;
		bool is_connected_;

#ifdef _WIN32
		tcp_socket(SOCKET socket_fd, const struct sockaddr_in& address, bool is_server_socket);
#else // _WIN32
		tcp_socket(int socket_fd, const struct sockaddr_in &address, bool is_server_socket);
#endif
	};

} // namespace net

#endif // TCP_SOCKET_HPP
