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

#define BUFFER_SIZE 2048  // buffer size for the messages
#define TO_SEC 4  // timeout in seconds
#define TO_MIC 0 // timeout in microseconds

int fds[4] = {-1, -1, -1, -1};  // array to hold the fds 
int fds_size = 0;  // size of the fds array
char unix_path[100];  // path for the unix domain socket

/**
 * closing all the fds in the fds array, and unlink the unix domain socket if it was created.
*/
void close_fds(int fds[], size_t size, int EXIT_CODE)
{
    for (int i = 0; i < size; i++)
    {
        if (fds[i] > -1) close(fds[i]);
    }
    if (unix_path != NULL)
    {
        unlink(unix_path);
    }
    exit(EXIT_CODE);
}

/**
 * generating tcp socket, operates: socket, bind, listen and accept.
 * NOTE: blocking function. ends ready to receive data from connected client.
 */
void start_tcp_server(int *server_fd, int *client_fd, int port)
{
    // creating the socket
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    printf("starting tcp server\n");
    *server_fd = socket(AF_INET, SOCK_STREAM, 0); // creating the socket on the given io fd
    if (*server_fd < 0)
    {
        perror("socket");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    printf("binding server\n");
    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    printf("server is listening\n");
    if (listen(*server_fd, 1) < 0)
    {
        perror("listen");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }

    *client_fd = accept(*server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (*client_fd < 0)
    {
        perror("accept");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    printf("client accepted!\n");
}

/**
 * handling the alarm signal, and printing a message.
*/
void handle_alarm(int sig)
{
    fprintf(stderr, "timeout reached\n");
    close_fds(fds, fds_size, EXIT_FAILURE);
}

/**
 * generating Unix Domain Socket Client Stream, operates: socket, connect.
*/
void start_udsc_datagram(int *client_fd, char *socket_path) // open Unix Domain Socket Client Datagram, and save the fd in client_fd
{
    struct sockaddr_un server_addr;
    printf("starting unix domain socket client over datagram\n");
    *client_fd = socket(AF_UNIX, SOCK_DGRAM, 0); // creating the socket on the given io fd
    if (*client_fd < 0)
    {
        perror("socket");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, socket_path);
    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    printf("connected to the server\n");
}

/**
 * generating Unix Domain Socket Server Datagram, operates: socket, bind, connect.
*/
void start_udss_datagram(int *server_fd, char *socket_path) // open Unix Domain Socket Server Datagram, and save the fd in server_fd
{
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;
    bzero((char *)&client_addr, sizeof(client_addr));
    bzero((char *)&server_addr, sizeof(server_addr));
    socklen_t client_len = sizeof(client_addr);
    printf("starting unix domain socket server over datagram\n");
    *server_fd = socket(AF_UNIX, SOCK_DGRAM, 0); // creating the socket on the given io fd
    if (*server_fd < 0)
    {
        perror("socket");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    server_addr.sun_family = AF_UNIX;
    unlink(socket_path);  // unlink the socket path

    strcpy(server_addr.sun_path, socket_path);
    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    if (connect(*server_fd, (struct sockaddr *)&client_addr, client_len) == -1)
    { // connecting to the client, so we can use send and recv instead of sendto and recvfrom
        perror("connect");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
}

/**
 * generating Unix Domain Socket Client Stream, operates: socket, connect.
*/
void start_udsc_stream(int *client_fd, char *socket_path) // open Unix Domain Socket Client Stream, and save the fd in client_fd
{
    struct sockaddr_un server_addr;
    printf("starting unix domain socket client over stream\n");
    *client_fd = socket(AF_UNIX, SOCK_STREAM, 0); // creating the socket on the given io fd
    if (*client_fd < 0)
    {
        perror("socket");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, socket_path);
    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
}

/**
 * generating Unix Domain Socket Server Stream, operates: socket, bind, listen, accept.
 * NOTE: blocking function. ends ready to receive data from connected client.
*/
void start_udss_stream(int *server_fd, int *client_fd, char *socket_path) // open Unix Domain Socket Server Stream, and save the fd in server_fd
{
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    printf("starting unix domain socket server over stream\n");
    *server_fd = socket(AF_UNIX, SOCK_STREAM, 0); // creating the socket on the given io fd
    if (*server_fd < 0)
    {
        perror("socket");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    server_addr.sun_family = AF_UNIX;
    unlink(socket_path);
    strcpy(server_addr.sun_path, socket_path);
    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    if (listen(*server_fd, 1) < 0)
    {
        perror("listen");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    *client_fd = accept(*server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (*client_fd < 0)
    {
        perror("accept");
        close_fds(fds, fds_size, EXIT_FAILURE);
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
        close_fds(fds, fds_size, EXIT_FAILURE);
    }

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);
    printf("connecting to the tcp server\n");
    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
}

/**
 * generating udp server.
*/
void start_udp_server(int *server_fd, int port)
{

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    printf("starting udp server\n");
    *server_fd = socket(AF_INET, SOCK_DGRAM, 0); // creating the socket on the given io fd
    if (*server_fd < 0)
    {
        perror("socket");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    int optval = 1;
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        perror("setsockopt");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    printf("binding server\n");
    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    char buffer[BUFFER_SIZE];
    size_t bytes_recv = recvfrom(*server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
    if (bytes_recv < 0)
    {
        perror("recvfrom");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    if (connect(*server_fd, (struct sockaddr *)&client_addr, client_len) == -1)
    { // connecting to the client, so we can use send and recv instead of sendto and recvfrom
        perror("connect");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
}

/**
 * generating udp client.
 * NOTE: ends with connected client ready to send data.
*/
void start_udp_client(int *client_fd, char *hostname, int port)
{
    struct sockaddr_in server_addr;
    struct hostent *server;
    printf("starting udp client\n");
    *client_fd = socket(AF_INET, SOCK_DGRAM, 0); // creating the socket on the given io fd
    if (*client_fd < 0)
    {
        perror("socket");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        close_fds(fds, fds_size, EXIT_FAILURE);
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
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
}

/**
 * handling the input arguments, and starting the appropriate server/client.
*/
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
        strcpy(unix_path, socket_path);

        printf("socket path: %s\n", socket_path);
        start_udss_datagram(input_fd, socket_path);
    }
    else if (strncmp(input_mode, "UDSCS", 5) == 0)
    { // handling Unix Domain Socket client over stream
        char *socket_path = input_mode + 5;
        strcpy(unix_path, socket_path);

        printf("socket path: %s\n", socket_path);
        start_udsc_stream(input_fd, socket_path);
    }
    else if (strncmp(input_mode, "UDSSS", 5) == 0)
    {
        char *socket_path = input_mode + 5;
        strcpy(unix_path, socket_path);

        printf("socket path: %s\n", socket_path);
        start_udss_stream(tcp_server_in_fd, input_fd, socket_path);
    }
    else
    {
        printf("Not a valid argument for input mode\n");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
}

/**
 * handling the output arguments, and starting the appropriate server/client.
*/
void handle_o_args(char *output_mode, int *output_fd, int *tcp_server_out_fd)
{
    if (strncmp(output_mode, "TCPS", 4) == 0)
    { // handling tcp server:
        int port = atoi(output_mode + 4);
        start_tcp_server(tcp_server_out_fd, output_fd, port); // output_fd will hold the socket returned from 'accept' (the current client socket as the server side).
        // printf("start tcp server returned\n");
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
        strcpy(unix_path, socket_path);
        printf("socket path: %s\n", socket_path);
        start_udsc_datagram(output_fd, socket_path);
    }
    else if (strncmp(output_mode, "UDSSS", 5) == 0)
    { // handling Unix Domain Socket client over datagram
        char *socket_path = output_mode + 5;
        strcpy(unix_path, socket_path);
        printf("socket path: %s\n", socket_path);
        start_udss_stream(tcp_server_out_fd, output_fd, socket_path);
    }
    else if (strncmp(output_mode, "UDSCS", 5) == 0)
    { // handling Unix Domain Socket client over stream
        char *socket_path = output_mode + 5;
        strcpy(unix_path, socket_path);
        printf("socket path: %s\n", socket_path);
        start_udsc_stream(output_fd, socket_path);
    }
    else
    {
        printf("Not a valid argument for output mode\n");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }
}

/**
 * generic recv function, handles the input from the user or from the input stream.
*/
size_t generic_recv(int input_fd, char *input_mode, char *buffer, size_t buffer_size)
{
    size_t bytes_recv = 0;
    if (input_mode != NULL)
    {
        bytes_recv = recv(input_fd, buffer, BUFFER_SIZE, 0);

        if (bytes_recv <= 0)
        {
            if (bytes_recv == 0)
            {
                fprintf(stderr, "Connection closed\n");
                close_fds(fds, fds_size, EXIT_FAILURE);
            }
            else
            {
                perror("recv");
                close_fds(fds, fds_size, EXIT_FAILURE);
            }
        }
    }

    else
    { // means it is the standart stdin
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
        {
            fprintf(stderr, "fgets error");
            close_fds(fds, fds_size, EXIT_FAILURE);
        }
    }
    return bytes_recv;
}

/**
 * generic send function, handles the output to the user or to the output stream.
*/
size_t generic_send(int output_fd, char *output_mode, char *buffer, size_t bytes_to_send)
{
    size_t bytes_sent = 0;
    if (output_mode != NULL)
    {
        bytes_sent = send(output_fd, buffer, strlen(buffer), 0);
        if (bytes_sent < 0)
        {
            fprintf(stderr, "send error");
            close_fds(fds, fds_size, EXIT_FAILURE);
        }
        return bytes_sent;
    }

    else
    { // means it is the stdout
        if ((bytes_sent = printf("%s\n",buffer)) < 0)
        {
            fprintf(stderr, "fputs error");
            close_fds(fds, fds_size, EXIT_FAILURE);
        }
        fflush(stdout);
    }
    return bytes_sent;
}

/**
 * main function, handling the command line arguments, and the communication between the user and the server/client.
*/
int main(int argc, char *argv[])
{
    int opt;
    char *executable = NULL;  // the executable to run
    char *input_mode = NULL;  // the input mode
    char *output_mode = NULL;  // the output mode
    char *timeout = NULL;  // the timeout
    int b_flag = 0;  // flag to check if the both mode was used
    int io_flag = 0;  // flag to check if the input/output mode was used

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
            close_fds(fds, fds_size, EXIT_FAILURE);
        }
    }

    char *executable_name, *executable_arg;
    char *ttt_args[3];  // array to hold the arguments for the execv

    if (b_flag && io_flag)  // if the both mode was used with the input/output mode
    {
        fprintf(stderr, "Usage: %s -e \"executable arguments\" [-i input_mode] [-o output_mode] [-b both_mode]\n", argv[0]);
        close_fds(fds, fds_size, EXIT_FAILURE);
    }

    // Handle input & output redirections:
    int input_fd = -1, output_fd = -1;  // to store the input/output fds
    int tcp_server_in_fd = -1, tcp_server_out_fd = -1; // to store the listening sockets fd

    // saving the fds in the fds array:
    fds[fds_size++] = input_fd;
    fds[fds_size++] = output_fd;
    fds[fds_size++] = tcp_server_in_fd;
    fds[fds_size++] = tcp_server_out_fd;

    // Check if the user provided a timeout for TCP:
    if ((input_mode != NULL && strncmp(input_mode, "TCP", 3) == 0) || (output_mode != NULL && strncmp(output_mode, "TCP", 3) == 0))
    {
        if (timeout != NULL)
        {
            fprintf(stderr, "Timeout is not available for TCP\n");
            close_fds(fds, fds_size, EXIT_FAILURE);
        }
    }

    if (input_mode != NULL)
    { // means the user passed -i or -b
        handle_i_args(input_mode, &input_fd, &tcp_server_in_fd);
    }
    if (b_flag)
    { // means the user passed -b
        output_fd = input_fd; // updating output_fd
    }
    else if (output_mode != NULL)
    { // means the user passed -o
        handle_o_args(output_mode, &output_fd, &tcp_server_out_fd);
    }

    // handling the executable:
    if (executable != NULL)
    {
        // Separate executable and arguments
        executable_name = strtok(executable, " "); // first call
        executable_arg = strtok(NULL, " ");        // second call

        if (executable_name == NULL || executable_arg == NULL)
        {
            fprintf(stderr, "Invalid format for -e option. Expected format: \"executable arguments\"\n");
            close_fds(fds, fds_size, EXIT_FAILURE);
        }

        // Prepare arguments for execv
        ttt_args[0] = executable_name;
        ttt_args[1] = executable_arg;
        ttt_args[2] = NULL;

        // redirecting the input/output streams to the executable:
        if (input_fd != -1)
        { // means there was assignition of input strem
            // original_input_fd = dup(STDIN_FILENO);
            if (dup2(input_fd, STDIN_FILENO) == -1)
            {
                fprintf(stderr, "dup2 failed");
                close_fds(fds, fds_size, EXIT_FAILURE);
            } // duplicate input_fd to be in stdin
        }
        if (output_fd != -1)
        { // means there was assignition of output strem
            // original_output_fd = dup(STDOUT_FILENO);
            if (dup2(output_fd, STDOUT_FILENO) == -1)
            {
                fprintf(stderr, "dup2 failed");
                close_fds(fds, fds_size, EXIT_FAILURE);
            } // duplicate output_fd to be in stdout
        }

        // closing unneeded fds:
        if (input_fd != -1)
        {
            close(input_fd);
            fds[0] = -1;
        }
        if (output_fd != -1)
        {
            close(output_fd);
            fds[1] = -1;
        }
        if (tcp_server_in_fd != -1)
        {
            close(tcp_server_in_fd);
            fds[2] = -1;
        }
        if (tcp_server_out_fd != -1)
        {
            close(tcp_server_out_fd);
            fds[3] = -1;
        }

        // Execute the new program
        execv(executable_name, ttt_args); // running the program with updated io streams
        perror("execv failed");
        close_fds(fds, fds_size, EXIT_FAILURE);
    }

    signal(SIGALRM, handle_alarm);  // setting the alarm signal

    char buffer[BUFFER_SIZE];  // buffer to hold the messages
    int timeout_int = -1;  // timeout in int
    if (timeout != NULL)
    {
        timeout_int = atoi(timeout);
    }

    int flag = 0;  // flag to check if it is the first iteration

    int isClient = 0, isServer = 0;  // flags to check if it is a client or a server
    if ((input_mode != NULL && input_mode[3] == 'C') || (output_mode != NULL && output_mode[3] == 'C'))
        isClient = 1;
    if((input_mode != NULL && input_mode[3] == 'S') || (output_mode != NULL && output_mode[3] == 'S'))
        isServer = 1;

    // checking if the timeout is valid for the input/output mode (only for datagram mode)
    int valid_timeout_input = ((input_mode != NULL) && ((strncmp(input_mode, "UDP", 3) == 0) || (strncmp(input_mode, "UDSSD", 5) == 0)));
    int valid_timeout_output = ((output_mode != NULL) && ((strncmp(output_mode, "UDP", 3) == 0) || (strncmp(output_mode, "UDSCD", 5) == 0)));

    // main loop to communicate between the user and the server/client:
    while (1)
    {
        if (isClient || flag)
        { // if it is a client or not the first iteration, we need to receive a message, and not to wait for the user to enter a message.
            size_t bytes_recv = 0;
            fflush(stdout);
            // recveiving msg:
            memset(buffer, 0, BUFFER_SIZE);
            if ((timeout_int != -1) && (valid_timeout_input || valid_timeout_output))
            { // if it is datagram, we need to set a timeout
                signal(SIGALRM, handle_alarm);
                alarm(timeout_int);
            }
            bytes_recv = generic_recv(input_fd, input_mode, buffer, BUFFER_SIZE);
            alarm(0);

            if (input_mode != NULL && bytes_recv > 0)
            { // means there was a message
                printf("%s\n", buffer); // printing msg to the terminal
            }
        }

        flag = 1;
        // communicate user from original i/o:
        if(input_mode != NULL || isServer)
        {  // check if the user used stdin in the generic recv above.
            memset(buffer, 0, BUFFER_SIZE);
            printf("Enter message to send (or 'exit' to quit):\n");
            fgets(buffer, BUFFER_SIZE, stdin); // taking input from client's keyboard
        }
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character from buffer
        if (strcmp(buffer, "exit") == 0)
        { // Check if the user wants to exit
            close_fds(fds, fds_size, EXIT_SUCCESS);
        }

        // Send message:
        if ((timeout_int != -1) && (valid_timeout_input || valid_timeout_output))
        { // if it is datagram, we need to set a timeout
            signal(SIGALRM, handle_alarm);
            alarm(timeout_int);
        }
        generic_send(output_fd, output_mode, buffer, (size_t)strlen(buffer));
        alarm(0);
    }

    // closing fds:
    close_fds(fds, fds_size, EXIT_SUCCESS);

    return 0;
}
