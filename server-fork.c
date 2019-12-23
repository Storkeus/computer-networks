#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void childend(int signo)
{
    printf("%s\n","Proces usunięty");
    wait(NULL); 
}

int main(int argc, char **argv)
{

    const char *const indexes[2] = {"138269", "55130"};
    const char *const studentData[2] = {"Bartosz Łyżwa", "Bartosz Nowak"};

    signal(SIGCHLD,childend);
    int fd = socket(PF_INET, SOCK_STREAM, 0);

    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

    struct sockaddr_in saddr, caddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(1234); // zabezpieczenie przed różnicami w kodowaniu liczb binarnych Little Endian, Big Endian
    saddr.sin_addr.s_addr = INADDR_ANY;

    bind(fd, (struct sockaddr *)&saddr, sizeof(saddr));
    listen(fd, 10);

    while (1)
    {
        socklen_t l = sizeof(caddr);
        int cfd = accept(fd, (struct sockaddr *)&caddr, &l);
        if(!fork())
        {
            close(fd);
        
        printf("new connection from: %s:%d\n",
               inet_ntoa(caddr.sin_addr), caddr.sin_port);
        char data[255];
        int responseSize = read(cfd, data, 255);

        char *givenIndex = (char *)malloc(responseSize);

        for (int i = 0; i < responseSize; i++)
        {
            if (data[i] == '\r' || data[i] == '\n')
            {
                givenIndex[i] = '\0';
            }
            else
            {
                givenIndex[i] = data[i];
            }
        }

        for (int i = 0; i < 2; i++)
        {
            if (strcmp(givenIndex, indexes[i]) == 0)
            {
                write(cfd, studentData[i], strlen(studentData[i]));
            }
        }

        close(cfd);
        exit(0);
        }
        else
        {
            close(cfd);
        }
    }

    return 0;
}