#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "utils.hpp"
#include <cstring>

int main() {
    int socketC = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
     addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int binfResult = connect(socketC, (const struct sockaddr*)&addr, sizeof(addr));
    if (binfResult)
        die("bind()");

    std::string msgS = "hello";
    write(socketC, msgS.data(), msgS.size());

    std::string rrbuf(64, '\0');
    ssize_t n = read(socketC, rrbuf.data(), rrbuf.size() - 1);
    if (n < 0)
        die("read() error");
    
    rrbuf.resize(n);
    msg("server says: " + rrbuf);
    close(socketC);

    return 0;
}