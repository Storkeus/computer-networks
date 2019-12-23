//compile with -lthread
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


    const char *const indexes[2] = {"138269", "55130"};
    const char *const studentData[2] = {"Bartosz Łyżwa", "Bartosz Nowak"};

struct cln
{
    int cfd;
    struct sockaddr_in caddr;
};

void * cthread(void* arg)
{
struct cln* c=(struct cln*)arg;

        printf("new connection from: %s:%d\n",
        inet_ntoa(c->caddr.sin_addr), c->caddr.sin_port);
        char data[255];
        int responseSize = read(c->cfd, data, 255);

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
                write(c->cfd, studentData[i], strlen(studentData[i]));
            }
        }

        close(c->cfd);
        free(c);
        return 0;
}

int main(int argc, char **argv)
{

    int fd = socket(PF_INET, SOCK_STREAM, 0);

    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(1234); // zabezpieczenie przed różnicami w kodowaniu liczb binarnych Little Endian, Big Endian
    saddr.sin_addr.s_addr = INADDR_ANY;

    bind(fd, (struct sockaddr *)&saddr, sizeof(saddr));
    listen(fd, 10);



    while (1)
    {
        pthread_t tid;
        struct cln* c= malloc(sizeof(struct cln));
        socklen_t l = sizeof(c->caddr);
        c->cfd = accept(fd, (struct sockaddr *)&c->caddr, &l);
        pthread_create(&tid,NULL,cthread,c);
        pthread_detach(tid);
    }
    close(fd);

    return 0;
}