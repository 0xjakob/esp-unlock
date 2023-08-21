#include <cstdio>
#include <string>
#include "esp_hmac.h"
#include "hash_command.hpp"

using namespace std;

namespace {
const string ERROR_STRING = "ERROR:";
const string HASH_HEADER = "HASH:";
}

HashRequest::HashRequest() { }

string HashRequest::get_listen_token() {
    return string("PASSPHRASE:");
}

string HashRequest::process_content(const std::vector<uint8_t>& content) {
    uint8_t* hmac = new uint8_t[32];
    char* hex_string_hmac = new char[100];
    size_t answer_position = 0;

    esp_err_t hmac_result = esp_hmac_calculate(HMAC_KEY0,
            content.data(),
            content.size(),
            hmac);

    if (hmac_result != ESP_OK) {
        answer_position += sprintf(hex_string_hmac, ERROR_STRING.c_str());
    } else {
        answer_position += sprintf(hex_string_hmac, HASH_HEADER.c_str());
        for (int i = 0; i < 32; i++) {
            answer_position += sprintf(hex_string_hmac + answer_position, "%02X", hmac[i]);
        }
    }

    string answer(hex_string_hmac, hex_string_hmac + answer_position);

    delete [] hex_string_hmac;
    delete [] hmac;

    answer.push_back('\n');
    return answer;
}
