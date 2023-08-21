# ESP-Unlock Software

This application for an ESP32-C3 calculates One-Time Passwords (OTP) on request. It implements the [OATH-TOTP](https://www.ietf.org/rfc/rfc6238.txt) protocol using SHA1 and a six digit code.

# Requirements

You need the [ESP-IDF](https://github.com/espressif/esp-idf) version 5.1 and [ESP-IDF-C++](https://github.com/espressif/esp-idf-cxx) version 1.0.1-beta (set automatically for ESP-IDF-C++). The required versions may change in the future. For more information on setting up ESP-IDF's requirements, visit the [Get Started page of the ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/release-v5.1/esp32c3/get-started/index.html).

# Build

After setting up and exporting the ESP-IDF toolchain, run:
```
idf.py build
```
Note that an internet connection is needed for the component manager while building.

# Run

This application is meant to be run on an ESP32-C3 and communicating via its USB-serial interface. But in principle, any other serial connection can be used, too. Use menuconfig and go to *Component config → ESP System Settings → Channel for console output* to change the serial connection. If you chose the default UART, you should be able to use any ESP-series development board.
