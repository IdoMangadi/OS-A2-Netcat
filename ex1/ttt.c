#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

size_t hou_won(size_t board[3][3]){

}

int main(int argc, char *argv[]) {

    // Argument handling:
    if (argc != 2 || strlen(argv[1]) != 9) {
        fprintf(stderr, "Invalid argument!\n");
        return 1;
    }

    int digit_count[10] = {0};
    char *input = argv[1];

    for (int i = 0; i < 9; i++) {
        // argument contain non-digit char || argument contain 0 || argument contain the same digit twice
        if (!isdigit(input[i]) || input[i] == '0' || ++digit_count[input[i] - '0'] > 1) {
            fprintf(stderr, "Invalid argument!\n");
            return 1;
        }
    }

    // // Converting the argument:
    // size_t ai_strategy = strtoul(input, NULL, 10);  // containing the argument as a size_t 
    // if (ai_strategy == 0) {
    //     fprintf(stderr, "error\n");
    //     return 1;
    // }

    size_t board[3][3] = {0};  // when: 1 = ai , 2 = human user
    size_t ai_pointer = 0;
    char user_input;

    board[ai_pointer/3][ai_pointer%3] = 1;  // ai first move

    for(size_t i=0; i<4; i++){  // loop over the turns of ai and human user
        do{
            // scaning user input:
            if(scanf("%c", user_input) == -1){
                fprintf(stderr, "scanf error\n");
                return 1;
            }
        } while(user_input < '1' || user_input > '9'  // user input is not a digit
                || board[((size_t)(user_input-'0'))/3][((size_t)(user_input-'0'))%3] != 0);  // board in that place already assigned
        
        
    }


    return 0;
}