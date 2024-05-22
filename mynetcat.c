#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/**
 * generating tcp socket, operates: socket, bind, listen and accept.
 * NOTE: blocking function. ends ready to receive data from connected client.
*/
void start_tcp_server(int *server_fd, int *client_fd, int port) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(*server_fd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    *client_fd = accept(*server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (*client_fd < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
}

/**
 * generating tcp client. 
 * ends with connected client ready to send data.
*/
void start_tcp_client(int *client_fd, char *hostname, int port) {
    struct sockaddr_in server_addr;
    struct hostent *server;

    *client_fd = socket(AF_INET, SOCK_STREAM, 0);
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

    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    int opt;
    char *executable = NULL;
    char *input_mode = NULL;
    char *output_mode = NULL;

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
                break;
            default:
                fprintf(stderr, "Usage: %s -e \"executable arguments\" [-i input_mode] [-o output_mode] [-b both_mode]\n", argv[0]);
                return 1;
        }
    }

    if (executable == NULL) {
        fprintf(stderr, "Usage: %s -e \"executable arguments\" [-i input_mode] [-o output_mode] [-b both_mode]\n", argv[0]);
        return 1;
    }

    // Separate executable and arguments
    char *executable_name = strtok(executable, " ");  // first call
    char *executable_arg = strtok(NULL, " ");  // second call

    if (executable_name == NULL || executable_arg == NULL) {
        fprintf(stderr, "Invalid format for -e option. Expected format: \"executable arguments\"\n");
        exit(EXIT_FAILURE);
    }

    // Prepare arguments for execv
    char *args[] = {executable_name, executable_arg, NULL};

    int input_fd = -1, output_fd = -1;

    // Handle input redirection
    if (input_mode != NULL) {  // means the user passed -i or -b argument

        if (strncmp(input_mode, "TCPS", 4) == 0) {  // handling tcp server:
            int port = atoi(input_mode + 4);
            start_tcp_server(&input_fd, &input_fd, port);
        
        } else if (strncmp(input_mode, "TCPC", 4) == 0) {  // handling tcp client:
            char *hostname = strtok(input_mode + 4, ",");
            int port = atoi(strtok(NULL, ","));
            start_tcp_client(&input_fd, hostname, port);
        }
    }

    // Handle output redirection
    if (output_mode != NULL) {  // means the user passed -o or -b argument

        if (strncmp(output_mode, "TCPS", 4) == 0) {
            int port = atoi(output_mode + 4);
            start_tcp_server(&output_fd, &output_fd, port);  // handling tcp server:

        } else if (strncmp(output_mode, "TCPC", 4) == 0) {  // handling tcp client:
            char *hostname = strtok(output_mode + 4, ",");
            int port = atoi(strtok(NULL, ","));
            start_tcp_client(&output_fd, hostname, port);
        }
    }

    // Redirect input
    if (input_fd != -1) {  // means there was assignition of input strem
        dup2(input_fd, STDIN_FILENO);  // duplicate input_fd to be the stdin
        close(input_fd);  // no need for it anymore
    }

    // Redirect output
    if (output_fd != -1) {  // means there was assignition of output strem
        dup2(output_fd, STDOUT_FILENO);  // duplicate input_fd to be the stdout
        close(output_fd);  // no need for it anymore
    }

    // Execute the new program
    execv(executable_name, args);
    perror("execv failed");
    return 1;
}
