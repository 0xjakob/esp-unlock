#include <stdio.h>
#include <memory.h>
#include "unity.h"
#include "command_registration.hpp"
#include "command_listener.hpp"

extern "C" {
#include "Mockusb_serial_jtag.h"
}

struct TestRegistration : public CommandRegistration {
    const std::vector<uint8_t> LISTEN_TOKEN = {'T', 'E', 'S', 'T',':'};
    const std::vector<uint8_t>& get_listen_token() const override {
        return LISTEN_TOKEN;
    }

    std::vector<uint8_t> process_content(const std::vector<uint8_t>& content) override {
        saved_content = content;
        return std::vector<uint8_t>{'T', 'E', 'S', 'T', ':', 'O', 'K'};
    }

    std::vector<uint8_t> saved_content;
};

TEST_CASE("basic test", "[CommandListener]")
{
    const std::string test_command = "TEST:bla\n"; // const char would produce wrong sizeof(test_command)
    std::vector<uint8_t> expected_content = {'b', 'l', 'a'};
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(9);
    usb_serial_jtag_read_bytes_ReturnMemThruPtr_buf(const_cast<void*>(static_cast<const void*>(test_command.c_str())),
            test_command.size());
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(0);
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(0);
    usb_serial_jtag_write_bytes_ExpectAnyArgsAndReturn(8); // "TEST:OK\n"
    CommandListener listener;
    std::shared_ptr<TestRegistration> test_reg = std::make_shared<TestRegistration>();
    listener.add_command_registration(test_reg);

    listener.run();

    TEST_ASSERT(test_reg->saved_content == expected_content);
}

TEST_CASE("basic test scattered command", "[CommandListener]")
{
    const std::string test_command_first = "TE";
    const std::string test_command_second = "ST:bla\n";
    std::vector<uint8_t> expected_content = {'b', 'l', 'a'};
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(2);
    usb_serial_jtag_read_bytes_ReturnMemThruPtr_buf(
            const_cast<void*>(static_cast<const void*>(test_command_first.c_str())),
            test_command_first.size());
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(7);
    usb_serial_jtag_read_bytes_ReturnMemThruPtr_buf(
            const_cast<void*>(static_cast<const void*>(test_command_second.c_str())),
            test_command_second.size());
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(0);
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(0);
    usb_serial_jtag_write_bytes_ExpectAnyArgsAndReturn(8);
    CommandListener listener;
    std::shared_ptr<TestRegistration> test_reg = std::make_shared<TestRegistration>();
    listener.add_command_registration(test_reg);

    listener.run();

    TEST_ASSERT(test_reg->saved_content == expected_content);
}

TEST_CASE("result is written to serial", "[CommandListener]")
{
    const std::string test_command = "TEST:bla\n";
    constexpr std::array<uint8_t, 8> expected_write = {'T', 'E', 'S', 'T', ':', 'O', 'K', '\n'};
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(9);
    usb_serial_jtag_read_bytes_ReturnMemThruPtr_buf(const_cast<void*>(static_cast<const void*>(test_command.c_str())),
            test_command.size());
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(0);
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(0);
    usb_serial_jtag_write_bytes_ExpectWithArrayAndReturn(expected_write.data(),
            expected_write.size(),  // comparison depth for Cmock
            expected_write.size(),  // expected size from
            1,                      // ticks to wait
            expected_write.size()); // return value
    CommandListener listener;
    std::shared_ptr<TestRegistration> test_reg = std::make_shared<TestRegistration>();
    listener.add_command_registration(test_reg);

    listener.run();
}

TEST_CASE("result is written to serial scattered", "[CommandListener]")
{
    const std::string test_command = "TEST:bla\n";
    constexpr std::array<uint8_t, 8> expected_write_1 = {'T', 'E', 'S', 'T', ':', 'O', 'K', '\n'};
    constexpr std::array<uint8_t, 1> expected_write_2 = {'\n'};
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(9);
    usb_serial_jtag_read_bytes_ReturnMemThruPtr_buf(const_cast<void*>(static_cast<const void*>(test_command.c_str())),
            test_command.size());
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(0);
    usb_serial_jtag_read_bytes_ExpectAnyArgsAndReturn(0);
    usb_serial_jtag_write_bytes_ExpectWithArrayAndReturn(expected_write_1.data(),
            expected_write_1.size(),      // comparison depth for Cmock
            expected_write_1.size(),      // expected size from
            1,                            // ticks to wait
            expected_write_1.size() - 1); // return value
    usb_serial_jtag_write_bytes_ExpectWithArrayAndReturn(expected_write_2.data(),
            expected_write_2.size(),    // comparison depth for Cmock
            expected_write_2.size(),    // expected size from
            1,                          // ticks to wait
            1);                         // return value
    CommandListener listener;
    std::shared_ptr<TestRegistration> test_reg = std::make_shared<TestRegistration>();
    listener.add_command_registration(test_reg);

    listener.run();
}

extern "C" void app_main()
{
    unity_run_menu();
}
