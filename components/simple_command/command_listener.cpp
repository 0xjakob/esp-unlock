#include "driver/usb_serial_jtag.h"
#include "command_listener.hpp"
#include <string>
#include <algorithm>

namespace {
const size_t WAIT_TIME = 100;
}

CommandListener::CommandListener() : serial_buf() {
}

CommandListener::~CommandListener() { }

void CommandListener::add_command_registration(std::shared_ptr<CommandRegistration> reg) {
    registry.push_back(reg);
}

void CommandListener::run() {
    std::vector<uint8_t> usb_read;
    std::vector<uint8_t>::iterator found_marker = usb_read.end();
    bool searching = true;
    std::shared_ptr<CommandRegistration> current_reg;

    // find tag
    while (searching) {
        if (usb_read.size() > BUF_SIZE) {
            usb_read.erase(usb_read.begin(), usb_read.begin() + usb_read.size() - BUF_SIZE);
        }

        // Read data from the UART
        int len = usb_serial_jtag_read_bytes(serial_buf.data(), serial_buf.size(), WAIT_TIME / portTICK_PERIOD_MS);
        if (len > 0) {
            usb_read.insert(usb_read.end(), serial_buf.data(), serial_buf.data() + len);
        }

        for (auto it = registry.begin(); it != registry.end(); ++it) {
            const std::vector<uint8_t>& token = (*it)->get_listen_token();
            found_marker = std::find_end(usb_read.begin(), usb_read.end(), token.begin(), token.end());
            if (found_marker != usb_read.end()) {
                current_reg = *it;
                searching = false;
                break;
            }
        }
    }

    usb_read.erase(usb_read.begin(), found_marker + current_reg->get_listen_token().size());

    // read content after tag
    // TODO: implement timeout
    while (1) {
        // Read data from the UART
        int len = usb_serial_jtag_read_bytes(serial_buf.data(), serial_buf.size(), 0);
        // Write data back to the UART
        if (len > 0) {
            usb_read.insert(usb_read.end(), serial_buf.data(), serial_buf.data() + len);
        }
        found_marker = std::find(usb_read.begin(), usb_read.end(), '\r');
        if (found_marker != usb_read.end()) {
            break;
        }
        found_marker = std::find(usb_read.begin(), usb_read.end(), '\n');
        if (found_marker != usb_read.end()) {
            break;
        }
        vTaskDelay(1); // try to prevent WDT timemouts
    }

    usb_read.erase(found_marker, usb_read.end());

    // process content after tag
    std::vector<uint8_t> answer = current_reg->process_content(usb_read);
//    answer.push_back('\n'); // TODO: USB serial delays sending if two newlines are sent on after another

    size_t write_counter = 0;
    while (write_counter < answer.size()) {
        size_t wr_len = usb_serial_jtag_write_bytes(answer.data() + write_counter, answer.size() - write_counter, 1);
        write_counter += wr_len;
        vTaskDelay(1); // try to prevent WDT timemouts
    }
}
