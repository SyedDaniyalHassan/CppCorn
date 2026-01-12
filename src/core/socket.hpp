#pragma once

#include "platform.hpp"
#include "event_loop.hpp"
#include "coroutine.hpp"
#include <vector>
#include <span>
#include <optional>

namespace cppcorn::core {

class Socket {
public:
    explicit Socket(NativeSocket fd = INVALID_SOCKET_VAL);
    ~Socket();

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    NativeSocket fd() const { return fd_; }
    void close();

    // Setup
    void bind(const char* ip, int port);
    void listen();
    std::optional<Socket> accept(); // Sync accept for now
    void set_non_blocking();

    // Async Operations
    Task<size_t> read(std::span<char> buffer);
    Task<size_t> write(std::span<const char> buffer);
    Task<Socket> accept_async();

private:
    NativeSocket fd_;

#ifdef _WIN32
    // Windows Awaitable
    struct IocpAwaitable {
        EventLoop& loop;
        NativeSocket fd;
        void* buffer;
        DWORD len;
        bool is_write;
        IocpOperation op;

        bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<> h);
        size_t await_resume();
    };

    struct AcceptAwaitable {
        EventLoop& loop;
        NativeSocket listen_fd;
        NativeSocket accept_fd;
        char buffer[128]; // Buffer for addresses
        IocpOperation op;

        bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<> h);
        NativeSocket await_resume();
    };
#else
    // Linux Awaitables
    struct ReadAwaitable {
        int fd;
        constexpr bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<> h) {
            EventLoop::instance().add_reader(fd, h);
        }
        void await_resume() {}
    };

    struct WriteAwaitable {
        int fd;
        constexpr bool await_ready() const noexcept { return false; }
        void await_suspend(std::coroutine_handle<> h) {
            EventLoop::instance().add_writer(fd, h);
        }
        void await_resume() {}
    };
#endif
};

} // namespace cppcorn::core
