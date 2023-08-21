#include <cstdlib>
#include <array>
#include <string_view>
#include <cstring>
#include "esp_err.h"
#include "system_cxx.hpp"
#include "totp_command_list.hpp"

namespace {

const std::vector<uint8_t> TOTP_TOKEN = {'T', 'O', 'T', 'P', '_', 'L', 'I', 'S', 'T', ':', };
const std::vector<uint8_t> STORAGE_ERROR = {'E', 'R', 'R', 'O', 'R', ':', 'S', 'T', 'O', 'R', 'A', 'G', 'E', '\n'};

}

TOTP_RequestList::TOTP_RequestList(std::shared_ptr<OTP_Storage> otp_storage_arg) : otp_storage(otp_storage_arg) { }

const std::vector<uint8_t>& TOTP_RequestList::get_listen_token() const {
    return TOTP_TOKEN;
}

std::vector<uint8_t> TOTP_RequestList::process_content(const std::vector<uint8_t>& content) {
//    printf("content: %s\n", content.c_str());
    std::vector<uint8_t> result(TOTP_TOKEN);
    try {
        OTP_Storage::Iterator it = otp_storage->iterator();
        std::array<char, OTP_Storage::MAX_NAME_SIZE> buffer;
        if (it.valid()) {
            do {
                it.get_name(buffer);
//                printf("name: %s\n", buffer.data());
                result.insert(result.end(), buffer.begin(), buffer.begin() + std::strlen(buffer.data()));
                result.push_back(',');
                ++it;
            } while (it.valid());
        } else {
            result.push_back(',');
        }
    } catch (idf::ESPException& e) {
//        printf("error_code: %s\n", esp_err_to_name(e.error));
        return STORAGE_ERROR;
    }

    result.push_back('\n');

    return result;
}
