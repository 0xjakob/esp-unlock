#pragma once

#include <list>
#include <array>
#include <memory>
#include "command_registration.hpp"

class CommandListener {
public:
    static constexpr size_t BUF_SIZE = 256;

    CommandListener();
    ~CommandListener();

    void add_command_registration(std::shared_ptr<CommandRegistration>);

    void run();

private:
    std::list<std::shared_ptr<CommandRegistration> > registry;

    std::array<uint8_t, BUF_SIZE> serial_buf;
};
