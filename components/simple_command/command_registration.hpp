#pragma once

#include <string_view>
#include <vector>
#include <stdint.h>
#include <sys/types.h>

inline std::string_view to_string_view(const uint8_t* data, size_t end) {
    return std::string_view(reinterpret_cast<const char*>(data), end);
}

class CommandRegistration {
public:
    virtual const std::vector<uint8_t>& get_listen_token() const = 0;

    virtual std::vector<uint8_t> process_content(const std::vector<uint8_t>& content) = 0;
};
