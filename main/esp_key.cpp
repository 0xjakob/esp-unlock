#include <cstdio>
#include <string>
#include <cstring>
#include <thread>
#include "esp_hmac.h"
#include "gpio_cxx.hpp"
#include "command_listener.hpp"
#include "hash_command.hpp"
#include "totp_command.hpp"
#include "totp_command_add.hpp"
#include "totp_command_list.hpp"
#include "driver/usb_serial_jtag.h"
#include "system_cxx.hpp"
#include "otp_storage.hpp"

using namespace std;
using namespace idf;

extern "C" void app_main(void)
{
    const chrono::milliseconds BLINK_TIME(200);
    usb_serial_jtag_driver_config_t usb_serial_jtag_config;
    usb_serial_jtag_config.tx_buffer_size = 256;
    usb_serial_jtag_config.rx_buffer_size = 256;
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_config));

    CommandListener cmd_listener;
    auto otp_storage = make_shared<OTP_Storage>("totp");
//    cmd_listener.add_command_registration(make_shared<HashRequest>());
    cmd_listener.add_command_registration(make_shared<TOTP_Request>(otp_storage));
    cmd_listener.add_command_registration(make_shared<TOTP_RequestAdd>(otp_storage));
    cmd_listener.add_command_registration(make_shared<TOTP_RequestList>(otp_storage));

    GPIO_OpenDrain gpio_blue(GPIONum(1));
    GPIO_OpenDrain gpio_red(GPIONum(0));

    gpio_red.set_floating();
    gpio_blue.set_low();
    this_thread::sleep_for(BLINK_TIME);
    gpio_blue.set_floating();
    gpio_red.set_low();
    this_thread::sleep_for(BLINK_TIME);
    gpio_red.set_floating();
    gpio_blue.set_low();

    while (true) {
        cmd_listener.run();
    }
}
