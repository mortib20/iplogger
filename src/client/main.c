#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// add parameter support
// remove \n in ip

#define DEBUG 0

#define BUFFER_SIZE 1024

#define SERVER_HOST "edward.martpaul.de"
#define SERVER_PORT "61103"

struct addrinfo* getServerAddress();
char* getIPAdress();
char* getHostname();
time_t getTimestamp();
struct hostinformation getHostinfo();

struct hostinformation {
    char* wan_ip;
    char* hostname;
    time_t unix_time;
};

int main(void)
{
    size_t error, request_length;
    struct addrinfo *server = getServerAddress();
    struct hostinformation hostinfo = getHostinfo();
    char request[BUFFER_SIZE];

    request_length = sprintf(request, "%s-%ld-%s", hostinfo.hostname, hostinfo.unix_time, hostinfo.wan_ip);

    printf("%s\n", request);

    /*struct sockaddr_in iplog_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr("192.168.2.106"),
        .sin_port = htons(SERVER_PORT)
    };*/

    int iplog_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(iplog_socket <= 0)
    {
        printf("Error creating socket to iplog server: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //error = connect(iplog_socket, (struct sockaddr*)&iplog_addr, sizeof(iplog_addr));
    error = connect(iplog_socket, server->ai_addr, server->ai_addrlen);
    if(error < 0)
    {
        printf("Error connecting so iplog server: %s\n", strerror(errno));
    }

    error = write(iplog_socket, request, request_length);
    if(error <= 0)
    {
        printf("Error sending request: %s\n", strerror(errno));
    }

    error = shutdown(iplog_socket, SHUT_RDWR);
    if(error < 0)
    {
        printf("Error shutdown server: %s\n", strerror(errno));
    }

    close(iplog_socket);

    return 0;
}

struct addrinfo* getServerAddress()
{
    struct addrinfo hints, *result;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    getaddrinfo(SERVER_HOST, SERVER_PORT, &hints, &result);

    return result;
}

char* getIPAdress()
{
    /* Variables */
    const char *request;
    char response[BUFFER_SIZE];
    static char ip[16];

    ssize_t error, response_length;
    struct addrinfo hints;
    struct addrinfo *result;
    int ifco_socket;

    memset(&hints, 0, sizeof(hints));

    /* Inizialization */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;

    request = 
    "GET /ip HTTP/1.1\n"
    "Host: ifconfig.co\n"
    "User-Agent: ipl_client/1.0\n"
    "Accept: */*\n"
    "Connection: close\n"
    "\n";
    
    error = getaddrinfo("ifconfig.co", "80", &hints, &result);
    if(error != 0)
    {
        printf("Error receiving socket address for ifconfig.co: %s\n", gai_strerror(error));
        freeaddrinfo(result);
        return NULL;
    }

    ifco_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(ifco_socket < 0)
    {
        printf("Error opening ifco_socket: %s\n", strerror(errno));
        close(ifco_socket);
        freeaddrinfo(result);
        return NULL;
    }

    error = connect(ifco_socket, result->ai_addr, result->ai_addrlen);
    if(error != 0)
    {
        printf("Error connecting to ifconfig.co: %s\n", strerror(errno));
        close(ifco_socket);
        freeaddrinfo(result);
        return NULL;
    }

    error = write(ifco_socket, request, strlen(request));
    if(error < 0)
    {
        printf("Error sending header to ifconfig.co: %s\n", strerror(error));
        close(ifco_socket);
        freeaddrinfo(result);
        return NULL;
    }

    #if DEBUG > 0
        printf("[DEBUG] Written %zi bytes to ifconfig.co\n", error);
    #endif

    response_length = read(ifco_socket, response, sizeof(response));
    if(response_length <= 0)
    {
        printf("Error reading from ifconfig.co: %s\n", strerror(errno));
        close(ifco_socket);
        freeaddrinfo(result);
        return NULL;
    }

    #if DEBUG > 0
        printf("[DEBUG] Received %zi bytes from ifconfig.co\n", response_length);
    #endif

    for(int i = 0; i < response_length; i++)
    {
        if(response[i] == '\r' && response[i+1] == '\n' && response[i+2] == '\r' && response[i+3] == '\n' )
        {
            sprintf(ip ,"%s", (response + i + 4));
            break;
        }
    }

    /* CLEANUP */
    close(ifco_socket);
    freeaddrinfo(result);

    return ip;
}

char* getHostname()
{
    static char hostname[_SC_HOST_NAME_MAX];
    gethostname(hostname, sizeof(hostname));

    return hostname;
}

time_t getTimestamp()
{
    return time(NULL);
}

struct hostinformation getHostinfo()
{
    struct hostinformation info = {
        .hostname = getHostname(),
        .wan_ip = getIPAdress(),
        .unix_time = getTimestamp()
    };

    #if DEBUG > 0
        printf("[DEBUG] Hostname is %s\n", info.hostname);
        printf("[DEBUG] unix_time is %ld\n", info.unix_time);
        printf("[DEBUG] wan_ip is %s", info.wan_ip);
    #endif

    return info;
}



















    /* struct addrinfo hints;
    struct addrinfo *ifconfig_co_addr;
    int ifconfig_co_socket, error = 0;
    socklen_t ifconfig_co_socket_length = 0;
    size_t response_size;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;
    hints.ai_next = NULL;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;

    error = getaddrinfo("ifconfig.co", "80", &hints, &ifconfig_co_addr);
    if(error != 0)
    {
        printf("Failed to get ifconfig.co: %s\n", gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    ifconfig_co_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(ifconfig_co_socket == -1)
    {
        printf("Failed to create socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    error = connect(ifconfig_co_socket, ifconfig_co_addr->ai_addr, sizeof(*ifconfig_co_addr->ai_addr));
    if(error != 0)
    {
        printf("Failed to connect to ifconfig.co: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char response[1000];
    char ip[15];
    char *request =
    "GET /ip HTTP/1.1\n"
    "Host: ifconfig.co\n"
    "User-Agent: iplogger\n"
    "\n";
    
    write(ifconfig_co_socket, request, strlen(request));

    response_size = read(ifconfig_co_socket, response, sizeof(response));

    int ip_length = -1;
    for(int i = response_size - 1; i >= 0; i--)
    {
        if(response[i] == '\n' && i != response_size - 1) { break; }
        ip[ip_length++] = response[i];
    }

    printf("size of ip = %i\n", ip_length);

    // for(int i = 0; i < ip_length; i++)
    // {
    //     //printf("%i = %c\n", i, ip[i]);
    //     char temp = ip[i];
    //     putchar(temp);
    //     printf("\n");
    // }

    for(int i = 0; i < ip_length/2; i++)
    {
        char temp = ip[i];  
        ip[i] = ip[13 - i - 1];  
        ip[13 - i - 1] = temp;  
    }
	
	ip[ip_length] = '\0';

    printf("%s\n", ip);

    close(ifconfig_co_socket);

    // create socket to connect to ipl_server
	
	int ipl_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(ipl_socket == -1)
	{
		printf("Error creating socket for ipl_server: %s\n", strerror(errno));
	}

	struct sockaddr_in ipl_addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK),
		.sin_port = htons(PORT)
	};

	error = connect(ipl_socket, (struct sockaddr*)&ipl_addr, sizeof(ipl_addr));
	if(error == -1)
	{
		printf("error connecting to ipl_server: %s\n", strerror(errno));
	}
	
	char request2[100];
	sprintf(request2, "%s-%s", "RASP", ip);
	
	write(ipl_socket, request2, strlen(request2));

	close(ipl_socket);
 */

