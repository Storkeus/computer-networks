#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>

int writeAll(int fd, const char *buf, size_t n)
{
    unsigned messageSize;

    for (messageSize = 0; messageSize < n; messageSize += write(fd, buf, n));

    return messageSize;
}


int main(int argc, char **argv)
{

    int fd = socket(PF_INET, SOCK_STREAM, 0);

    struct hostent *host;
    host = gethostbyname(argv[1]);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));                                // zabezpieczenie przed różnicami w kodowaniu liczb binarnych Little Endian, Big Endian
    memcpy(&addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length); //było h_addr zamiast h_addr_list -> to makro dla starego sprzętu
    //addr.sin_addr.s_addr=inet_addr(); //Serwer datetime

    connect(fd, (struct sockaddr *)&addr, sizeof(addr));

const char* message="0\nbartek\npassword\nkarolina@127.0.0.1\nTest\nLoremIpsum\nABC";
    write(fd, message, strlen(message));
    char data[255];
    int responseSize = read(fd, data, 255);
    write(1, data, responseSize);
    write(1, "\n", 1);

    close(fd);
}