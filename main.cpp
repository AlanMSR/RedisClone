#include <sys/socket.h>
#include <iostream>
// #include <cstring>
#include <cerrno>
#include <cstdlib>

void msg(const std::string &m);
void die(const std::string &msg);

static void something(int connSocket);

int main() {
    int socket = socket(AF_INET, SOCK_STREAM, 0);

    int val = 1;
    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(0)

    int bindResult = bind(socket, (const struct sockaddr*)&addr, sizeof(addr));

    if (bindResult)
        die("bind()");

    bindResult = listen(socket, SOMAXCONN);

    if (bindResult)
        die("listen()");

    while (true) {
        // accept
        struct sockaddr_in clientAddr = {};
        socklen_t addrLen = sizeof(clientAddr);
        int connSocket = accept(socket, (struct sockaddr*)&clientAddr, $addrLen);

        if (connSocket < 0)
            continue; //error

        close(connSocket);
    }

    return 0;
}

void msg(const std::string &m) {
    std::cerr << m << std::endl;
}

void die(const std::string &msg) {
    int err = errno;
    std::cerr << "[" << err << "] " << msg << ": " << strerror(err) << std::endl;
    abort();
}

static void something(int connSocket) {
    char rbuf[64] = {};
    ssize_t n = read(connSocket, rbuf, sizeof(rbuf) - 1);

    if (n < 0) {
        msg("read() error");
        return;
    }

    msg("client says: " + std::string(rbuf));

    char wbuf[] = "world";
    write(connSocket, wbuf, strlen(wbuf));
}