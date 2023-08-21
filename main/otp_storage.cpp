#include <array>
#include <cstring>
#include "esp_partition.h"
#include "otp_storage.hpp"
#include <cstdio>

namespace {

constexpr uint8_t NAME_OFFSET = 0;
constexpr uint8_t KEY_SIZE_OFFSET = 63;
constexpr uint8_t KEY_SIZE_SIZE = 1;
constexpr uint8_t KEY_OFFSET = 64;
constexpr uint8_t MAX_KEY_SIZE = 32;
constexpr uint8_t HASH_OFFSET = 64 + 32;
constexpr uint8_t HASH_SIZE = 32;
constexpr uint8_t MAX_KEY_NUM = 32; // one key + name entry occupies 128 bytes, the partition is 4K for now
constexpr size_t ENTRY_LEN = 128; // one key + name entry occupies 128 bytes, the partition is 4K for now

}

OTP_Storage::OTP_Storage(const std::string_view& partition_name) {
    const esp_partition_t* partition = esp_partition_find_first(static_cast<esp_partition_type_t>(0x40),
            ESP_PARTITION_SUBTYPE_ANY,
            "totp");
    if (partition == nullptr) {
        throw idf::ESPException(ESP_FAIL);
    }

    private_data = partition;
}

std::vector<uint8_t> OTP_Storage::find(const std::string_view& key_name) {
    size_t index = find_index(key_name);
    if (index != end()) {
         uint8_t key_size = read_key_size(index);
         return read_key(index, key_size);
    }

    throw OTP_StorageNotFoundException();
}

void OTP_Storage::insert(const std::string_view& key_name, const std::vector<uint8_t> key) {
    if (key_name.size() > MAX_NAME_SIZE) {
        throw idf::ESPException(ESP_ERR_INVALID_ARG);
    }

    if (key.size() > MAX_KEY_SIZE) {
        throw idf::ESPException(ESP_ERR_INVALID_ARG);
    }

    size_t index;
    index = find_index(key_name);
    if (index != end()) {
        erase_indexed(index);
    } else {
        index = find_first_free();
    }

    write_indexed(key_name, key, index);
}

void OTP_Storage::erase(const std::string_view& key_name) {
    size_t index;
    index = find_index(key_name);
    if (index != end()) {
        erase_indexed(index);
    }
}

size_t OTP_Storage::find_first_free() {
    const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(private_data);
    for (size_t i = 0; i < (partition->size / ENTRY_LEN); i ++) {
        uint32_t test_read;
        esp_err_t ret = esp_partition_read(partition, i * ENTRY_LEN, &test_read, sizeof(test_read));
        if (ret != ESP_OK) {
            throw idf::ESPException(ret);
        }

        if (test_read == 0xFFFFFFFF) {
            return i;
        }
    }

    throw OTP_StorageFullException();
}

size_t OTP_Storage::find_index(const std::string_view& key_name) {
    const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(private_data);
    std::array<char, MAX_NAME_SIZE> name_buffer;
    size_t i;
    for (i = 0; i < (partition->size / ENTRY_LEN); i++) {
        esp_err_t ret = esp_partition_read(partition, i * ENTRY_LEN, name_buffer.data(), name_buffer.size());
        if (ret != ESP_OK) {
            throw idf::ESPException(ret);
        }

        if (*reinterpret_cast<uint32_t*>(name_buffer.data()) == 0xFFFFFFFF) {
            continue;
        }

        for (size_t j = 0; j < name_buffer.size(); j++) {
            if (name_buffer[j] == '\0') {
                if (std::string_view(name_buffer.data()) == key_name) {
                    return i;
                }
            }
        }
    }

    return end();
}

