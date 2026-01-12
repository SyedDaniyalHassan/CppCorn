#include "event_loop.hpp"
#include <fmt/core.h>
#include <stdexcept>

namespace cppcorn::core {

EventLoop& EventLoop::instance() {
    static EventLoop loop;
    return loop;
}

#ifdef _WIN32
// ============================================================================
// Windows IOCP Implementation
// ============================================================================

EventLoop::EventLoop() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    // Create IOCP
    iocp_handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (!iocp_handle_) {
        throw std::runtime_error("Failed to create IOCP");
    }
}

EventLoop::~EventLoop() {
    if (iocp_handle_) CloseHandle(iocp_handle_);
    WSACleanup();
}

void EventLoop::register_handle(NativeSocket fd) {
    // Use 0 as completion key since we rely on OVERLAPPED pointer for context
    if (CreateIoCompletionPort((HANDLE)fd, iocp_handle_, 0, 0) == NULL) {
        DWORD err = GetLastError();
        if (err != ERROR_INVALID_PARAMETER) { // Ignore 87 if already associated
             throw std::runtime_error(fmt::format("Failed to associate handle with IOCP: {}", err));
        }
    }
}

void EventLoop::run() {
    running_ = true;
    fmt::print("EventLoop (IOCP) started.\n");

    DWORD bytes_transferred;
    ULONG_PTR completion_key;
    LPOVERLAPPED overlapped;

    while (running_) {
        BOOL ok = GetQueuedCompletionStatus(
            iocp_handle_,
            &bytes_transferred,
            &completion_key,
            &overlapped,
            INFINITE
        );

        if (!overlapped) {
            // General error or exit signal (if we posted one)
            if (GetLastError() == ERROR_ABANDONED_WAIT_0) break;
            continue;
        }

        // The OVERLAPPED struct is assumed to be embedded in our IocpOperation struct
        // or derived from it.
        auto* op = reinterpret_cast<IocpOperation*>(overlapped);
        
        // Resume the coroutine
        if (op->handle) {
            // We can pass the result back to the coroutine if needed
            op->bytes_transferred = bytes_transferred;
            op->success = ok;
            op->handle.resume();
        }
    }
}

void EventLoop::stop() {
    running_ = false;
    PostQueuedCompletionStatus(iocp_handle_, 0, 0, NULL);
}

#else
// ============================================================================
// Linux Epoll Implementation
// ============================================================================

EventLoop::EventLoop() {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        throw std::runtime_error("Failed to create epoll fd");
    }
}

EventLoop::~EventLoop() {
    if (epoll_fd_ >= 0) close(epoll_fd_);
}

void EventLoop::register_handle(NativeSocket fd) {
    // No-op for readiness model until we add interest (add_reader/writer)
    // But we might want to track it
}

void EventLoop::run() {
    running_ = true;
    const int MAX_EVENTS = 64;
    struct epoll_event events[MAX_EVENTS];

    fmt::print("EventLoop (Epoll) started.\n");

    while (running_) {
        int nfds = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd; // Simplified
            auto it = fd_map_.find(fd);
            if (it == fd_map_.end()) continue;

            if (events[i].events & EPOLLIN) {
                if (it->second.read_handle) {
                    auto h = it->second.read_handle;
                    it->second.read_handle = nullptr;
                    h.resume();
                }
            }
            if (events[i].events & EPOLLOUT) {
                if (it->second.write_handle) {
                    auto h = it->second.write_handle;
                    it->second.write_handle = nullptr;
                    h.resume();
                }
            }
        }
    }
}

void EventLoop::stop() {
    running_ = false;
}

void EventLoop::add_reader(int fd, std::coroutine_handle<> handle) {
    auto& ctx = fd_map_[fd];
    ctx.read_handle = handle;

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLONESHOT;
    ev.data.fd = fd;
    
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
         epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
    }
}

void EventLoop::remove_reader(int fd) {
    if (fd_map_.count(fd)) fd_map_[fd].read_handle = nullptr;
}

void EventLoop::add_writer(int fd, std::coroutine_handle<> handle) {
    auto& ctx = fd_map_[fd];
    ctx.write_handle = handle;

    struct epoll_event ev;
    ev.events = EPOLLOUT | EPOLLONESHOT;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
    }
}

void EventLoop::remove_writer(int fd) {
    if (fd_map_.count(fd)) fd_map_[fd].write_handle = nullptr;
}

#endif

} // namespace cppcorn::core
