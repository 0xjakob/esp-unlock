#pragma once

#include <memory>
#include "command_registration.hpp"
#include "otp_storage.hpp"

class TOTP_RequestList : public CommandRegistration {
public:
    TOTP_RequestList(std::shared_ptr<OTP_Storage> otp_storage_arg);

    const std::vector<uint8_t>& get_listen_token() const override;

    std::vector<uint8_t> process_content(const std::vector<uint8_t>& content) override;

private:
    std::shared_ptr<OTP_Storage> otp_storage;
};
