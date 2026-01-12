#pragma once

#include "../core/coroutine.hpp"
#include "../core/socket.hpp"
#include <nlohmann/json.hpp>

namespace cppcorn::asgi {

class Bridge {
public:
    Bridge();
    ~Bridge();

    void listen();
    void accept_worker(); // Call this to wait for a connection
    
    void spawn_worker();

    core::Task<void> send_request(const nlohmann::json& scope);
    core::Task<nlohmann::json> receive_response();

private:
    core::Socket ipc_socket_;     // Listening socket
    core::Socket worker_socket_;  // Active connection
};

} // namespace cppcorn::asgi
