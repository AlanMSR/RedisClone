#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

#include "utils.hpp"

int headerBytes = 4;

enum {
    TAG_NIL = 0,    // nil
    TAG_ERR = 1,    // error code + msg
    TAG_STR = 2,    // string
    TAG_INT = 3,    // int64
    TAG_DBL = 4,    // double
    TAG_ARR = 5,    // array
};

static int32_t read_full(int fd, uint8_t* buf, size_t n);
static int32_t write_all(int fd, uint8_t* buf, size_t n);
static int32_t send_req(int fd, const std::vector<std::string> &cmd);
static int32_t read_res(int fd);
static int32_t print_response(uint8_t*, size_t);

int main(int argc, char **argv) {
    int socketCFD = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int binfResult = connect(socketCFD, (const struct sockaddr*)&addr, sizeof(addr));
    if (binfResult)
        die("bind()");

    std::vector<std::string> cmd;
    for (int i = 1; i < argc; ++i) {
        cmd.push_back(argv[i]);
    }
    int32_t err = send_req(socketCFD, cmd);
    if (err) {
        close(socketCFD);
        return 1;
    }
    err = read_res(socketCFD);

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

static int32_t send_req(int fd, const std::vector<std::string> &cmd) {
    uint32_t len = 4;
    for (const std::string &s : cmd) {
        len += 4 + s.size();
    }

    if (len > k_max_msg) {
        msg("message too long");
        return -1;
    }

    // send request
    std::vector<uint8_t> wbuf(headerBytes + k_max_msg);
    memcpy(wbuf.data(), &len, headerBytes);
    uint32_t n = cmd.size();
    memcpy(wbuf.data() + headerBytes, &n, headerBytes);
    size_t cur = 8;
    for (const std::string &s : cmd) {
        uint32_t p = (uint32_t)s.size();
        memcpy(wbuf.data() + cur, &p, headerBytes);
        memcpy(wbuf.data() + cur + headerBytes, s.data(), s.size());
        cur += 4 + s.size();
    }
    return write_all(fd, wbuf.data(), 4 + len);

    return 0;
}

static int32_t print_response(uint8_t* data, size_t size) {
    if (size < 1) {
        msg("bad response");
        return -1;
    }

    switch (data[0]) {
    case TAG_NIL:
        printf("(nil)\n");
        return 1;
    case TAG_ERR:
        if (size < 1 + 8) {
            msg("bad response");
            return -1;
        }
        {
            int32_t code = 0;
            uint32_t len = 0;
            memcpy(&code, &data[1], 4);
            memcpy(&len, &data[1 + 4], 4);
            if (size < 1 + 8 + len) {
                msg("bad response");
                return -1;
            }
            printf("(err) %d %.*s\n", code, len, &data[1 + 8]);
            return 1 + 8 + len;
        }
    case TAG_STR:
        if (size < 1 + 4) {
            msg("bad response");
            return -1;
        }
        {
            uint32_t len = 0;
            memcpy(&len, &data[1], 4);
            if (size < 1 + 4 + len) {
                msg("bad response");
                return -1;
            }
            printf("(str) %.*s\n", len, &data[1 + 4]);
            return 1 + 4 + len;
        }
    case TAG_INT:
        if (size < 1 + 8) {
            msg("bad response");
            return -1;
        }
        {
            int64_t val = 0;
            memcpy(&val, &data[1], 8);
            printf("(int) %ld\n", val);
            return 1 + 8;
        }
    case TAG_DBL:
        if (size < 1 + 8) {
            msg("bad response");
            return -1;
        }
        {
            double val = 0;
            memcpy(&val, &data[1], 8);
            printf("(dbl) %g\n", val);
            return 1 + 8;
        }
    case TAG_ARR:
        if (size < 1 + 4) {
            msg("bad response");
            return -1;
        }
        {
            uint32_t len = 0;
            memcpy(&len, &data[1], 4);
            printf("(arr) len=%u\n", len);
            size_t arr_bytes = 1 + 4;
            for (uint32_t i = 0; i < len; ++i) {
                int32_t rv = print_response(&data[arr_bytes], size - arr_bytes);
                if (rv < 0) {
                    return rv;
                }
                arr_bytes += (size_t)rv;
            }
            printf("(arr) end\n");
            return (int32_t)arr_bytes;
        }
    default:
        msg("bad response");
        return -1;
    }
}

static int32_t read_res(int fd) {
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

    // memcpy(&rescode, &rbuf[4], 4);
    // printf("server says: [%u] %.*s\n", rescode, replyLen - 4, (const char*)(rbuf.data() + 8));    return 0;
    int32_t rv = print_response((uint8_t *)&rbuf[4], replyLen);
    if (rv > 0 && (uint32_t)rv != replyLen) {
        msg("bad response");
        rv = -1;
    }
    return rv;
}