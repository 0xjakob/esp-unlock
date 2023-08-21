#include <string.h>
#include "sha1.hpp"
#include "hmac_sha1.hpp"

namespace {

constexpr size_t SHA1_BLOCK_LEN = 64;

}

void hmac_sha1(const std::vector<uint8_t>& key,
        const std::vector<uint8_t>& input,
        std::array<uint8_t, 20>& output) {
    assert(key.size() <= SHA1_BLOCK_LEN); // TODO: implement key > block length
    std::vector<uint8_t> key_block(SHA1_BLOCK_LEN);
    
    // (1)
    memcpy(key_block.data(), key.data(), key.size());
    for (size_t i = key.size(); i < SHA1_BLOCK_LEN; i++) {
        key_block[i] = 0;
    }

    // (2)
    for (size_t i = 0; i < SHA1_BLOCK_LEN; i++) {
        key_block[i] = key_block[i] ^ 0x36;
    }

    // (3)
    for (uint8_t input_byte: input) {
        key_block.push_back(input_byte);
    }

    // (4)
    std::array<uint8_t, 20> inner_hash;
    Sha1 sha;
    sha.update(key_block.data(), key_block.size());
    sha.finish(inner_hash.data());

    // (5)
    key_block = std::vector<uint8_t>(SHA1_BLOCK_LEN);
    memcpy(key_block.data(), key.data(), key.size());
    for (size_t i = key.size(); i < SHA1_BLOCK_LEN; i++) {
        key_block[i] = 0;
    }

    for (size_t i = 0; i < SHA1_BLOCK_LEN; i++) {
        key_block[i] = key_block[i] ^ 0x5C;
    }

    // (6)
    for (uint8_t hash_byte: inner_hash) {
        key_block.push_back(hash_byte);
    }

    // (7)
    Sha1 sha2;
    sha2.update(key_block.data(), key_block.size());
    sha2.finish(output.data());
}
