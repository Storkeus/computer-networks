#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUF_SIZE 1024

int main(int argc, char* argv[])
{
        WORD WRequiredVersion;
        WSADATA WData;
        SOCKET SSocket;
        int nConnect;
        int nBytes;
        struct sockaddr_in stServerAddr;
        struct hostent* lpstServerEnt;
        char cbBuf[BUF_SIZE];

        if (argc != 4)
        {
                fprintf(stderr, "Usage: %s server_name port_number\n", argv[0]);
                exit(1);
        }

        /* initialize winsock */
        WRequiredVersion = MAKEWORD(2, 0);
        if (WSAStartup(WRequiredVersion, &WData) != 0){
                fprintf(stderr, "WSAStartup failed!");
                exit(1);
        }

        /* look up server's IP address */
        lpstServerEnt = gethostbyname(argv[1]);
        if (! lpstServerEnt)
        {
                fprintf(stderr, "%s: Can't get the server's IP address.\n", argv[0]);
                exit(1);
        }

        /* create a socket */
        SSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        /* server info */
        memset(&stServerAddr, 0, sizeof(struct sockaddr));
        stServerAddr.sin_family = AF_INET;
        memcpy(&stServerAddr.sin_addr.s_addr, lpstServerEnt->h_addr, lpstServerEnt->h_length);
        stServerAddr.sin_port = htons(atoi(argv[2]));

        /* connect to the server */
        nConnect = connect(SSocket, (struct sockaddr*)&stServerAddr, sizeof(struct sockaddr));
        if (nConnect < 0)
        {
                fprintf(stderr, "%s: Can't connect to the server (%s:%i).\n",
                        argv[0], argv[1], atoi(argv[2]));
                exit(1);
        }

        send(SSocket, cbBuf, sizeof(cbBuf), 0);

        /* connection is now established; read from socket */
        nBytes = recv(SSocket, cbBuf, sizeof(cbBuf), 0);

        cbBuf[nBytes] = '\x0';

        printf("Data from SERVER [%s]:\t%s", argv[1], cbBuf);

        closesocket(SSocket);

        /* terminate use of the winsock */
        WSACleanup();

        return 0;
}
