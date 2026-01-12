#include "socket.hpp"
#include <stdexcept>
#include <system_error>
#include <fmt/core.h>

namespace cppcorn::core {

#ifndef _WIN32
// Helper to init winsock mock if needed, but we are on linux
#else
// Helper to init winsock on Windows
static struct WinsockInit {
    WinsockInit() {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    }
} winsock_init;
#endif

Socket::Socket(NativeSocket fd) : fd_(fd) {
    if (fd_ == INVALID_SOCKET_VAL) {
        fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd_ == INVALID_SOCKET_VAL) {
            throw std::runtime_error("Failed to create socket");
        }
#ifdef _WIN32
        // Associate with IOCP immediately if we are creating it
        // But better to do it lazily or when adding to loop?
        // Actually EventLoop::register_handle does it.
        // We should probably do it in constructor? 
        // No, we need the loop instance.
        // We will assume user calls register or we do it efficiently.
#endif
    }
}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = INVALID_SOCKET_VAL;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = INVALID_SOCKET_VAL;
    }
    return *this;
}

void Socket::close() {
    if (fd_ != INVALID_SOCKET_VAL) {
        closesocket(fd_);
        fd_ = INVALID_SOCKET_VAL;
    }
}

void Socket::set_non_blocking() {
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(fd_, FIONBIO, &mode);
#else
    int flags = fcntl(fd_, F_GETFL, 0);
    fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
#endif
}

void Socket::bind(const char* ip, int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    // Reuse addr
    int opt = 1;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    if (::bind(fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }
}

void Socket::listen() {
    if (::listen(fd_, SOMAXCONN) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }
}

std::optional<Socket> Socket::accept() {
    sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);
    NativeSocket client_fd = ::accept(fd_, (struct sockaddr*)&client_addr, &len);
    
    if (client_fd == INVALID_SOCKET_VAL) {
        if (is_would_block()) return std::nullopt;
        // Other errors?
        return std::nullopt;
    }
    return Socket(client_fd);
}

// ----------------------------------------------------------------------------
// Async I/O Implementations
// ----------------------------------------------------------------------------

#ifdef _WIN32

// Helper for AcceptEx
static LPFN_ACCEPTEX load_accept_ex(SOCKET s) {
    static LPFN_ACCEPTEX fp = nullptr;
    if (fp) return fp;
    
    GUID guid = WSAID_ACCEPTEX;
    DWORD bytes = 0;
    if (WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
                 &fp, sizeof(fp), &bytes, NULL, NULL) == SOCKET_ERROR) {
        throw std::runtime_error("Failed to load AcceptEx");
    }
    return fp;
}

void Socket::AcceptAwaitable::await_suspend(std::coroutine_handle<> h) {
    op.handle = h;
    
    accept_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (accept_fd == INVALID_SOCKET) {
        op.success = false;
        h.resume();
        return;
    }

    EventLoop::instance().register_handle(listen_fd);

    DWORD bytes = 0;
    auto pAcceptEx = load_accept_ex(listen_fd);
    
    BOOL ret = pAcceptEx(listen_fd, accept_fd, buffer, 0, 
                         sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, 
                         &bytes, &op.overlapped);
                         
    if (ret == FALSE) {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            op.success = false;
            closesocket(accept_fd);
            accept_fd = INVALID_SOCKET;
            h.resume();
        }
    }
}

NativeSocket Socket::AcceptAwaitable::await_resume() {
    if (!op.success || accept_fd == INVALID_SOCKET) {
        if (accept_fd != INVALID_SOCKET) closesocket(accept_fd);
        return INVALID_SOCKET;
    }
    
    setsockopt(accept_fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, 
               (char*)&listen_fd, sizeof(listen_fd));
               
    return accept_fd;
}

void Socket::IocpAwaitable::await_suspend(std::coroutine_handle<> h) {
    op.handle = h;
    DWORD flags = 0;
    
    EventLoop::instance().register_handle(fd);

    int ret = 0;
    if (is_write) {
        WSABUF wbuf;
        wbuf.buf = (char*)buffer;
        wbuf.len = len;
        ret = WSASend(fd, &wbuf, 1, NULL, flags, &op.overlapped, NULL);
    } else {
        WSABUF wbuf;
        wbuf.buf = (char*)buffer;
        wbuf.len = len;
        ret = WSARecv(fd, &wbuf, 1, NULL, &flags, &op.overlapped, NULL);
    }

    if (ret != 0) {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            op.success = false;
            h.resume();
        }
    }
}

size_t Socket::IocpAwaitable::await_resume() {
    if (!op.success) return 0;
    return op.bytes_transferred;
}

Task<Socket> Socket::accept_async() {
    co_return Socket(co_await AcceptAwaitable{
        EventLoop::instance(), fd_, INVALID_SOCKET, {}, {}
    });
}


Task<size_t> Socket::read(std::span<char> buffer) {
    co_return co_await IocpAwaitable{
        EventLoop::instance(), fd_, 
        buffer.data(), (DWORD)buffer.size(), 
        false // read
    };
}

Task<size_t> Socket::write(std::span<const char> buffer) {
    co_return co_await IocpAwaitable{
        EventLoop::instance(), fd_, 
        (void*)buffer.data(), (DWORD)buffer.size(), 
        true // write
    };
}

#else

// Linux Implementation

Task<Socket> Socket::accept_async() {
    while (true) {
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        NativeSocket client_fd = ::accept(fd_, (struct sockaddr*)&client_addr, &len);

        if (client_fd != INVALID_SOCKET_VAL) {
            co_return Socket(client_fd);
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            co_await ReadAwaitable{fd_};
        } else {
            co_return Socket(INVALID_SOCKET_VAL);
        }
    }
}

Task<size_t> Socket::read(std::span<char> buffer) {
    while (true) {
        ssize_t n = ::read(fd_, buffer.data(), buffer.size());
        if (n >= 0) co_return n;
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            co_await ReadAwaitable{fd_};
        } else {
            // Error
            co_return 0; 
        }
    }
}

Task<size_t> Socket::write(std::span<const char> buffer) {
    size_t total = 0;
    while (total < buffer.size()) {
        ssize_t n = ::write(fd_, buffer.data() + total, buffer.size() - total);
        if (n >= 0) {
            total += n;
            if (total == buffer.size()) co_return total;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            co_await WriteAwaitable{fd_};
        } else {
            co_return total; // Partial or error
        }
    }
    co_return total;
}

#endif

} // namespace cppcorn::core
