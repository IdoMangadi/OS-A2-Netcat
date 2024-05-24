#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFER_SIZE 2048

/**
 * generating tcp socket, operates: socket, bind, listen and accept.
 * NOTE: blocking function. ends ready to receive data from connected client.
*/
void start_tcp_server(int *server_fd, int *client_fd, int port) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    printf("starting tcp server\n");
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);  // creating the socket on the given io fd
    if (*server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    printf("binding server\n");
    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("server is listening\n");
    if (listen(*server_fd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    *client_fd = accept(*server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (*client_fd < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    printf("client accepted!\n");
}

/**
 * generating tcp client. 
 * ends with connected client ready to send data.
*/
void start_tcp_client(int *client_fd, char *hostname, int port) {
    struct sockaddr_in server_addr;
    struct hostent *server;
    printf("starting tcp client\n");
    *client_fd = socket(AF_INET, SOCK_STREAM, 0);  // creating the socket on the given io fd
    if (*client_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);
    printf("connecting to the tcp server\n");
    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
}

void close_fds(int fds[], size_t size){
    for(int i=0; i<size; i++){
        if(fds[i] != 1) close(fds[i]);
    }
}

int main(int argc, char *argv[]) {

    int opt;
    char *executable = NULL;
    char *input_mode = NULL;
    char *output_mode = NULL;
    int b_flag = 0;

    // Parse command line options
    while ((opt = getopt(argc, argv, "e:i:o:b:")) != -1) {
        switch (opt) {
            case 'e':
                executable = optarg;
                break;
            case 'i':
                input_mode = optarg;
                break;
            case 'o':
                output_mode = optarg;
                break;
            case 'b':
                input_mode = optarg;
                output_mode = optarg;
                b_flag = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s -e \"executable arguments\" [-i input_mode] [-o output_mode] [-b both_mode]\n", argv[0]);
                return 1;
        }
    }

    char *executable_name, *executable_arg;
    char *ttt_args[3];

    if(executable != NULL){  // means the user provide -e argument

        // Separate executable and arguments
        executable_name = strtok(executable, " ");  // first call
        executable_arg = strtok(NULL, " ");  // second call

        if (executable_name == NULL || executable_arg == NULL) {
            fprintf(stderr, "Invalid format for -e option. Expected format: \"executable arguments\"\n");
            exit(EXIT_FAILURE);
        }

        // Prepare arguments for execv
        ttt_args[0] = executable_name;
        ttt_args[1] = executable_arg;
        ttt_args[2] = NULL;
    }

    // Handle input & output redirections:
    int input_fd = -1, output_fd = -1;
    int tcp_server_in_fd = -1, tcp_server_out_fd = -1;  // to store the listening sockets fd

    if (input_mode != NULL) {  // means the user passed -i or -b 

        if (strncmp(input_mode, "TCPS", 4) == 0) {  // handling tcp server:
            int port = atoi(input_mode + 4);
            start_tcp_server(&tcp_server_in_fd, &input_fd, port);  // input_fd will hold the socket returned from 'accept' (the current client socket as the server side).
            printf("start tcp server returned\n");
        
        } else if (strncmp(input_mode, "TCPC", 4) == 0) {  // handling tcp client:
            char *hostname = strtok(input_mode + 4, ",");
            int port = atoi(strtok(NULL, ","));
            printf("hostname: %s, port: %d\n", hostname, port);
            start_tcp_client(&input_fd, hostname, port); // means that the incomming messages from server will be throw input_fd.
        }
    }
    if(b_flag){  // means the user passed -b
        output_fd = input_fd;  // updating output_fd
    }
    else if (output_mode != NULL) {  // means the user passed -o

        if (strncmp(output_mode, "TCPS", 4) == 0) { // handling tcp server:
            int port = atoi(output_mode + 4);
            start_tcp_server(&tcp_server_out_fd, &output_fd, port);  // output_fd will be the socket returned from 'accept'.

        } else if (strncmp(output_mode, "TCPC", 4) == 0) {  // handling tcp client:
            char *hostname = strtok(output_mode + 4, ",");
            int port = atoi(strtok(NULL, ","));
            start_tcp_client(&output_fd, hostname, port);
        }
    }

    // saving terminal input and keyboard output:
    int original_input_fd = STDIN_FILENO, original_output_fd = STDOUT_FILENO;
    // Redirect input
    if (input_fd != -1) {  // means there was assignition of input strem
        original_input_fd = dup(STDIN_FILENO); 
        dup2(input_fd, STDIN_FILENO);  // duplicate input_fd to be in stdin
    }

    // Redirect output
    if (output_fd != -1) {  // means there was assignition of output strem
        original_output_fd = dup(STDOUT_FILENO);
        dup2(output_fd, STDOUT_FILENO);  // duplicate output_fd to be in stdout
    }

    // creating access to original output strem:
    FILE *original_os = fdopen(original_output_fd, "w");
    if (original_os == NULL) {
        fprintf(stderr, "fdopen failed");
        close(original_output_fd);
        close(output_fd);
        close(input_fd);
        return 1;
    }
    // creating access to original input strem:
    FILE *original_is = fdopen(original_input_fd, "r");
    if (original_is == NULL) {
        fprintf(stderr, "fdopen failed");
        close(original_input_fd);
        close(output_fd);
        close(input_fd);
        return 1;
    }

    // handling running programm with -e:
    if(executable != NULL){

        // closing unneeded fds:
        if(input_fd != -1) close(input_fd);
        if(output_fd != -1) close(output_fd);
        if(tcp_server_in_fd!= -1) close(tcp_server_in_fd);
        if(tcp_server_out_fd != -1) close(tcp_server_out_fd);

        // Execute the new program
        execv(executable_name, ttt_args);  // running the program with updated io streams
        perror("execv failed");
        return 1;
    }
    
    // if(strncmp(input_mode, "TCPC", 4) == 0){  // handling tcp client

    char buffer[BUFFER_SIZE];
    
    while (1) {

        fflush(original_os);
        // recveiving msg:
        memset(buffer, 0, BUFFER_SIZE);
        if(input_mode != NULL){  // means it is a socket:
            if(recv(input_fd, buffer, BUFFER_SIZE, 0) < 0){
                fprintf(stderr,"recv error");
                break;
            }
        }
        else{  // means it is the standart stdin
            if(fgets(buffer, BUFFER_SIZE, stdin) < 0){
                fprintf(stderr,"fgets error");
                break;
            }
        }
        fprintf(original_os, "%s\n", buffer);  // printing msg to the terminal
        fflush(original_os);

        // communicate user:
        memset(buffer, 0, BUFFER_SIZE);
        fprintf(original_os, "Enter message to send (or 'exit' to quit):\n");
        fgets(buffer, BUFFER_SIZE, original_is);  // taking input from client's keyboard
        buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline character from buffer
        if (strcmp(buffer, "exit") == 0) {  // Check if the user wants to exit
            break;
        }

        // Send message:
        if(output_mode != NULL){  // means it is a socket:
            if(send(output_fd, buffer, BUFFER_SIZE, 0) < 0){
                fprintf(stderr,"send error");
                break;
            }
        }
        else{  // means it is the standart output
            if (fputs(buffer, stdout) < 0) {
                fprintf(stderr,"send failed");
                break;
            }
        }
    }

    // closing fds:
    if(input_fd != -1) close(input_fd);
    if(output_fd != -1) close(output_fd);
    if(tcp_server_in_fd!= -1) close(tcp_server_in_fd);
    if(tcp_server_out_fd != -1) close(tcp_server_out_fd);

    return 0;
}
