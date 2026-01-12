#pragma once

#include "platform.hpp"
#include <functional>
#include <vector>
#include <memory>
#include <coroutine>
#include <unordered_map>

namespace cppcorn::core {

#ifdef _WIN32
struct IocpOperation {
    OVERLAPPED overlapped;
    std::coroutine_handle<> handle;
    DWORD bytes_transferred;
    bool success;

    IocpOperation() {
        memset(&overlapped, 0, sizeof(OVERLAPPED));
        handle = nullptr;
        bytes_transferred = 0;
        success = false;
    }
};
#endif

class EventLoop {
public:
    static EventLoop& instance();

    EventLoop();
    ~EventLoop();

    void run();
    void stop();

    // Common Interface
    void register_handle(NativeSocket fd);

#ifdef _WIN32
    // Windows Specific: No explicit add_reader/add_writer. Logic is in Socket.
    HANDLE iocp_handle() const { return iocp_handle_; }
#else
    // Linux Specific: Reactor pattern registration
    void add_reader(int fd, std::coroutine_handle<> handle);
    void remove_reader(int fd);
    void add_writer(int fd, std::coroutine_handle<> handle);
    void remove_writer(int fd);
#endif

private:
    bool running_ = false;

#ifdef _WIN32
    HANDLE iocp_handle_ = NULL;
#else
    int epoll_fd_ = -1;
    struct FdContext {
        std::coroutine_handle<> read_handle;
        std::coroutine_handle<> write_handle;
    };
    std::unordered_map<int, FdContext> fd_map_;
#endif
};

} // namespace cppcorn::core
