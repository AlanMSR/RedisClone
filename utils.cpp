#include "utils.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>

void msg(const std::string &m) {
    std::cerr << m << std::endl;
}

void die(const std::string &msg) {
    int err = errno;
    std::cerr << "[" << err << "] " << msg << ": " << strerror(err) << std::endl;
    abort();
}