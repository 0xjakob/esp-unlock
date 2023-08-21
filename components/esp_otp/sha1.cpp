#include <assert.h>
#include "sha1.hpp"
#include "mbedtls/sha1.h"

Sha1::Sha1() {
    mbedtls_sha1_context* sha_ctx = new mbedtls_sha1_context;
    mbedtls_sha1_init(sha_ctx);
    mbedtls_sha1_starts(sha_ctx);
    data = sha_ctx;
}

Sha1::~Sha1() {
    mbedtls_sha1_context* sha_ctx = static_cast<mbedtls_sha1_context*>(data);
    mbedtls_sha1_free(sha_ctx);
    delete sha_ctx;
}

void Sha1::update(const uint8_t* input, size_t len) {
    mbedtls_sha1_context* sha_ctx = static_cast<mbedtls_sha1_context*>(data);
    int ret = mbedtls_sha1_update(sha_ctx, input, len);
    assert(ret == 0);
}

void Sha1::finish(uint8_t* out_buff) {
    mbedtls_sha1_context* sha_ctx = static_cast<mbedtls_sha1_context*>(data);
    int ret = mbedtls_sha1_finish(sha_ctx, out_buff);
    assert(ret == 0);
}
