#include "board.h"
#include "LED_Matrix.h"
#include "game_logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"

char *pointer1 = "0\n";
const int MAX_LENGTH1 = 1024;

// int read_Button() //reads the value of the gpio pin by opening the file and comparing the read value with 0\n
// {
//     FILE * f = fopen("/sys/class/gpio/gpio75/value",  "r"); //open file
//     if (f == NULL) //if the file cannot be opened exit program and print error.
//     {
//         printf("ERROR: Unable to open file (%s) for read\n", "/sys/class/gpio/gpio72/value");
//         exit(-1);
//     }
//     sleep(.005);
//     char buff[MAX_LENGTH1];
//     fgets(buff, MAX_LENGTH1, f); //read content pin file
//     sleep(1.0050);
//     fclose(f); //close file
//     return strcmp(buff, pointer1); //if 0\n is read return 0 otherwise return 1 (gpio pin is active low so 0 = pressed)
// }

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
#define ROW_ON 0b11111111
#define ROW_OFF 0b00000000

int main(){
    //configPin(8,42, 0);
    initChessboard();
    set_i2cFileDesc();
    reset_Display();
    startThreads();
    while(1){
        //do nothing
    }
    int array[] = {ROW_ON, ROW_OFF, ROW_OFF, ROW_OFF, ROW_OFF, ROW_OFF, ROW_OFF, ROW_OFF};
    for(int i = 0; i < 5; i++){
        displayFromArr(array);
        array[i] = ROW_OFF;
        array[++i] = ROW_ON;
        sleep(0.5);
    }
    sleep(1);
    reset_Display();
    turnOffDisplay();



    // stopThreads();
    // while(1){
    //     printf("WAITING = 0, PICKED_UP = 1 ->%d\n", getState());
    //     displayRSValues();
    //     printf("\n");
    //     displayBoard();
    //     printf("-----\n");
    //     printf("X value: ");
    //     scanf("%d", &x);
    //     printf("Y value: ");
    //     scanf("%d", &y);
    //     printf("\n");
    //     TILE tempTile = getTile(x, y);
    //     setRSValue(x, y, (!tempTile.rs.value));
    //     gameUpdate();
    // }
}