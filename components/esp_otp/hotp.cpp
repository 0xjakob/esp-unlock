#include <array>
#include <cstddef>
#include "hotp.hpp"
#include "hmac_sha1.hpp"

namespace {

uint32_t dynamic_truncation(std::array<uint8_t, 20>& hash) {
    uint8_t offset = (hash.back() & 0x0F);
    uint32_t p = 0;

    p = p | hash[offset + 3];
    p = p | (hash[offset + 2] << 8);
    p = p | (hash[offset + 1] << 16);
    p = p | ((hash[offset]) << 24);
    p = p & 0x7FFFFFFF;
    return p;
}

uint32_t truncate_result(std::array<uint8_t, 20>& hash) {
    return dynamic_truncation(hash) % 1'000'000;
}

}

uint32_t hotp_sha1(const std::vector<uint8_t>& key, uint64_t counter) {
    std::vector<uint8_t> counter_msb;
    for (size_t i = 0; i < 8; i++) {
        counter_msb.push_back((counter >> (56 - (i * 8))) & 0xFF);
    }

    std::array<uint8_t, 20> hmac_result;
    hmac_sha1(key, counter_msb, hmac_result);

    return truncate_result(hmac_result);
}
