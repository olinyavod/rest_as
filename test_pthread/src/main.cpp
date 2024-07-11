#include "tcp_socket.h"
#include <iostream>
#include <sstream>

int main()
{
	net::tcp_socket server(8080, "");

	auto listen_result = server.start_listening();
	if (!listen_result)
	{
		std::cerr << "Failed to start listening: " << listen_result.error() << std::endl;
		return 1;
	}

	auto client_socket_result = server.accept_connection();
	if (!client_socket_result)
	{
		std::cerr << "Failed to accept connection: " << client_socket_result.error() << std::endl;
		return 1;
	}

	auto client_socket = std::move(client_socket_result.value());

	std::cout << "Connection accepted from " << client_socket->get_address() << std::endl;

	// Send and receive data
	std::stringstream send_stream;
	send_stream << "Hello, client!";
	auto send_result = client_socket->send(send_stream);
	if (!send_result)
	{
		std::cerr << "Failed to send data: " << send_result.error() << std::endl;
		return 1;
	}

	std::stringstream receive_stream;
	auto receive_result = client_socket->receive(receive_stream);
	if (!receive_result)
	{
		std::cerr << "Failed to receive data: " << receive_result.error() << std::endl;
		return 1;
	}
	else
	{
		std::cout << "Received from client: " << receive_stream.str() << std::endl;
	}

	server.stop_listening();

	return 0;
}
