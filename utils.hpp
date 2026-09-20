#pragma once
#include <string>
#include <cstdint>

const size_t k_max_msg = 32 << 20;
const uint64_t k_idle_timeout_ms = 5 * 1000;

void msg(const std::string &m);
void die(const std::string &msg);

#define container_of(ptr, type, member) ({                  \
    const __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type, member) );})

inline uint64_t str_hash(const uint8_t *data, size_t len) {
    uint32_t h = 0x811C9DC5;
    for (size_t i = 0; i < len; i++) {
        h = (h + data[i]) * 0x01000193;
    }
    return h;
}