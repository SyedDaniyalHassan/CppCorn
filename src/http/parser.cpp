#include "parser.hpp"
#include <stdexcept>

namespace cppcorn::http {

Parser::Parser() {
    llhttp_settings_init(&settings_);
    settings_.on_message_begin = on_message_begin;
    settings_.on_url = on_url;
    settings_.on_header_field = on_header_field;
    settings_.on_header_value = on_header_value;
    settings_.on_headers_complete = on_headers_complete;
    settings_.on_body = on_body;
    settings_.on_message_complete = on_message_complete;

    llhttp_init(&parser_, HTTP_REQUEST, &settings_);
    parser_.data = this;
}

Parser::~Parser() {}

void Parser::reset() {
    llhttp_reset(&parser_);
    complete_ = false;
    curr_req_ = Request{};
}

void Parser::feed(std::string_view data) {
    enum llhttp_errno err = llhttp_execute(&parser_, data.data(), data.size());
    if (err != HPE_OK) {
        throw std::runtime_error(std::string("HTTP Parse Error: ") + llhttp_errno_name(err));
    }
}

int Parser::on_message_begin(llhttp_t* p) {
    return 0;
}

int Parser::on_url(llhttp_t* p, const char* at, size_t length) {
    Parser* self = (Parser*)p->data;
    self->curr_req_.path.append(at, length);
    return 0;
}

int Parser::on_header_field(llhttp_t* p, const char* at, size_t length) {
    Parser* self = (Parser*)p->data;
    if (!self->curr_req_.current_header_value.empty()) {
        // flush previous header
        self->curr_req_.headers.emplace_back(
            std::move(self->curr_req_.current_header_field),
            std::move(self->curr_req_.current_header_value)
        );
        self->curr_req_.current_header_field.clear();
        self->curr_req_.current_header_value.clear();
    }
    self->curr_req_.current_header_field.append(at, length);
    return 0;
}

int Parser::on_header_value(llhttp_t* p, const char* at, size_t length) {
    Parser* self = (Parser*)p->data;
    self->curr_req_.current_header_value.append(at, length);
    return 0;
}

int Parser::on_headers_complete(llhttp_t* p) {
    Parser* self = (Parser*)p->data;
    if (!self->curr_req_.current_header_field.empty()) {
        self->curr_req_.headers.emplace_back(
            std::move(self->curr_req_.current_header_field),
            std::move(self->curr_req_.current_header_value)
        );
    }
    self->curr_req_.method = llhttp_method_name((llhttp_method_t)p->method);
    self->curr_req_.version_major = p->http_major;
    self->curr_req_.version_minor = p->http_minor;
    return 0;
}

int Parser::on_body(llhttp_t* p, const char* at, size_t length) {
    Parser* self = (Parser*)p->data;
    self->curr_req_.body.append(at, length);
    return 0;
}

int Parser::on_message_complete(llhttp_t* p) {
    Parser* self = (Parser*)p->data;
    self->complete_ = true;
    return 0;
}

} // namespace cppcorn::http
