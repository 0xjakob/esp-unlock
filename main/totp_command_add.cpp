#include <cstdlib>
#include <string_view>
#include "esp_err.h"
#include "esp_partition.h"
#include "system_cxx.hpp"
#include "totp_command_add.hpp"

namespace {
//std::vector<uint8_t> hmac_key = {0x08, 0x24, 0x4d, 0xea, 0x44, 0x14, 0x49, 0x3d, 0xeb, 0x7a};
const std::vector<uint8_t> FORMAT_ERROR_STRING =
     {'E', 'R', 'R', 'O', 'R', ',', 'F', 'O', 'R', 'M', 'A', 'T', '\n'};
const std::vector<uint8_t> STORAGE_ERROR_STRING =
     {'E', 'R', 'R', 'O', 'R', ',', 'S', 'T', 'O', 'R', 'A', 'G', 'E', '\n'};
const std::vector<uint8_t> TOTP_HEADER = {'T', 'O', 'T', 'P', '_', 'A', 'D', 'D', ':'};

std::vector<uint8_t> convert_hex_to_bin(const std::string_view& hex_coded_key) {
    std::vector<uint8_t> key;
    for (size_t i = 0; i < hex_coded_key.size(); i = i + 2) {
        // really inefficient due to constructing string, but we don't have stoul for string_view
        unsigned long byte = std::stoul(std::string(hex_coded_key.data() + i, hex_coded_key.data() + i + 2), nullptr, 16);
        key.push_back(static_cast<uint8_t>(byte));
    }
    return key;
}

}

TOTP_RequestAdd::TOTP_RequestAdd(std::shared_ptr<OTP_Storage> otp_storage_arg) : otp_storage(otp_storage_arg) { }

const std::vector<uint8_t>& TOTP_RequestAdd::get_listen_token() const {
    return TOTP_HEADER;
}

std::vector<uint8_t> TOTP_RequestAdd::process_content(const std::vector<uint8_t>& content) {
//    printf("content: %s\n", content.c_str());
    size_t comma_pos = to_string_view(content.data(), content.size()).find(',');
    if (comma_pos == std::string_view::npos) {
        return FORMAT_ERROR_STRING;
    }
    printf("comma_pos: %u\n", comma_pos);
    const std::string_view key_name(to_string_view(content.data(), comma_pos));
    if (key_name.empty()) {
        return FORMAT_ERROR_STRING;
    }

    const std::string_view hex_key(to_string_view(content.data() + comma_pos + 1, content.size() - comma_pos - 1));
    if (hex_key.size() % 2 != 0) {
        return FORMAT_ERROR_STRING;
    }

    std::vector<uint8_t> key = convert_hex_to_bin(hex_key);

    if (key.empty()) {
        return FORMAT_ERROR_STRING;
    }

    try {
        otp_storage->insert(key_name, key);
    } catch (idf::ESPException& e) {
        return STORAGE_ERROR_STRING;
//        printf("error_code: %s\n", esp_err_to_name(e.error));
    }

    return {'T', 'O', 'T', 'P', '_', 'A', 'D', 'D', ':', 'O', 'K', '\n'};
}
