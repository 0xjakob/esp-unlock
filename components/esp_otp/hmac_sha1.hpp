#include <array>
#include <vector>

void hmac_sha1(const std::vector<uint8_t>& key, const std::vector<uint8_t>& input, std::array<uint8_t, 20>& output);
