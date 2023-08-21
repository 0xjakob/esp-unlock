#include <vector>
#include <cstdint>

uint32_t totp(std::vector<uint8_t>& key, uint64_t unix_time, uint64_t step_size = 30);
