#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DEFAULT_IP INADDR_ANY
#define DEFAULT_PORT 61103
#define MAX_CONNECTIONS 10
#define BUFFER_SIZE 500

/*
1. socket
2. bind
3. listen
4. accept
*/



int main(int argc, char* argv[])
{
    size_t error;

    if(argc > 1 && strcmp(argv[1], "-h") == 0)
    { 
        printf("Usage: %s -h [ip] [port]\n", argv[0]); 
        exit(EXIT_SUCCESS); 
    }

    FILE *log;
    log = fopen("log.txt", "a+");
    if(log == NULL)
    {
        printf("Failed to open logfile: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket <= 0)
    {
        printf("Error creating server socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    
    if(argc > 1)
        server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    else
        server_addr.sin_addr.s_addr = DEFAULT_IP;
    
    if(argc > 2)
        server_addr.sin_port = htons(atoi(argv[2]));
    else
        server_addr.sin_port = htons(DEFAULT_PORT);

    error = bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));
    if(error == -1)
    {
        printf("Error binding socket to port: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    error = listen(server_socket, MAX_CONNECTIONS);
    if(error == -1)
    {
        printf("Error starting listen on socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Server running on %s:%i.\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    while(1)
    {
        char request[BUFFER_SIZE];
        // Maybe put in struct 
        socklen_t client_len;
        struct sockaddr client_addr;
        //
        int client_socket = accept(server_socket, &client_addr, &client_len);
        if(client_socket == -1)
        {
            printf("Error accept new client: %s\n", strerror(errno));
        }

        int size = read(client_socket, request, sizeof(request));
        memset((request + size - 1), '\0', sizeof(request)); size--; // Replace on size \n -> \0 and decrease size by 1

        printf("\nReceived(%i): %s", size, request);
        fprintf(log, "%s\n", request);
        fflush(NULL);

        close(client_socket);
    }

    // Cleanup
    fclose(log);
    close(server_socket);

    return 0;
}
