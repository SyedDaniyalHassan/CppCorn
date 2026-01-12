#pragma once

#include <coroutine>
#include <exception>
#include <variant>

namespace cppcorn::core {

template <typename T = void>
class Task {
public:
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T value;
        std::exception_ptr exception;

        Task get_return_object() {
            return Task(handle_type::from_promise(*this));
        }

        std::suspend_always initial_suspend() { return {}; }

        struct FinalAwaitable {
            bool await_ready() const noexcept { return false; }
            void await_suspend(handle_type h) noexcept {
                auto continuation = h.promise().continuation;
                if (continuation) {
                    continuation.resume();
                }
            }
            void await_resume() noexcept {}
        };

        FinalAwaitable final_suspend() noexcept { return {}; }

        void unhandled_exception() {
            exception = std::current_exception();
        }

        template <typename U>
        requires std::convertible_to<U, T>
        void return_value(U&& val) {
            value = std::forward<U>(val);
        }

        std::coroutine_handle<> continuation = nullptr;
    };

    Task(handle_type h) : handle(h) {}
    ~Task() {
        if (handle) handle.destroy();
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task(Task&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (handle) handle.destroy();
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    bool await_ready() const { return handle.done(); }

    void await_suspend(std::coroutine_handle<> continuation) {
        handle.promise().continuation = continuation;
        handle.resume();
    }

    T await_resume() {
        if (handle.promise().exception) {
            std::rethrow_exception(handle.promise().exception);
        }
        return std::move(handle.promise().value);
    }

private:
    handle_type handle;
};

// Specialization for void
template <>
class Task<void> {
public:
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        std::exception_ptr exception;

        Task get_return_object() {
            return Task(handle_type::from_promise(*this));
        }

        std::suspend_always initial_suspend() { return {}; }

        struct FinalAwaitable {
            bool await_ready() const noexcept { return false; }
            void await_suspend(handle_type h) noexcept {
                auto continuation = h.promise().continuation;
                if (continuation) {
                    continuation.resume();
                }
            }
            void await_resume() noexcept {}
        };

        FinalAwaitable final_suspend() noexcept { return {}; }

        void unhandled_exception() {
            exception = std::current_exception();
        }

        void return_void() {}

        std::coroutine_handle<> continuation = nullptr;
    };

    Task(handle_type h) : handle(h) {}
    ~Task() {
        if (handle) handle.destroy();
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task(Task&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (handle) handle.destroy();
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    bool await_ready() const { return handle.done(); }

    void await_suspend(std::coroutine_handle<> continuation) {
        handle.promise().continuation = continuation;
        handle.resume();
    }

    void await_resume() {
        if (handle.promise().exception) {
            std::rethrow_exception(handle.promise().exception);
        }
    }

private:
    handle_type handle;
};

// Fire and forget task (detached)
struct FireAndForget {
    struct promise_type {
        FireAndForget get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {
            // Log or terminate. For now, just terminate to be safe.
            std::terminate();
        }
    };
};

} // namespace cppcorn::core
