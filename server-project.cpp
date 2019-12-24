#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#include <iostream>
#include <string>
#include "lib/struct/User.cpp"
#include "lib/class/Message.cpp"

int writeAll(int fd, const char *buf, size_t n)
{
    unsigned messageSize;
    for (messageSize = 0; messageSize < n; messageSize += write(fd, buf, n))
        ;
    return messageSize;
}

int readAll(int fd, void *buf, size_t n)
{

    unsigned responseSize = 0;

    do
    {
        responseSize += read(fd, buf, n);
    } while (((char *)buf)[responseSize] != '\0');

    return responseSize;
}

int main(int argc, char **argv)
{

    const User users[2] = {{"bartek", "password"}, {"karolina", "123456789"}};

    int fd = socket(PF_INET, SOCK_STREAM, 0);

    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

    struct sockaddr_in saddr, caddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(1234); // zabezpieczenie przed różnicami w kodowaniu liczb binarnych Little Endian, Big Endian
    saddr.sin_addr.s_addr = INADDR_ANY;

    bind(fd, (struct sockaddr *)&saddr, sizeof(saddr));
    listen(fd, 10);

    fd_set rmask;
    FD_ZERO(&rmask);

    fd_set wmask;
    FD_ZERO(&wmask);

    int fdmax = fd;

    while (1)
    {

        FD_SET(fd, &rmask);

        static struct timeval timeout;
        timeout.tv_sec = 5 * 60;
        timeout.tv_usec = 0;
        int rc = select(fdmax + 1, &rmask, &wmask, (fd_set *)0, &timeout);
        if (rc == 0)
        {
            printf("timed out\n");
            continue;
        }

        int fda = rc;
        socklen_t l;
        int cfd;
        if (FD_ISSET(fd, &rmask))
        {
            fda -= 1;
            l = sizeof(caddr);
            cfd = accept(fd, (struct sockaddr *)&caddr, &l);
            printf("new connection from: %s:%d\n",
                   inet_ntoa(caddr.sin_addr), caddr.sin_port);
            FD_SET(cfd, &rmask);

            if (cfd > fdmax)
            {
                fdmax = cfd;
            }
        }
        unsigned bufforSize = 2000;
        for (int i = fd + 1; i <= fdmax && fda > 0; i++)
        {

            char data[bufforSize];
            for (unsigned j = 0; j < bufforSize; j++)
            {
                data[j] = '\0';
            }
            int responseSize;
            Message message;

            // Reading (reciving) data
            if (FD_ISSET(i, &rmask))
            {

                fda -= 1;
                std::cout << "Test0" << std::endl
                          << std::flush;

                responseSize = readAll(cfd, data, bufforSize);

                std::cout << "Test1" << std::endl
                          << std::flush;
                std::string loadedData = "";
                std::cout << "Test2" << std::endl;
                for (int j = 0; j < responseSize; j++)
                {

                    loadedData += data[j];
                    if (data[j] == '\n' || j == responseSize - 1) //Nowa linia lub koniec wiadomości
                    {
                        message.addData(loadedData);
                        loadedData = "";
                    }
                }

                FD_CLR(i, &rmask);
                FD_SET(i, &wmask);
            }

            //Writing (sending) data
            if (FD_ISSET(i, &wmask))
            {

                fda -= 1;

                if (message.isAuthorized(users,2))
                {
                    writeAll(cfd, message.content.c_str(), message.content.length() + 1);
                }
                else
                {
                    const char* errorMessage="Nieprawidłowy login lub hasło";
                    writeAll(cfd,  errorMessage, strlen(errorMessage));
                }

                close(i);
                FD_CLR(i, &wmask);

                if (i == fdmax)
                {
                    while (fdmax > fd && !FD_ISSET(fdmax, &wmask))
                    {
                        fdmax -= 1;
                    }
                }
            }
        }
    }
    close(fd);

    return 0;
}