#pragma once

#include "command_registration.hpp"

class HashRequest : public CommandRegistration {
public:
    HashRequest();

    const std::vector<uint8_t>& get_listen_token() const override;

    std::vector<uint8_t> process_content(const std::vector<uint8_t>& content) override;
};
