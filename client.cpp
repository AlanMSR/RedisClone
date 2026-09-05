#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <vector>
#include <cassert>
#include <iostream>
#include "utils.hpp"

int headerBytes = 4;

static int32_t read_full(int fd, uint8_t* buf, size_t n);
static int32_t write_all(int fd, uint8_t* buf, size_t n);
static int32_t send_req(int fd, const uint8_t* text, size_t len);
static int32_t read_res(int fd, uint8_t* buf, size_t n);
int main() {
    int socketCFD = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
     addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int binfResult = connect(socketCFD, (const struct sockaddr*)&addr, sizeof(addr));
    if (binfResult)
        die("bind()");

    std::vector<std::string> messagesS = {
        "Whos there?",
        "I am client",
        "youre the serber right?",
        "perfect!",
        "I am client!!!"
    };

    // PIPELINE: send requests and read responses
    // send requests
    for (const auto& msgg : messagesS) {
        int32_t err = send_req(socketCFD, (const uint8_t*)msgg.data(), msgg.size());
        if (err) {
            die("send_req() failed");
        }
    }

    // read responses
    for (size_t i = 0; i < messagesS.size(); ++i) {
        int32_t err = read_res(socketCFD, nullptr, 0);
        if (err) {
            die("read_res() failed");
        }
    }

    return 0;
}

static int32_t read_full(int fd, uint8_t* buf, size_t n) {
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

static int32_t write_all(int fd, uint8_t* buf, size_t n) {
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

static int32_t send_req(int fd, const uint8_t* text, size_t len) {
    if (len > k_max_msg) {
        msg("message too long");
        return -1;
    }

    // send request
    std::vector<uint8_t> wbuf(headerBytes + k_max_msg);
    memcpy(wbuf.data(), &len, headerBytes);
    memcpy(&wbuf[headerBytes], text, len);
    if (int32_t err = write_all(fd, wbuf.data(), headerBytes + len))
        return err;

    return 0;
}

static int32_t read_res(int fd, uint8_t* buf, size_t n) {
    std::vector<uint8_t> rbuf(headerBytes);
    errno = 0;
    int32_t err = read_full(fd, rbuf.data(), headerBytes);
    if (err) {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    uint32_t replyLen = 0;
    memcpy(&replyLen, rbuf.data(), headerBytes);
    if (replyLen > k_max_msg) {
        msg("message too long");
        return -1;
    }

    // reply body
    rbuf.resize(headerBytes + replyLen);
    err = read_full(fd, &rbuf[headerBytes], replyLen);
    if (err) {
        msg("read() error");
        return err;
    }

    printf("len:%u data:%.*s\n", replyLen, (int)replyLen, &rbuf[headerBytes]);

    return 0;
}