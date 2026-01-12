#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <mswsock.h>
    
    // Link definitions
    #ifdef _MSC_VER
    #pragma comment(lib, "Ws2_32.lib")
    #pragma comment(lib, "Mswsock.lib")
    #endif

    using NativeSocket = SOCKET;
    constexpr NativeSocket INVALID_SOCKET_VAL = INVALID_SOCKET;

    inline int get_last_error() { return WSAGetLastError(); }
    inline bool is_would_block() { return get_last_error() == WSAEWOULDBLOCK; }

#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/epoll.h>
    #include <errno.h>

    using NativeSocket = int;
    constexpr NativeSocket INVALID_SOCKET_VAL = -1;

    inline int get_last_error() { return errno; }
    inline bool is_would_block() { return errno == EAGAIN || errno == EWOULDBLOCK; }
    inline void closesocket(int fd) { close(fd); }
#endif
