#pragma once

#include <string_view>
#include <vector>
#include <cstdint>
#include <exception>

#include "esp_exception.hpp"

class OTP_Storage {
public:
    static constexpr uint8_t MAX_NAME_SIZE = 63;

    class Iterator {
        friend class OTP_Storage;
    private:
        Iterator(size_t first_entry, const void* private_data_arg);
    public:
        void operator++();
        void get_name(std::array<char, MAX_NAME_SIZE>& data);
        bool valid();
    private:
        size_t current_index;
        const void* private_data;
    };

    OTP_Storage(const std::string_view& partition_name);

    std::vector<uint8_t> find(const std::string_view& key_name);

    void insert(const std::string_view& key_name, const std::vector<uint8_t> key);

    void erase(const std::string_view& key_name);

    [[nodiscard]] Iterator iterator();

private:
    size_t find_first_free();

    size_t find_index(const std::string_view& key_name);

    void write_indexed(const std::string_view& key_name, const std::vector<uint8_t>& key, size_t index);

    void erase_indexed(size_t index);

    uint8_t read_key_size(size_t index);

    std::vector<uint8_t> read_key(size_t index, size_t key_size);

    size_t end() const;

    const void* private_data;
};

class OTP_StorageFullException : public std::exception { };
class OTP_StorageNotFoundException : public std::exception { };
