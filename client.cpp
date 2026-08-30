#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cassert>
#include <iostream>
#include "utils.hpp"


static int32_t read_full(int fd, char* buf, size_t n);
static int32_t write_all(int fd, const char* buf, size_t n);
static int32_t query(int fd, const char* text);

int main() {
    int socketCFD = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
     addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int binfResult = connect(socketCFD, (const struct sockaddr*)&addr, sizeof(addr));
    if (binfResult)
        die("bind()");

    int32_t err = query(socketCFD, "hello SONYAAA");
    if (err) {
        close(socketCFD);
        return err;
    }
    err = query(socketCFD, "hello SOPHIAAAAAA");
    if (err) {
        close(socketCFD);
        return err;
    }
    return 0;
}

static int32_t read_full(int fd, char* buf, size_t n) {
    while (n > 0) {
        ssize_t rev = read(fd, buf, n);
        if(rev <= 0)
            return -1;
        
        assert((size_t)rev <= n);
        n -= (size_t)rev;
        buf += rev;
    }
    return 0;
}

static int32_t write_all(int fd, const char* buf, size_t n) {
    while (n > 0) {
        ssize_t rev = write(fd, buf, n);
        if(rev <= 0)
            return -1;

        assert((size_t)rev <= n);
        n -= (size_t)rev;
        buf += rev;

    }
    return 0;
}

static int32_t query(int fd, const char* text) {
    int headerBytes = 4;
    uint32_t len = (uint32_t)strlen(text);
    if (len > k_max_msg) {
        msg("message too long");
        return -1;
    }

    // send request
    char wbuf[headerBytes + k_max_msg];
    memcpy(wbuf, &len, headerBytes);
    memcpy(&wbuf[4], text, len);
    if (int32_t err = write_all(fd, wbuf, headerBytes + len))
        return err;

    char rbuf[headerBytes + k_max_msg];
    errno = 0;
    int32_t err = read_full(fd, rbuf, headerBytes);
    if (err) {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    memcpy(&len, rbuf, headerBytes);
    if (len > k_max_msg) {
        msg("message too long");
        return -1;
    }

    // reply body
    err = read_full(fd, &rbuf[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    std::cout << "server says: " << std::string(&rbuf[4], len) << std::endl;
    return 0;
}