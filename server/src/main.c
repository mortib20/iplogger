#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <errno.h>
#include <signal.h>

#define PORT 61103

/*
1. socket
2. bind
3. listen
4. accept
*/

void INThandler(int);

int main(void)
{
    struct sockaddr_in server_addr =
    {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(PORT)
    };

    // 1. Socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock == -1)
    {
        printf("socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(server_sock, F_GETFL);
    fcntl(server_sock, F_SETFL, flags | O_NONBLOCK);

    // 2. Bind to address and port
    if(bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
    {
        printf("bind: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(listen(server_sock, 10) == -1)
    {
        printf("listen: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Server running on %i and any address.\n", PORT);

    for(;;)
    {
        int client_sock = accept(server_sock, NULL, NULL);

        signal(SIGINT, INThandler);
        if(client_sock == -1)
        {
            //printf("No connection:%s(%i)\n", strerror(errno), errno);
            sleep(1);
        } else {
            char message[100];
            int size_message = read(client_sock, message, sizeof(message));
            message[size_message - 1] = '\0';
            printf("Received: %s\n", message);
            close(client_sock);
        }
    }

    return 0;
}

void INThandler(int sig)
{
    signal(sig, SIG_IGN);
    
    exit(0);
}