#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cassert>
#include <cstring>
#include <cstdint>

#include "utils.hpp"

// static void something(int connSocket);
static int32_t read_full(int fd, char* buf, size_t n);
static int32_t write_all(int fd, const char* buf, size_t n);
static size_t one_request(int connFd);

// server
int main() {
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);

    int val = 1;
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(0);

    int bindResult = bind(socketFD, (const struct sockaddr*)&addr, sizeof(addr));

    if (bindResult)
        die("bind()");

    bindResult = listen(socketFD, SOMAXCONN);

    if (bindResult)
        die("listen()");

    while (true) {
        // accept
        struct sockaddr_in clientAddr = {};
        socklen_t addrLen = sizeof(clientAddr);
        int connFD = accept(socketFD, (struct sockaddr*)&clientAddr, &addrLen);

        if (connFD < 0)
            continue; //error

        while (true) {
            int32_t err = one_request(connFD);
            if (err) {
                break;
            }
        }

        close(connFD);
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

static size_t one_request(int connFd) {
    // 4 bytes header
    int headerBytes = 4;
    char rbuf[headerBytes + k_max_msg];
    errno = 0;
    
    int32_t err = read_full(connFd, rbuf, headerBytes);
    if (err) {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, headerBytes);
    if (len > k_max_msg) {
        msg("message too long");
        return -1;
    }

    // request body
    err = read_full(connFd, &rbuf[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    // do something
    std::cout << "client says: " << std::string(&rbuf[4], len) << std::endl;

    // reply using the same protocol
    std::string reply = "DO NOT DISTURB!";
    char wbuf[headerBytes + reply.size()];
    len = (uint32_t)reply.size();
    memcpy(wbuf, &len, headerBytes);
    memcpy(&wbuf[4], reply.data(), reply.size());

    return write_all(connFd, wbuf, headerBytes + reply.size());
}

// static void something(int connSocket) {
//     char rbuf[64] = {};
//     ssize_t n = read(connSocket, rbuf, sizeof(rbuf) - 1);

//     if (n < 0) {
//         msg("read() error");
//         return;
//     }

//     msg("client says: " + std::string(rbuf));

//     std::string wbuf = "hello";
//     write(connSocket, wbuf.data(), wbuf.size());
// }