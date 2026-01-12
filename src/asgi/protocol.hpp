#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <span>
#include <nlohmann/json.hpp>

namespace cppcorn::asgi {

enum class MessageType : uint8_t {
    JSON = 1,
    BINARY = 2 // For optimization later (e.g. file descriptors or raw bytes)
};

// Protocol: [4 bytes Length (Big Endian or host? Host is faster for local IPC)][1 byte Type][Payload]
// Let's use Host Endian for IPC on same machine, assuming same endianness (safe for UDS/Loopback).

struct Message {
    MessageType type;
    nlohmann::json data;
    // std::vector<char> binary_data; // If mixed
};

class Protocol {
public:
    static std::vector<char> encode(const nlohmann::json& payload) {
        std::string s = payload.dump();
        uint32_t len = s.size() + 1; // +1 for type
        
        std::vector<char> buffer(sizeof(uint32_t) + 1 + s.size());
        
        // Header
        std::memcpy(buffer.data(), &len, sizeof(len));
        buffer[sizeof(uint32_t)] = (uint8_t)MessageType::JSON;
        
        // Payload
        std::memcpy(buffer.data() + sizeof(uint32_t) + 1, s.data(), s.size());
        
        return buffer;
    }

    // Returns number of bytes consumed if full message, else 0
    static size_t try_decode(std::span<const char> buffer, Message& out_msg) {
        if (buffer.size() < sizeof(uint32_t) + 1) return 0;
        
        uint32_t len;
        std::memcpy(&len, buffer.data(), sizeof(len));
        
        if (buffer.size() < sizeof(uint32_t) + len) {
            return 0; // Incomplete
        }
        
        uint8_t type = buffer[sizeof(uint32_t)];
        if (type == (uint8_t)MessageType::JSON) {
            out_msg.type = MessageType::JSON;
            std::string_view sv(buffer.data() + sizeof(uint32_t) + 1, len - 1);
            out_msg.data = nlohmann::json::parse(sv);
        }
        
        return sizeof(uint32_t) + len;
    }
};

} // namespace cppcorn::asgi
