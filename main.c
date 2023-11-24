#include "board.h"
#include "LED_Matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void print2DArray(int rows, int cols, int array[rows][cols]) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", array[i][j]);
        }
        printf("\n");
    }
}

void printArrayInRows(const int* array, int size, int elementsPerRow) {
    for (int i = 0; i < size; i++) {
        printf("%d ", array[i]);

        // Check if it's time to start a new row
        if ((i + 1) % elementsPerRow == 0) {
            printf("\n");  // Move to the next line for a new row
        }
    }

    // Print a newline if the last row is incomplete
    if (size % elementsPerRow != 0) {
        printf("\n");
    }
}

void printArray(int arr[], int size) {
    for (int i = 0; i < size; ++i) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

typedef enum {
    IDLE,
    PICKED_UP,
    PLACED_DOWN,
} STATE;

int main(){
    int* moveArr;
    int* led_arr;
    int rsArr[BOARD_SIZE][BOARD_SIZE] = {0};
    initChessboard();
    while(1){
        for(int i = 0; i < BOARD_SIZE; i++){
            for(int j = 0; j < BOARD_SIZE; j++){
                TILE currentTile = getTile(i,j);
                //printf("%s\n", currentTile.rs.filePath);
                rsArr[i][j] = readIntFromFile(currentTile.rs.filePath);
                //printf("%d\n",readIntFromFile(currentTile.rs.filePath));
            }
        }
        print2DArray(BOARD_SIZE, BOARD_SIZE, rsArr);
        printf("----------------\n");
        sleep(1);
    }
    
    free(moveArr);
    free(led_arr);
    //displayBoard();
    //movePiece(1,1,2,2);
    //displayBoard();
}