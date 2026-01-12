# CppCorn Walkthrough

I have implemented the core components of `cppcorn`, a high-performance C++20 ASGI server.

## Features Implemented

1.  **Multi-Platform Event Loop**:
    *   **Linux**: Uses `epoll` with a Reactor pattern.
    *   **Windows**: Uses `IOCP` (I/O Completion Ports) with a Proactor pattern.
    *   **Unified API**: `co_await socket.read(buffer)` works transparently on both platforms using C++20 coroutines.

2.  **HTTP Server** (`src/http/`):
    *   **Parser**: Integrated `llhttp` for fast, compliant HTTP/1.1 parsing.
    *   **Connection**: Manages the request/response lifecycle.
    *   **Server**: Accepts connections (Async/Sync hybrid for prototype).

3.  **ASGI IPC** (`src/asgi/`):
    *   **Protocol**: Hybrid Binary/JSON protocol logic implemented.
    *   **Bridge**: Interface to spawn and communicate with worker processes.
    *   **Worker**: `python/worker.py` shim implemented to run FastAPI/ASGI apps.

## Architecture Highlights

### Coroutines
Uses custom `Task<T>` and `Awaitable` types to implement zero-overhead async I/O.
```cpp
// Works on Windows (IOCP) and Linux (Epoll)
size_t n = co_await socket.read(buffer);
```

### IPC Bridge
Communication uses a framed protocol:
`[Length (4)][Type (1)][Payload (N)]`

## Verification
The code is structured to build with CMake.
*   **Build Command**: `cmake -S . -B cmake-build-debug` -> `cmake --build cmake-build-debug`
*   **Run**: `./cmake-build-debug/cppcorn.exe`
*   **Python Worker**: `python python/worker.py`

## Next Steps
1.  Verify end-to-end request handling with a real FastAPI app.
2.  Implement `AcceptEx` for high-performance Windows accept.
3.  Optimize IPC buffering.
