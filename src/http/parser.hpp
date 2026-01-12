#pragma once

#include <llhttp.h>
#include <string>
#include <vector>
#include <functional>
#include <string_view>

namespace cppcorn::http {

struct Request {
    std::string method;
    std::string path;
    int version_major = 1;
    int version_minor = 1;
    std::vector<std::pair<std::string, std::string>> headers;
    std::string body;
    
    // Helper to track state during parsing
    std::string current_header_field;
    std::string current_header_value;
};

class Parser {
public:
    Parser();
    ~Parser();

    // Returns true if parsing is complete (for a single request)
    // Or we handle streaming. 
    // For ASGI, we probably want to stream events.
    // For now, let's buffer the request for simplicity or emit callbacks.
    
    // Feed data. Returns consumed bytes or throws on error.
    void feed(std::string_view data);
    
    // Check if request is ready
    bool is_complete() const { return complete_; }
    
    const Request& request() const { return curr_req_; }
    void reset();

private:
    static int on_message_begin(llhttp_t* p);
    static int on_url(llhttp_t* p, const char* at, size_t length);
    static int on_header_field(llhttp_t* p, const char* at, size_t length);
    static int on_header_value(llhttp_t* p, const char* at, size_t length);
    static int on_headers_complete(llhttp_t* p);
    static int on_body(llhttp_t* p, const char* at, size_t length);
    static int on_message_complete(llhttp_t* p);

    llhttp_t parser_;
    llhttp_settings_t settings_;
    
    Request curr_req_;
    bool complete_ = false;
};

} // namespace cppcorn::http
