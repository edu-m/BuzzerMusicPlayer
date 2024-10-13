#pragma once

#include <cstdint>
#include <string>

class Speaker {
public:
    Speaker();
    ~Speaker();

    void sendTone(int tone);
    void stop();

private:
    int fd_;
    struct timeval tv_;
    struct EventVal {
        struct timeval t_val;
        uint16_t type;
        uint16_t code;
        uint32_t value;
    } ev_;
};
