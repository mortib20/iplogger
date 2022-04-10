/* Libraries */
#include <stdlib.h>         // Standard library definitions
#include <stdio.h>          // Standard input/output
#include <string.h>         // String operations
#include <unistd.h>         // Standard symbolic constants and types
#include <sys/socket.h>     // Socket interface
#include <netinet/in.h>     // IP implementation
#include <netinet/tcp.h>    // TCP implementation
#include <arpa/inet.h>      // Conversion host <> network order
#include <netdb.h>          // Definitions for network database operations

/* Type definitons */
typedef struct {
    int socket;
    struct sockaddr_in address;
} IPLOG_SERV;

/* Function prototypes */
int server_init(IPLOG_SERV* iplogger, char* hostname, int port);
int server_bind(IPLOG_SERV* iplogger);
void server_accept(IPLOG_SERV* iplogger);

/* Main */
int main(int argc, char *argv[])
{
    size_t error;
    IPLOG_SERV iplog_serv;

    /* Initialize server */
    error = server_init(&iplog_serv, "0.0.0.0", 61103);
    if(error != 0)
        goto CLEANUP;

    /* Bind server to address and start listening */
    error = server_bind(&iplog_serv);
    if(error != 0)
        goto CLEANUP;

    while(1)
    {
        // Accept new client and write to log
        server_accept(&iplog_serv);
    }
    
    CLEANUP:
    close(iplog_serv.socket);

    return 0;
}

/*
    Initialize the IPLOG_SERV struct
*/
int server_init(IPLOG_SERV *iplogger, char *ip, int port)
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(inet_addr(ip));
    address.sin_port = htons(port);

    iplogger->socket = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    iplogger->address = address;

    if(iplogger->socket < 0)
    {
        printf("Error creating socket: %s\n", strerror(iplogger->socket));
        return -1;
    }

    return 0;
}

/*
    Bind socket to IP and start listening on port
*/
int server_bind(IPLOG_SERV *iplogger)
{
    size_t error;

    error = bind(iplogger->socket, (struct sockaddr*) &iplogger->address, sizeof(iplogger->address));
    if(error != 0)
    {
        printf("Error binding socket: %s\n", strerror(error));
        return -1;
    }
    
    error = listen(iplogger->socket, 10);
    if(error != 0)
    {
        printf("Error listen on socket: %s\n", strerror(error));
        return -1;
    }

    return 0;
}

/*
    Accept new client and log to stdout and 
*/
void server_accept(IPLOG_SERV* iplogger)
{
    char message[1000];
    size_t message_len;
    FILE *log_file;

    int client_socket = accept(iplogger->socket, NULL, NULL);

    message_len = read(client_socket, message, sizeof(message));
    message[message_len-1] = '\0'; // Remove \n in and change to \0
    
    printf("Received: %s\n", message);

    // Write to log file
    log_file = fopen("log.txt", "a+");
    fprintf(log_file, "%s\n", message);
    fflush(NULL);

    fclose(log_file);
    close(client_socket);
}