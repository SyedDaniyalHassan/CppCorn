#include "bridge.hpp"
#include "protocol.hpp"
#include <fmt/core.h>
#include <stdexcept>

namespace cppcorn::asgi {

Bridge::Bridge() {
}

Bridge::~Bridge() {}

void Bridge::listen() {
    ipc_socket_ = core::Socket(INVALID_SOCKET_VAL); // Create new
    ipc_socket_.bind("127.0.0.1", 8005);
    ipc_socket_.listen();
    fmt::print("Bridge listening on 127.0.0.1:8005 for Workers...\n");
}

void Bridge::accept_worker() {
    // Sync accept for prototype simplicity
    // In production this would be async loop accepting multiple workers
    fmt::print("Waiting for worker connection...\n");
    // Hack: loop until we get one (blocking-ish but manual)
    // Actually, let's just make it blocking for the 'setup' phase.
    
    // We need to set blocking temporarily? Socket is blocking by default until set_non_blocking called.
    // Our Socket constructor creates a blocking socket by default logic? 
    // Yes, unless set_non_blocking is called.
    
    // However, our `Socket` wrapper wraps `socket()` which is blocking.
    // So `accept` will block.
    auto sock = ipc_socket_.accept();
    if (sock) {
        worker_socket_ = std::move(*sock);
        worker_socket_.set_non_blocking();
        fmt::print("Worker connected!\n");
    } else {
        fmt::print("Failed to accept worker (would block?)\n");
    }
}

void Bridge::spawn_worker() {
    // Manual for now
    fmt::print("Please run 'python python/worker.py' in a separate terminal.\n");
}

core::Task<void> Bridge::send_request(const nlohmann::json& scope) {
    auto data = Protocol::encode(scope);
    co_await worker_socket_.write(std::span(data.data(), data.size()));
}

core::Task<nlohmann::json> Bridge::receive_response() {
    char header_buf[5];
    size_t total = 0;
    while (total < 5) {
        size_t r = co_await worker_socket_.read(std::span(header_buf + total, 5 - total));
        if (r == 0) throw std::runtime_error("IPC Closed");
        total += r;
    }
    
    uint32_t len;
    std::memcpy(&len, header_buf, 4);
    
    size_t payload_len = len - 1;
    std::vector<char> payload(payload_len);
    total = 0;
    while (total < payload_len) {
        size_t r = co_await worker_socket_.read(std::span(payload.data() + total, payload_len - total));
        if (r == 0) throw std::runtime_error("IPC Closed pending payload");
        total += r;
    }

    std::string_view sv(payload.data(), payload_len);
    co_return nlohmann::json::parse(sv);
}

} // namespace cppcorn::asgi
