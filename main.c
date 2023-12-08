#include "board.h"
#include "LED_Matrix.h"
#include "game_logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

char *pointer1 = "0\n";
const int MAX_LENGTH1 = 1024;

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

int main(){
    initChessboard();
    set_i2cFileDesc();
    reset_Display();
    startThreads(); //start the threads 

    //this loop was needed to keep the threads running multiple times
    while(1){
        //do nothing
    }
   
   //stop threads when the game enters CHECKMATE state (gamelogic.c)
}