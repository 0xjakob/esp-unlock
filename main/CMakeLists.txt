idf_component_register(SRCS "esp_key.cpp" "totp_command.cpp" "otp_storage.cpp" "totp_command_add.cpp" "totp_command_list.cpp"
                    PRIV_REQUIRES "esp_otp" "simple_command" "driver" "esp_partition"
                    INCLUDE_DIRS ".")
