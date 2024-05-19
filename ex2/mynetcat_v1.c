#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <bits/getopt_core.h>

#define OUTPUT_FILE "ttt_output.txt"

int main(int argc, char *argv[]) {
    int opt;
    char *executable = NULL;

    // Parse command-line options using getopt
    while ((opt = getopt(argc, argv, "e:")) != -1) {
        switch (opt) {
            case 'e':
                executable = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -e <executable>\n", argv[0]);
                return 1;
        }
    }

    // Check if the executable was provided
    if (executable == NULL) {
        fprintf(stderr, "Usage: %s -e <executable>\n", argv[0]);
        return 1;
    }

    // Open the output file in append mode
    FILE *output = fopen(OUTPUT_FILE, "a");
    if (output == NULL) {
        perror("Error opening output file");
        return 1;
    }

    // Execute the specified program with its arguments and redirect output to the file
    char command[1024];
    snprintf(command, sizeof(command), "%s >> %s", executable, OUTPUT_FILE);
    if (system(command) == -1) {
        perror("Error executing program");
        fclose(output);
        return 1;
    }

    // Close the output file
    fclose(output);

    return 0;
}