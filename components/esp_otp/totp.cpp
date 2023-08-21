#include "totp.hpp"
#include "hotp.hpp"

uint32_t totp(std::vector<uint8_t>& key, uint64_t unix_time, uint64_t step_size) {
    uint64_t input_time = unix_time / step_size;
    return hotp_sha1(key, input_time);
}
