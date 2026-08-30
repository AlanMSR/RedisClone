#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include "utils.hpp"

static void something(int connSocket);

int main() {
    int socketV = socket(AF_INET, SOCK_STREAM, 0);

    int val = 1;
    setsockopt(socketV, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(0);

    int bindResult = bind(socketV, (const struct sockaddr*)&addr, sizeof(addr));

    if (bindResult)
        die("bind()");

    bindResult = listen(socketV, SOMAXCONN);

    if (bindResult)
        die("listen()");

    while (true) {
        // accept
        struct sockaddr_in clientAddr = {};
        socklen_t addrLen = sizeof(clientAddr);
        int connSocket = accept(socketV, (struct sockaddr*)&clientAddr, &addrLen);

        if (connSocket < 0)
            continue; //error

        something(connSocket);
        close(connSocket);
    }

    return 0;
}

static void something(int connSocket) {
    char rbuf[64] = {};
    ssize_t n = read(connSocket, rbuf, sizeof(rbuf) - 1);

    if (n < 0) {
        msg("read() error");
        return;
    }

    msg("client says: " + std::string(rbuf));

    std::string wbuf = "hello";
    write(connSocket, wbuf.data(), wbuf.size());
}