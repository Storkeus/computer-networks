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
#include "lib/struct/ResponseMessage.cpp"
#include "lib/class/Message.cpp"
#include "lib/class/MessageList.cpp"
#include <ctime>

enum ConnectionType
{
    SEND_HOME_SERVER,
    SEND_RECIVER_SERVER,
    GET_INBOX,
    GET_OUTBOX
};

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
    srand(time(0));
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
            MessageList messageList;
            //std::cout << "Inicjalizacja Klasy Message List" << std::endl<< std::flush;
            ConnectionType connectionType;
            // Reading (reciving) data
            if (FD_ISSET(i, &rmask))
            {

                fda -= 1;

                responseSize = readAll(cfd, data, bufforSize);

                std::string loadedData = "";

                connectionType = (ConnectionType)(data[0] - '0');
                for (int j = 0; j < responseSize; j++)
                {
                    loadedData += data[j];
                    if (data[j] == '\n' || j == responseSize - 1) //Nowa linia lub koniec wiadomości
                    {
                        switch (connectionType)
                        {
                        case SEND_HOME_SERVER:
                        case SEND_RECIVER_SERVER:
                        {
                             //std::cout<<"Zapis w "<<SEND_RECIVER_SERVER<<std::endl<<std::flush;
                            message.addData(loadedData);
                            break;
                        }
                        case GET_INBOX:
                        case GET_OUTBOX:
                        {
                            messageList.addData(loadedData);
                            break;
                        }
                        }

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
                switch (connectionType)
                {
                case SEND_HOME_SERVER:
                {
                    if (message.isAuthorized(users, 2))
                    {
                        message.saveOut(); //zapis w folderze "wysłane"
                        message.send();    //wysłanie wiadomości na serwer docelowy
                        const char *successMessage = "Wysłano wiadomość";
                        writeAll(cfd, successMessage, strlen(successMessage));
                    }
                    else
                    {
                        const char *errorMessage = "Nieprawidłowy login lub hasło";
                        writeAll(cfd, errorMessage, strlen(errorMessage));
                    }
                    break;
                }
                case SEND_RECIVER_SERVER:
                {

                    message.saveIn(); //zapis w folderze "odebrane"
                    break;
                }
                case GET_INBOX:
                case GET_OUTBOX:
                {
                    if (messageList.isAuthorized(users, 2))
                    {
                        std::string list = messageList.get();
                        writeAll(cfd, list.c_str(), list.length());
                    }
                    else
                    {
                        const char *errorMessage = "Nieprawidłowy login lub hasło";
                        writeAll(cfd, errorMessage, strlen(errorMessage));
                    }

                    break;
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