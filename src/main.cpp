#include <iostream>
#include <fmt/core.h>
#include "core/event_loop.hpp"
#include "http/server.hpp"
#include "asgi/bridge.hpp"

using namespace cppcorn::core;
using namespace cppcorn::http;
using namespace cppcorn::asgi;

// Global bridge definition
namespace cppcorn::http {
    cppcorn::asgi::Bridge* g_bridge = nullptr;
}

#include <cstdio>

int main() {
    std::printf("CppCorn: Initializing...\n");
    std::fflush(stdout);

    try {
        EventLoop& loop = EventLoop::instance();
        std::printf("CppCorn: EventLoop initialized.\n");
        std::fflush(stdout);
        
        Bridge bridge;
        bridge.listen();
        std::printf("CppCorn: Bridge listening.\n");
        std::fflush(stdout);

        bridge.spawn_worker(); 
        bridge.accept_worker(); 
        
        cppcorn::http::g_bridge = &bridge;
        
        // Start HTTP Server
        Server server("0.0.0.0", 8000);
        // We need to keep the server task alive. 
        // In this simple model, we can just fire it if the loop runs indefinitely.
        // However, `run()` is a coroutine, so we need to start it.
        auto server_task = server.run();
        
        loop.run();
    } catch (const std::exception& e) {
        fmt::print("Critial Error: {}\n", e.what());
    }
    return 0;
}
