#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/un.h>

#define BUFFER_SIZE 2048
#define TO_SEC 4
#define TO_MIC 0

/**
 * generating tcp socket, operates: socket, bind, listen and accept.
 * NOTE: blocking function. ends ready to receive data from connected client.
 */
void start_tcp_server(int *server_fd, int *client_fd, int port)
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    printf("starting tcp server\n");
    *server_fd = socket(AF_INET, SOCK_STREAM, 0); // creating the socket on the given io fd
    if (*server_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    printf("binding server\n");
    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("server is listening\n");
    if (listen(*server_fd, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    *client_fd = accept(*server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (*client_fd < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("client accepted!\n");
}

void handle_alarm(int sig)
{
    fprintf(stderr, "timeout reached\n");
    exit(EXIT_FAILURE);
}

void start_udsc_datagram(int *client_fd, char *socket_path) // open Unix Domain Socket Client Datagram, and save the fd in client_fd
{
    struct sockaddr_un server_addr;
    printf("starting unix domain socket client\n");
    *client_fd = socket(AF_UNIX, SOCK_DGRAM, 0); // creating the socket on the given io fd
    if (*client_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, socket_path);
    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
}

void start_udss_datagram(int *server_fd, char *socket_path) // open Unix Domain Socket Server Datagram, and save the fd in server_fd
{
    struct sockaddr_un server_addr;
    printf("starting unix domain socket server\n");
    *server_fd = socket(AF_UNIX, SOCK_DGRAM, 0); // creating the socket on the given io fd
    if (*server_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, socket_path);
    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    char buffer[BUFFER_SIZE];
    size_t bytes_recv = recv(*server_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_recv < 0)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    if (connect(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    { // connecting to the client, so we can use send and recv instead of sendto and recvfrom
        perror("connect");
        exit(EXIT_FAILURE);
    }
}

void start_udsc_stream(int *client_fd, char *socket_path) // open Unix Domain Socket Client Stream, and save the fd in client_fd
{
    struct sockaddr_un server_addr;
    printf("starting unix domain socket client\n");
    *client_fd = socket(AF_UNIX, SOCK_STREAM, 0); // creating the socket on the given io fd
    if (*client_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, socket_path);
    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
}
void start_udss_stream(int *server_fd, int *client_fd, char *socket_path) // open Unix Domain Socket Server Stream, and save the fd in server_fd
{
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    printf("starting unix domain socket server\n");
    *server_fd = socket(AF_UNIX, SOCK_STREAM, 0); // creating the socket on the given io fd
    if (*server_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, socket_path);
    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(*server_fd, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    *client_fd = accept(*server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (*client_fd < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("client accepted!\n");
}

/**
 * generating tcp client.
 * ends with connected client ready to send data.
 */
void start_tcp_client(int *client_fd, char *hostname, int port)
{
    struct sockaddr_in server_addr;
    struct hostent *server;
    printf("starting tcp client\n");
    *client_fd = socket(AF_INET, SOCK_STREAM, 0); // creating the socket on the given io fd
    if (*client_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);
    printf("connecting to the tcp server\n");
    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
}

void start_udp_server(int *server_fd, int port)
{

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    printf("starting udp server\n");
    *server_fd = socket(AF_INET, SOCK_DGRAM, 0); // creating the socket on the given io fd
    if (*server_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int optval = 1;
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        perror("setsockopt");
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    printf("binding server\n");
    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    char buffer[BUFFER_SIZE];
    size_t bytes_recv = recvfrom(*server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
    if (bytes_recv < 0)
    {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    if (connect(*server_fd, (struct sockaddr *)&client_addr, client_len) == -1)
    { // connecting to the client, so we can use send and recv instead of sendto and recvfrom
        perror("connect");
        exit(EXIT_FAILURE);
    }
}

void start_udp_client(int *client_fd, char *hostname, int port)
{
    struct sockaddr_in server_addr;
    struct hostent *server;
    printf("starting udp client\n");
    *client_fd = socket(AF_INET, SOCK_DGRAM, 0); // creating the socket on the given io fd
    if (*client_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);
    printf("connecting to the udp server\n");
    char *buffer = "hello, I am the client, I am ready to send you some";
    if (sendto(*client_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
}

void handle_i_args(char *input_mode, int *input_fd, int *tcp_server_in_fd)
{
    if (strncmp(input_mode, "TCPS", 4) == 0)
    { // handling tcp server:
        int port = atoi(input_mode + 4);
        start_tcp_server(tcp_server_in_fd, input_fd, port); // input_fd will hold the socket returned from 'accept' (the current client socket as the server side).
        printf("start tcp server returned\n");
    }
    else if (strncmp(input_mode, "TCPC", 4) == 0)
    { // handling tcp client:
        char *hostname = strtok(input_mode + 4, ",");
        int port = atoi(strtok(NULL, ","));
        printf("hostname: %s, port: %d\n", hostname, port);
        start_tcp_client(input_fd, hostname, port); // means that the incomming messages from server will be throw input_fd.
    }
    else if (strncmp(input_mode, "UDPS", 4) == 0)
    { // handling udp server:
        int port = atoi(input_mode + 4);
        start_udp_server(input_fd, port); // input_fd will hold the socket returned from 'accept' (the current client socket as the server side).
        printf("start udp server returned\n");
    }
    else if (strncmp(input_mode, "UDSSD", 5) == 0)
    { // handling Unix Domain Socket server over datagram
        char *socket_path = input_mode + 5;
        printf("socket path: %s\n", socket_path);
        start_udss_datagram(input_fd, socket_path);
    }
    else if (strncmp(input_mode, "UDSCS", 5) == 0)
    { // handling Unix Domain Socket client over stream
        char *socket_path = input_mode + 5;
        printf("socket path: %s\n", socket_path);
        start_udsc_stream(input_fd, socket_path);
    }
    else if (strncmp(input_mode, "UDSSS", 5) == 0)
    {
        char *socket_path = input_mode + 5;
        printf("socket path: %s\n", socket_path);
        start_udss_stream(tcp_server_in_fd, input_fd, socket_path);
    }
    else
    {
        printf("Not a valid argument for input mode\n");
        exit(1);
    }
}
void handle_o_args(char *output_mode, int *output_fd, int *tcp_server_out_fd)
{
    if (strncmp(output_mode, "TCPS", 4) == 0)
    { // handling tcp server:
        int port = atoi(output_mode + 4);
        start_tcp_server(tcp_server_out_fd, output_fd, port); // output_fd will hold the socket returned from 'accept' (the current client socket as the server side).
        printf("start tcp server returned\n");
    }
    else if (strncmp(output_mode, "TCPC", 4) == 0)
    { // handling tcp client:
        char *hostname = strtok(output_mode + 4, ",");
        int port = atoi(strtok(NULL, ","));
        printf("hostname: %s, port: %d\n", hostname, port);
        start_tcp_client(output_fd, hostname, port); // means that the incomming messages from server will be throw output_fd.
    }
    else if (strncmp(output_mode, "UDPC", 4) == 0)
    { // handling udp client:
        char *hostname = strtok(output_mode + 4, ",");
        int port = atoi(strtok(NULL, ","));
        printf("hostname: %s, port: %d\n", hostname, port);
        start_udp_client(output_fd, hostname, port); // means that the incomming messages from server will be throw output_fd.
    }
    else if (strncmp(output_mode, "UDSCD", 5) == 0)
    { // handling Unix Domain Socket client over datagram
        char *socket_path = output_mode + 5;
        printf("socket path: %s\n", socket_path);
        start_udsc_datagram(output_fd, socket_path);
    }
    else if (strncmp(output_mode, "UDSSS", 5) == 0)
    { // handling Unix Domain Socket client over datagram
        char *socket_path = output_mode + 5;
        printf("socket path: %s\n", socket_path);
        start_udss_stream(tcp_server_out_fd, output_fd, socket_path);
    }
    else if (strncmp(output_mode, "UDSCS", 5) == 0)
    { // handling Unix Domain Socket client over stream
        char *socket_path = output_mode + 5;
        printf("socket path: %s\n", socket_path);
        start_udsc_stream(output_fd, socket_path);
    }
    else
    {
        printf("Not a valid argument for output mode\n");
        exit(1);
    }
}

size_t generic_recv(int input_fd, char *input_mode,char* output_mode, char *buffer, size_t buffer_size, int timeout)
{ // uses recv or recvfrom based on the input_mode, recv for TCP and recvfrom for UDP
    size_t bytes_recv = 0;
    if (input_mode != NULL)
    {
        if (timeout != -1 && (strncmp(input_mode, "UDP", 3) == 0 || strncmp(output_mode, "UDP", 3) == 0))
        {
            signal(SIGALRM, handle_alarm);
            alarm(timeout);
        }
        bytes_recv = recv(input_fd, buffer, BUFFER_SIZE, 0);
        alarm(0);

        if (bytes_recv <= 0)
        {
            if (bytes_recv == 0)
            {
                fprintf(stderr, "Connection closed\n");
                exit(1);
            }
            else
            {
                perror("recv");
                exit(1);
            }
        }
    }

    else
    { // means it is the standart stdin
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
        {
            fprintf(stderr, "fgets error");
            return 1;
        }
    }
    return bytes_recv;
}

size_t generic_send(int output_fd, char *output_mode, char *buffer, size_t bytes_to_send)
{
    size_t bytes_sent = 0;
    if (output_mode != NULL)
    {
        bytes_sent = send(output_fd, buffer, strlen(buffer), 0);
        if (bytes_sent < 0)
        {
            fprintf(stderr, "send error");
            return 1;
        }
        return bytes_sent;
    }

    else
    { // means it is the standart stdout
        if ((bytes_sent = fputs(buffer, stdout)) < 0)
        {
            fprintf(stderr, "fputs error");
            return 1;
        }
    }
    return bytes_sent;
}

void close_fds(int fds[], size_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (fds[i] != 1)
            close(fds[i]);
    }
}
 
int main(int argc, char *argv[])
{
 
    int opt;
    char *executable = NULL;
   char *input_mode = NULL;
    char *output_mode = NULL;
    char *timeout = NULL;
    int b_flag = 0;
    int io_flag = 0;

    // Parse command line options
    while ((opt = getopt(argc, argv, "e:i:o:b:t:")) != -1)
    {
        switch (opt)
        {
        case 'e':
            executable = optarg;
            break;
        case 'i':
            input_mode = optarg;
            io_flag = 1;
            break;
        case 'o':
            output_mode = optarg;
            io_flag = 1;
            break;
        case 'b':
            input_mode = optarg;
            output_mode = optarg;
            b_flag = 1;
            break;
        case 't':
            timeout = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s -e \"executable arguments\" [-i input_mode] [-o output_mode] [-b both_mode]\n", argv[0]);
            return 1;
        }
    }

    char *executable_name, *executable_arg;
    char *ttt_args[3];

    if (b_flag && io_flag)
    {
        fprintf(stderr, "Usage: %s -e \"executable arguments\" [-i input_mode] [-o output_mode] [-b both_mode]\n", argv[0]);
        return 1;
    }

    // Handle input & output redirections:
    int input_fd = -1, output_fd = -1;
    int tcp_server_in_fd = -1, tcp_server_out_fd = -1; // to store the listening sockets fd
    if((input_mode!=NULL&&strncmp(input_mode,"TCP",3)==0) || (output_mode!=NULL&&strncmp(output_mode,"TCP",3)==0))
    {
        if(timeout!=NULL)
        {
            fprintf(stderr,"Timeout is not available for TCP\n");
            return 1;
        }
    }

    if (input_mode != NULL)
    { // means the user passed -i or -b
        handle_i_args(input_mode, &input_fd, &tcp_server_in_fd);
    }
    if (b_flag)
    {                         // means the user passed -b
        output_fd = input_fd; // updating output_fd
    }
    else if (output_mode != NULL)
    { // means the user passed -o
        handle_o_args(output_mode, &output_fd, &tcp_server_out_fd);
    }

    // saving terminal input and keyboard output:
    int original_input_fd = STDIN_FILENO, original_output_fd = STDOUT_FILENO;
    // Redirect input

    if (executable != NULL)
    { // means the user provide -e argument

        // Separate executable and arguments
        executable_name = strtok(executable, " "); // first call
        executable_arg = strtok(NULL, " ");        // second call

        if (executable_name == NULL || executable_arg == NULL)
        {
            fprintf(stderr, "Invalid format for -e option. Expected format: \"executable arguments\"\n");
            exit(EXIT_FAILURE);
        }

        // Prepare arguments for execv
        ttt_args[0] = executable_name;
        ttt_args[1] = executable_arg;
        ttt_args[2] = NULL;

        if (input_fd != -1)
        { // means there was assignition of input strem
            original_input_fd = dup(STDIN_FILENO);
            if (dup2(input_fd, STDIN_FILENO) == -1)
            {
                fprintf(stderr, "dup2 failed");
                close(original_input_fd);
                close(output_fd);
                close(input_fd);
                return 1;
            } // duplicate input_fd to be in stdin
        }

        // Redirect output
        if (output_fd != -1)
        { // means there was assignition of output strem
            original_output_fd = dup(STDOUT_FILENO);
            if (dup2(output_fd, STDOUT_FILENO) == -1)
            {
                fprintf(stderr, "dup2 failed");
                close(original_output_fd);
                close(output_fd);
                close(input_fd);
                return 1;
            } // duplicate output_fd to be in stdout
        }
        
    

        // closing unneeded fds:
        if (input_fd != -1)
            close(input_fd);
        if (output_fd != -1)
            close(output_fd);
        if (tcp_server_in_fd != -1)
            close(tcp_server_in_fd);
        if (tcp_server_out_fd != -1)
            close(tcp_server_out_fd);

        // Execute the new program
        execv(executable_name, ttt_args); // running the program with updated io streams
        perror("execv failed");
        return 1;
    
    }

    // creating access to original output strem:
    FILE *original_os = fdopen(original_output_fd, "w");
    if (original_os == NULL)
    {
        fprintf(stderr, "fdopen failed");
        close(original_output_fd);
        close(output_fd);
        close(input_fd);
        return 1;
    }
    // creating access to original input strem:
    FILE *original_is = fdopen(original_input_fd, "r");
    if (original_is == NULL)
    {
        fprintf(stderr, "fdopen failed");
        close(original_input_fd);
        close(output_fd);
        close(input_fd);
        return 1;
    }

    signal(SIGALRM, handle_alarm);
   

    char buffer[BUFFER_SIZE];
    int timeout_int = -1;
    if (timeout != NULL)
    {
        timeout_int = atoi(timeout);
    }

    int flag = 0;
    int isClient = 0;
    if ((input_mode != NULL && input_mode[3] == 'C') || (output_mode != NULL && output_mode[3] == 'C'))
        isClient = 1;
    while (1)
    {

        if (isClient || flag)
        { // if it is a client or not the first iteration, we need to receive a message, and not to wait for the user to enter a message.
            size_t bytes_recv = 0;
            fflush(original_os);
            // recveiving msg:
            memset(buffer, 0, BUFFER_SIZE);
            bytes_recv = generic_recv(input_fd, input_mode,output_mode, buffer, BUFFER_SIZE, timeout_int);
            if (input_mode != NULL && bytes_recv > 0)
            {                                         // means there was a message
                fprintf(original_os, "%s\n", buffer); // printing msg to the terminal
            }
        }

        flag = 1;
        // communicate user from original i/o:
        memset(buffer, 0, BUFFER_SIZE);
        fprintf(original_os, "Enter message to send (or 'exit' to quit):\n");
        if (timeout_int != -1 && ((input_mode!=NULL&&strncmp(input_mode, "UDP", 3) == 0) ||(output_mode!=NULL&&strncmp(output_mode, "UDP", 3) == 0)))
        {
            signal(SIGALRM, handle_alarm);
            alarm(timeout_int);
        }
        fgets(buffer, BUFFER_SIZE, original_is); // taking input from client's keyboard
        alarm(0);
        buffer[strcspn(buffer, "\n")] = '\0';    // Remove newline character from buffer
        if (strcmp(buffer, "exit") == 0)
        { // Check if the user wants to exit
            break;
        }

        // Send message:
        generic_send(output_fd, output_mode, buffer, (size_t)strlen(buffer));
    }

    // closing fds:
    if (input_fd != -1)
        close(input_fd);
    if (output_fd != -1)
        close(output_fd);
    if (tcp_server_in_fd != -1)
        close(tcp_server_in_fd);
    if (tcp_server_out_fd != -1)
        close(tcp_server_out_fd);

    return 0;
}
