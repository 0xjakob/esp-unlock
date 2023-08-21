idf_component_register(SRCS "sha1.cpp" "hmac_sha1.cpp" "hotp.cpp" "totp.cpp"

                    PRIV_REQUIRES mbedtls
                    INCLUDE_DIRS ".")
