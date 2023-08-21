#include <vector>
#include <cstdint>

uint32_t hotp_sha1(const std::vector<uint8_t>& key, uint64_t counter);
