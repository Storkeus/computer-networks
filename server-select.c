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

int main(int argc, char **argv)
{

    const char *const indexes[2] = {"138269", "55130"};
    const char *const studentData[2] = {"Bartosz Łyżwa", "Bartosz Nowak"};

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
        int i;
        for (i = fd + 1; i <= fdmax && fda > 0; i++)
        {

            char data[255];
            int responseSize;
            char *givenIndex;

            if (FD_ISSET(i, &rmask))
            {

                fda -= 1;
                responseSize = read(cfd, data, 255);
                givenIndex = (char *)malloc(responseSize);

                int j;
                for (j = 0; j < responseSize; j++)
                {
                    if (data[j] == '\r' || data[j] == '\n')
                    {
                        givenIndex[j] = '\0';
                    }
                    else
                    {
                        givenIndex[j] = data[j];
                    }
                }
                FD_CLR(i, &rmask);
                FD_SET(i, &wmask);
            }

            if (FD_ISSET(i, &wmask))
            {

                fda -= 1;

                int j;
                for (j = 0; j < 2; j++)
                {
                    if (strcmp(givenIndex, indexes[j]) == 0)
                    {
                        write(cfd, studentData[j], strlen(studentData[j]));
                    }
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