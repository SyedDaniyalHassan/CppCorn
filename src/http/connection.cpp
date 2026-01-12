#include "connection.hpp"
#include "../asgi/bridge.hpp"
#include <fmt/core.h>

namespace cppcorn::http {

// Demo: Global bridge instance (ugly but functional for prototype)
extern asgi::Bridge* g_bridge;

Connection::Connection(core::Socket socket) 
    : socket_(std::move(socket)) {
    read_buffer_.resize(8192);
}

Connection::~Connection() {}

core::FireAndForget Connection::start() {
    socket_.set_non_blocking();
    try {
        while (true) {
            size_t n = co_await socket_.read(std::span(read_buffer_));
            if (n == 0) break;

            parser_.feed(std::string_view(read_buffer_.data(), n));

            if (parser_.is_complete()) {
                const auto& req = parser_.request();
                fmt::print("Request: {} {}\n", req.method, req.path);

                if (g_bridge) {
                    // Forward to ASGI
                    nlohmann::json scope;
                    scope["method"] = req.method;
                    scope["path"] = req.path;
                    scope["headers"] = nlohmann::json::array();
                    for(auto& h : req.headers) {
                        scope["headers"].push_back({h.first, h.second});
                    }

                    co_await g_bridge->send_request(scope);
                    auto resp = co_await g_bridge->receive_response();
                    
                    // Parse response
                    std::string body = resp.value("body", "");
                    int status = resp.value("status", 200);
                    
                    co_await send_response(body, status);
                } else {
                    co_await send_response("No Worker Attached");
                }
                parser_.reset();
            }
        }
    } catch (const std::exception& e) {
        fmt::print("Connection Error: {}\n", e.what());
    }
    delete this;
}

core::Task<void> Connection::send_response(const std::string& body, int status) {
    std::string response = fmt::format(
        "HTTP/1.1 {} OK\r\n"
        "Content-Length: {}\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "{}",
        status, body.size(), body
    );

    co_await socket_.write(std::span(response.data(), response.size()));
}

} // namespace cppcorn::http
