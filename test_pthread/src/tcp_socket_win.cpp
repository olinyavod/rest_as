#include "tcp_socket.h"

namespace net {

void tcp_socket::initialize_winsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
}

tcp_socket::tcp_socket()
    : port_(0), host_(""), socket_fd_(INVALID_SOCKET), is_listening_(false), is_server_socket_(false), is_connected_(false) {
    memset(&address_, 0, sizeof(address_));
    initialize_winsock();
}

tcp_socket::tcp_socket(int port, const std::string& host)
    : port_(port), host_(host), socket_fd_(INVALID_SOCKET), is_listening_(false), is_server_socket_(true), is_connected_(false) {
    memset(&address_, 0, sizeof(address_));
    initialize_winsock();
}

tcp_socket::~tcp_socket() {
    if (is_listening_) {
        stop_listening();
    }
    if (socket_fd_ != INVALID_SOCKET) {
        closesocket(socket_fd_);
        WSACleanup();
    }
}

std::expected<void, std::string> tcp_socket::start_listening() {
    if (is_connected_) {
        return std::unexpected("Cannot listen on a connected socket");
    }

    if (!is_server_socket_) {
        return std::unexpected("Cannot listen on a client socket");
    }

    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ == INVALID_SOCKET) {
        return std::unexpected("Socket creation failed");
    }

    address_.sin_family = AF_INET;
    address_.sin_port = htons(port_);

    if (host_.empty()) {
        address_.sin_addr.s_addr = INADDR_ANY;
    } else {
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int status = getaddrinfo(host_.c_str(), nullptr, &hints, &res);
        if (status != 0) {
            closesocket(socket_fd_);
            WSACleanup();
            return std::unexpected("getaddrinfo failed: " + std::string(gai_strerror(status)));
        }

        struct sockaddr_in* addr_in = (struct sockaddr_in*)res->ai_addr;
        address_.sin_addr = addr_in->sin_addr;

        freeaddrinfo(res);
    }

    if (bind(socket_fd_, (struct sockaddr*)&address_, sizeof(address_)) == SOCKET_ERROR) {
        closesocket(socket_fd_);
        WSACleanup();
        return std::unexpected("Bind failed");
    }

    if (listen(socket_fd_, 3) == SOCKET_ERROR) {
        closesocket(socket_fd_);
        WSACleanup();
        return std::unexpected("Listen failed");
    }

    is_listening_ = true;
    std::cout << "Listening on port " << port_ << (host_.empty() ? "" : " on host " + host_) << std::endl;
    return {};
}

std::expected<std::unique_ptr<tcp_socket>, std::string> tcp_socket::accept_connection() {
    if (!is_listening_) {
        return std::unexpected("Socket is not listening");
    }

    int addrlen = sizeof(address_);
    SOCKET new_socket = accept(socket_fd_, (struct sockaddr*)&address_, &addrlen);
    if (new_socket == INVALID_SOCKET) {
        return std::unexpected("Accept failed");
    }

    return std::unique_ptr<tcp_socket>(new tcp_socket(new_socket, address_, false));
}

std::expected<void, std::string> tcp_socket::connect(const std::string& host, int port) {
    if (is_listening_) {
        return std::unexpected("Cannot connect on a listening socket");
    }

    if (is_connected_) {
        return std::unexpected("Socket is already connected");
    }

    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ == INVALID_SOCKET) {
        return std::unexpected("Socket creation failed");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0) {
        return std::unexpected("Invalid address/ Address not supported");
    }

    if (::connect(socket_fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        return std::unexpected("Connection failed");
    }

    is_connected_ = true;
    is_server_socket_ = false;
    return {};
}

void tcp_socket::disconnect() {
    if (is_connected_) {
        closesocket(socket_fd_);
        socket_fd_ = INVALID_SOCKET;
        is_connected_ = false;
        std::cout << "Disconnected from remote socket" << std::endl;
        WSACleanup();
    }
}

void tcp_socket::stop_listening() {
    if (is_listening_) {
        closesocket(socket_fd_);
        socket_fd_ = INVALID_SOCKET;
        is_listening_ = false;
        std::cout << "Stopped listening on port " << port_ << std::endl;
        WSACleanup();
    }
}

std::string tcp_socket::get_address() const {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address_.sin_addr), ip, INET_ADDRSTRLEN);
    return std::string(ip);
}

std::expected<void, std::string> tcp_socket::send(std::istream& data) {
    char buffer[1024];
    while (data.read(buffer, sizeof(buffer)) || data.gcount() > 0) {
        int bytes_to_send = static_cast<int>(data.gcount());
        int bytes_sent = ::send(socket_fd_, buffer, bytes_to_send, 0);
        if (bytes_sent == SOCKET_ERROR) {
            return std::unexpected("Send failed");
        }
    }
    return {};
}

std::expected<void, std::string> tcp_socket::receive(std::ostream& output) {
    char buffer[1024];
    int bytes_received = 0;

    while ((bytes_received = ::recv(socket_fd_, buffer, sizeof(buffer), 0)) > 0) {
        output.write(buffer, bytes_received);
        if (bytes_received < sizeof(buffer)) {
            break; // No more data to read
        }
    }

    if (bytes_received == SOCKET_ERROR) {
        return std::unexpected("Receive failed");
    }

    return {};
}

tcp_socket::tcp_socket(SOCKET socket_fd, const struct sockaddr_in& address, bool is_server_socket)
    : port_(0), host_(""), socket_fd_(socket_fd), address_(address), is_listening_(false), is_server_socket_(is_server_socket), is_connected_(false) {}

} // namespace net
