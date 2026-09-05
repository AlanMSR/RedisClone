#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <cassert>
#include <iostream>
#include "utils.hpp"


static int32_t read_full(int fd, uint8_t* buf, size_t n);
static int32_t write_all(int fd, uint8_t* buf, size_t n);
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

    std::vector<std::string> messagesS = {
        "Whos there?",
        "I am client",
        "youre the serber right?",
        "perfect!",
        "I am client!!!"
    };

    for (const auto& msgg : messagesS) {
        int32_t err = query(socketCFD, msgg.data());
        if (err) {
            close(socketCFD);
            return err;
        }
    }

    // int32_t err = query(socketCFD, "hello SONYAAA");
    // if (err) {
    //     close(socketCFD);
    //     return err;
    // }
    // err = query(socketCFD, "hello SOPHIAAAAAA");
    // if (err) {
    //     close(socketCFD);
    //     return err;
    // }
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

static int32_t query(int fd, const char* text) {
    int headerBytes = 4;
    uint32_t len = (uint32_t)strlen(text);
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

    std::vector<uint8_t> rbuf(headerBytes + k_max_msg);
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
    err = read_full(fd, &rbuf[headerBytes], replyLen);
    if (err) {
        msg("read() error");
        return err;
    }

    // std::cout << "server says: " << std::string(&rbuf[headerBytes], replyLen) << std::endl;

    return 0;
}