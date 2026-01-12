#pragma once

#include "../core/socket.hpp"
#include "../core/coroutine.hpp"
#include "connection.hpp"
#include <vector>

namespace cppcorn::http {

class Server {
public:
    Server(std::string ip, int port);
    
    // Main loop
    core::FireAndForget run();

private:
    std::string ip_;
    int port_;
    core::Socket listen_socket_;
};

} // namespace cppcorn::http
