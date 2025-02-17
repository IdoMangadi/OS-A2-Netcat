#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 4096
/**
 * function to check if some side won the game in a given board.
 * returns: 0 for no one, 1 for player 1 and 2 for player 2.
*/
size_t hou_won_ttt(size_t board[3][3]){
    // Check rows and columns
    for (size_t i = 0; i < 3; i++) {
        if (board[i][0] != 0 && board[i][0] == board[i][1] && board[i][0] == board[i][2]) {
            return board[i][0];
        }
        if (board[0][i] != 0 && board[0][i] == board[1][i] && board[0][i] == board[2][i]) {
            return board[0][i];
        }
    }

    // Check diagonals
    if (board[0][0] != 0 && board[0][0] == board[1][1] && board[0][0] == board[2][2]) {
        return board[0][0];
    }
    if (board[0][2] != 0 && board[0][2] == board[1][1] && board[0][2] == board[2][0]) {
        return board[0][2];
    }

    // If no winner found, return 0 (no one)
    return 0;
}

/**
 * This function prints the noard into the string BUFFER.
*/
void board_printing_ttt(size_t board[3][3], char* buffer){
    for(size_t i=0; i<3; i++){
        sprintf(buffer+strlen(buffer),"[%zu] [%zu] [%zu]\n", board[i][0], board[i][1], board[i][2]);
    }
    sprintf(buffer+strlen(buffer),"\n");
    // fflush(stdout);
}

int main(int argc, char *argv[]) {

    // Argument handling:
    if (argc != 2 || strlen(argv[1]) != 9) {
        fprintf(stderr, "Invalid argument!\n");
        return 1;
    }

    int digit_count[10] = {0};
    char *sai_input = argv[1];

    for (int i = 0; i < 9; i++) {
        // argument contain non-digit char || argument contain 0 || argument contain the same digit twice
        if (!isdigit(sai_input[i]) || sai_input[i] == '0' || ++digit_count[sai_input[i] - '0'] > 1) {
            fprintf(stderr, "Invalid argument!\n");
            fflush(stderr);
            return 1;
        }
    }

    // game handling:
    size_t board[3][3] = {0};  // with: 1 = sai , 2 = human user

    size_t sai_pointer = 0;  // represent the 'stuped ai' current strategy
    size_t sai_curr_digit;
    char user_input;

    size_t row;
    size_t col;

    size_t turn = 1;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // printf("Welcome to Ai tic-tac-toe game\n");
    // char opening[1024];
    // if(scanf)

    for(size_t i=0; i<9; i++){  // loop over the turns of human and sai user

        if(turn == 1){  // sai turn:
            
            do{
                sai_curr_digit = sai_input[sai_pointer++] - '0' -1; // extracting the digit from the char (-1 for row and col calculation, starts from 1).
                row = sai_curr_digit / 3;
                col = sai_curr_digit % 3;
            } while(board[row][col]);  // as long as the position is already taken

            board[row][col] = 1;  // performig sai move
            sprintf(buffer+strlen(buffer), "sai move: %zu\n", sai_curr_digit+1);
        }

        else{  // user turn:
            int first = 1;
            memset(buffer, 0, BUFFER_SIZE);
            do{
                 if(!first){
                    char nv_buffer[256];  // creating not valid response
                    memset(nv_buffer, 0, 256);
                    sprintf(nv_buffer, "NOT A VALID MOVE!\n");  // not valid msg
                    board_printing_ttt(board, nv_buffer);  // appending board
                    printf("%s", nv_buffer);  // printing not valid msg and board
                    fflush(stdout);
                }
                // scaning user input:
                // printf("Your choice: ");
                if(scanf(" %c", &user_input) == -1){
                    fprintf(stderr, "scanf error\n");
                    return 1;
                }
                row = (size_t)(user_input-'0'-1) / 3;
                col = (size_t)(user_input-'0'-1) % 3;
                first = 0;
               
            // while(user input is not a digit || position in that place already taken):
            } while(user_input < '1' || user_input > '9' || board[row][col]);  // assuming the third condition will not be evaluated if the first two will return true.
            
            board[row][col] = 2;  // performig user move
            // fflush(stdout);
        }

        board_printing_ttt(board, buffer);

        // victory check:
        size_t winner = hou_won_ttt(board);
        if(winner == 1){  // means sai won
            sprintf(buffer+strlen(buffer), "I won! lunar deportation starting!\n");
            // fflush(stdout);
            // return 0;
        }
        if(winner == 2){  // means user won
            sprintf(buffer+strlen(buffer), "I lost... lunar deportation will be executed in some advance version of me...\n");
            //fflush(stdout);
            // return 0;
        }

        if(turn == 1 || winner != 0){ // sai move or winning scenario
            printf("%s", buffer);
            fflush(stdout);
            if (winner != 0) return 0;
        } 

        // turns updating:
        turn = (turn == 1) ? 2 : 1;

    }

    sprintf(buffer+strlen(buffer), "Draw\n");
    printf("%s", buffer);
    fflush(stdout);
    return 0;
}