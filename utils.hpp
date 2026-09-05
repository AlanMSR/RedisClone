#pragma once
#include <string>
#include <cstdint>

struct Buffer {
    uint8_t* buffer_begin;
    uint8_t* buffer_end;
    uint8_t* data_begin;
    uint8_t* data_end;
};

const size_t k_max_msg = 32 << 20;

void msg(const std::string &m);
void die(const std::string &msg);