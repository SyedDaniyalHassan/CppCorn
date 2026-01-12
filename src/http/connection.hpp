#pragma once

#include "../core/socket.hpp"
#include "../core/coroutine.hpp"
#include "parser.hpp"
#include <span>
#include <vector>

namespace cppcorn::http {

class Connection {
public:
    explicit Connection(core::Socket socket);
    ~Connection();

    // The main coroutine for handling this client
    core::FireAndForget start();

private:
    core::Task<void> send_response(const std::string& body, int status = 200);
    
    core::Socket socket_;
    Parser parser_;
    std::vector<char> read_buffer_;
};

} // namespace cppcorn::http
