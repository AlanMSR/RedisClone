#pragma once
#include <string>
#include <cstdint>

const size_t k_max_msg = 32 << 20;

void msg(const std::string &m);
void die(const std::string &msg);