#include <cstdlib>
#include <vector>
#include <array>
#include "esp_partition.h"
#include "system_cxx.hpp"
#include "totp_command.hpp"
#include "totp.hpp"

namespace {
//std::vector<uint8_t> hmac_key = {0x08, 0x24, 0x4d, 0xea, 0x44, 0x14, 0x49, 0x3d, 0xeb, 0x7a};
const std::string TOTP_HEADER = "TOTP:";

const std::vector<uint8_t> TOTP_TOKEN = {'T', 'O', 'T', 'P', ':'};

}

TOTP_Request::TOTP_Request(std::shared_ptr<OTP_Storage> otp_storage_arg) : otp_storage(otp_storage_arg) { }

const std::vector<uint8_t>&  TOTP_Request::get_listen_token() const {
    return TOTP_TOKEN;
}

std::vector<uint8_t> TOTP_Request::process_content(const std::vector<uint8_t>& content) {
    size_t comma_pos = to_string_view(content.data(), content.size()).find(',');
    if (comma_pos == std::string_view::npos) {
        return {'E', 'R', 'R', 'O', 'R', ':', 'F', 'O', 'R', 'M', 'A', 'T',
                ',', 'm', 'i', 's', 's', '_', 'c', 'o', 'm', 'm', 'a', '\n'};
    }
    std::string_view key_name(to_string_view(content.data(), comma_pos));
    if (key_name.empty()) {
        return {'E', 'R', 'R', 'O', 'R', ':', 'F', 'O', 'R', 'M', 'A', 'T',
                ',', 'k', 'e', 'y', '_', 'e', 'm', 'p', 't', 'y', '\n'};
    }

    std::string raw_number(reinterpret_cast<const char*>(content.data()) + comma_pos + 1);
    uint64_t time = strtoull(raw_number.c_str(), nullptr, 10);

    if (time == 0) {
        return {'E', 'R', 'R', 'O', 'R', ':', 'F', 'O', 'R', 'M', 'A', 'T', ',', 't', 'i', 'm', 'e', '\n'};
    }

    std::vector<uint8_t> hmac_key = otp_storage->find(key_name);
//    for (size_t i = 0; i < hmac_key.size(); i++) {
//        printf("%02X, ", hmac_key[i]);
//    }
//    printf("\n");
    uint32_t totp_code = totp(hmac_key, time);

    // Buffer should be large enough for header, up to 8-digit code, and \n
    std::vector<uint8_t> totp_buf(TOTP_HEADER.size() + 9);
    sprintf(reinterpret_cast<char*>(totp_buf.data()), "%s%06lu\n", TOTP_HEADER.c_str(), totp_code);
    return totp_buf;
}
