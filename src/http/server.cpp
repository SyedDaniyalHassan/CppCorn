#include "server.hpp"
#include <fmt/core.h>

namespace cppcorn::http {

Server::Server(std::string ip, int port) 
    : ip_(std::move(ip)), port_(port) {}

core::FireAndForget Server::run() {
    listen_socket_.bind(ip_.c_str(), port_);
    listen_socket_.listen();
    listen_socket_.set_non_blocking();

    fmt::print("CppCorn listening on {}:{}\n", ip_, port_);
    
    while (true) {
        auto client = co_await listen_socket_.accept_async();
        if (client.fd() != INVALID_SOCKET_VAL) {
            auto conn = new Connection(std::move(client));
            conn->start(); // Fire and forget (self-deleting)
        }
    }
}

} // namespace cppcorn::http
