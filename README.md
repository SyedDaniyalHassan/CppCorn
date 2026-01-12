# CppCorn Architecture & Implementation Walkthrough

## Overview
CppCorn is a high-performance, asynchronous ASGI HTTP server written in C++20. It is designed to be a drop-in replacement for Uvicorn, capable of running Python ASGI applications (like FastAPI) with superior performance by leveraging native C++ concurrency primitives and efficient I/O models (IOCP on Windows, Epoll on Linux).

## 1. Core System: The Event Loop & Coroutines
The foundation of CppCorn is its asynchronous runtime, built from scratch using C++20 Coroutines.

### 1.1 `Task<T>` and `FireAndForget`
- **Location**: `src/core/coroutine.hpp`
- **Concept**: C++20 coroutines require a "Promise" type to manage state. We implemented:
    - `Task<T>`: A lazily-executed coroutine that returns a value `T`. It is "awaitable", meaning execution pauses until the result is ready.
    - `FireAndForget`: A detached task that starts immediately and manages its own lifetime. Used for "fire-and-forget" operations like handling a new client connection where we don't await the result in the main loop.

### 1.2 The Event Loop
- **Location**: `src/core/event_loop.cpp` & `.hpp`
- **Concept**: The engine that drives asynchronous I/O.
- **Implementation**:
    - **Windows (IOCP)**: Uses Input/Output Completion Ports. This is the most efficient I/O model on Windows. We created a `CreateIoCompletionPort` and use `GetQueuedCompletionStatus` to wait for I/O events.
    - **Linux (Epoll)**: Uses `epoll`. It monitors file descriptors for readiness (Read/Write) and resumes the corresponding coroutine.

## 2. Networking Layer: Sockets & Async I/O
- **Location**: `src/core/socket.cpp` & `.hpp`
- **Concept**: A wrapper around native OS sockets that integrates with the Event Loop.

### 2.1 Asynchronous Operations
Instead of blocking threads, we use "Awaitables":
- **`read()` / `write()`**: On Windows, these call `WSARecv` / `WSASend` (Overlapped I/O). On Linux, they use non-blocking `read`/`write` and suspend if `EAGAIN` is returned.
- **`accept_async()`**:
    - **Windows**: Uses **`AcceptEx`**, a Microsoft-specific extension that allows accepting connections asynchronously without creating a new thread. This was a critical step to achieve high performance.
    - **Linux**: Uses loop-based non-blocking `accept`.

## 3. The HTTP Server
- **Location**: `src/http/server.cpp`
- **Flow**:
    1.  **Initialize**: Binds to an IP/Port (e.g., `0.0.0.0:8000`).
    2.  **Run Loop**:
        -   Calls `accept_async()` to wait for a client.
        -   When a client connects, it spawns a new `Connection` object.
        -   Calls `conn->start()` as a `FireAndForget` task. This ensures the main loop immediately goes back to accepting new clients while the connection is handled concurrently.

## 4. Connection Handling & Parsing
- **Location**: `src/http/connection.cpp`
- **Flow**:
    1.  **Read Loop**: Reads data from the socket into a buffer.
    2.  **Parser**: We use `llhttp` (Node.js HTTP parser) to parse the raw bytes into a request object (Method, Path, Headers).
    3.  **Bridge Handoff**: Once a full request is parsed, it constructs a JSON object representing the **ASGI Scope** and sends it to the `Bridge`.

## 5. The ASGI Bridge (IPC)
- **Location**: `src/asgi/bridge.cpp`
- **Concept**: Since C++ cannot directly run Python code efficiently in the same thread without GIL issues, we run Python in a separate process/worker and communicate via a high-speed Local Socket (TCP Loopback for now).
- **Protocol**:
    -   Simple binary protocol: `[Length (4 bytes)] [Type (1 byte)] [Payload]`
    -   Type 1: JSON (Metadata, Headers)
    -   Type 2: Binary (Body content - conceptual)

## 6. Python Worker
- **Location**: `python/worker.py`
- **Concept**: A standalone Python script that acts as the "Application Server".
- **Steps**:
    1.  **Connect**: Connects to the C++ Bridge on port 8001.
    2.  **Load App**: Dynamically imports the user's ASGI app (e.g., `demo.main:app`).
    3.  **Loop**:
        -   Reads the ASGI Scope JSON from the socket.
        -   Constructs a shim `receive` and `send` awaitable.
        -   Calls `await app(scope, receive, send)`.
        -   Captures the response and sends it back to C++ via IPC.

## Summary of Execution Flow
1.  **User** runs `./cppcorn.exe`.
2.  **CppCorn** starts EventLoop and listens on Port 8000 (HTTP) and Port 8001 (IPC).
3.  **User** runs `python worker.py`.
4.  **Worker** connects to IPC Port 8001.
5.  **Client** (Browser/Curl) sends `GET /` to Port 8000.
6.  **Connection** accepts, reads data, parses HTTP.
7.  **Bridge** sends JSON `{method: "GET", path: "/", ...}` to Worker.
8.  **Worker** executes FastAPI app, generates JSON response `{"message": "Hello..."}`.
9.  **Worker** sends response payload back to Bridge.
10. **Connection** formatting HTTP Response (`HTTP/1.1 200 OK...`) and writes to Client.

## Key Technical Decisions
-   **No Threads for I/O**: We use a single-threaded event loop (for this prototype), minimizing context switching.
-   **Zero-Copy (Goals)**: We use spans and string views where possible to avoid unnecessary copying.
-   **AcceptEx**: Utilizing the most efficient Windows API for accepting connections prevents the "thundering herd" problem and reduces CPU usage.