void OTP_Storage::write_indexed(const std::string_view& key_name, const std::vector<uint8_t>& key, size_t index) {
    const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(private_data);
    std::array<uint8_t, ENTRY_LEN> write_data;
    write_data.fill(0xFF);

    printf("key_name size: %u\n", key_name.size());

    // write key_name to entry
    std::memcpy(write_data.data(), key_name.data(), key_name.size());
    write_data[key_name.size()] = '\0';

    // write key size to entry
    write_data[KEY_SIZE_OFFSET] = static_cast<uint8_t>(key.size());
    printf("key_name: %s\n", write_data.data());

    // write key to entry
    for (size_t i = 0; i < key.size(); i++) {
        write_data[KEY_OFFSET + i] =  key[i];
    }

    esp_err_t ret;

    // TODO: sector size is 0x1000, need to erase 32 entries at once...
//    // erase before writing
//    ret = esp_partition_erase_range(partition, index * ENTRY_LEN, ENTRY_LEN);
//    printf("index: %u\n", index);
//    if (ret != ESP_OK) {
//        throw idf::ESPException(ret);
//    }

    // write entire entry to flash
    ret = esp_partition_write(partition, index * ENTRY_LEN, write_data.data(), ENTRY_LEN);
    if (ret != ESP_OK) {
        throw idf::ESPException(ret);
    }
}

void OTP_Storage::erase_indexed(size_t index) {
    const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(private_data);
    esp_err_t ret = esp_partition_erase_range(partition, index * ENTRY_LEN, ENTRY_LEN);
    if (ret != ESP_OK) {
        throw idf::ESPException(ret);
    }
}

uint8_t OTP_Storage::read_key_size(size_t index) {
    const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(private_data);
    uint8_t key_size;
    esp_err_t ret;
    size_t read_pos = index * ENTRY_LEN + KEY_SIZE_OFFSET;
    ret = esp_partition_read(partition, read_pos, &key_size, KEY_SIZE_SIZE);
    if (ret != ESP_OK) {
        throw idf::ESPException(ret);
    }

    return key_size;
}

std::vector<uint8_t> OTP_Storage::read_key(size_t index, size_t key_size) {
    const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(private_data);
    std::vector<uint8_t> key(key_size);

    esp_err_t ret;
    ret = esp_partition_read(partition, index * ENTRY_LEN + KEY_OFFSET, key.data(), key_size);
    if (ret != ESP_OK) {
        throw idf::ESPException(ret);
    }

    return key;
}

size_t OTP_Storage::end() const {
    const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(private_data);
    return partition->size / ENTRY_LEN;
}

[[nodiscard]] OTP_Storage::Iterator OTP_Storage::iterator() {
    const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(private_data);
    for (size_t index = 0; index < (partition->size / ENTRY_LEN); index++) {
        uint32_t test_read;
        esp_err_t ret = esp_partition_read(partition, index * ENTRY_LEN, &test_read, sizeof(test_read));
        if (ret != ESP_OK) {
            throw idf::ESPException(ret);
        }
        if (test_read != 0xFFFFFFFF) {
            return Iterator(index, private_data);
        }
    }
    return Iterator(end(), private_data);
}


OTP_Storage::Iterator::Iterator(size_t first_entry, const void* private_data_arg)
        : current_index(first_entry),
        private_data(private_data_arg) {
}

void OTP_Storage::Iterator::operator++() {
    if (!valid()) {
        throw std::exception();
    }

    const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(private_data);
    uint32_t test_read = 0xFFFFFFFF;
    current_index++;
    while (valid()) {
        esp_err_t ret = esp_partition_read(partition, current_index * ENTRY_LEN, &test_read, sizeof(test_read));
        if (ret != ESP_OK) {
            throw idf::ESPException(ret);
        }
        if (test_read != 0xFFFFFFFF) {
            break;
        }
        current_index++;
    }
}

void OTP_Storage::Iterator::get_name(std::array<char, MAX_NAME_SIZE>& data) {
    if (!valid()) {
        throw std::exception();
    }

    const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(private_data);
    esp_err_t ret;
    ret = esp_partition_read(partition, current_index * ENTRY_LEN + NAME_OFFSET, data.data(), KEY_SIZE_OFFSET);
    if (ret != ESP_OK) {
        throw idf::ESPException(ret);
    }
}

bool OTP_Storage::Iterator::valid() {
    const esp_partition_t* partition = reinterpret_cast<const esp_partition_t*>(private_data);
    return current_index < (partition->size / ENTRY_LEN);
}
