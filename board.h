#include <stdbool.h>

#ifndef BOARD_H
#define BOARD_H

#define BOARD_SIZE 5

typedef struct{
    int value;
    char* filePath;
}reedSwitch;

typedef struct{
    char piece; // ' '
    reedSwitch rs; // 1
}TILE;

void initChessboard();

void initChessboardForTesting();

//int readIntFromFile(char* filePath);

//Returns an array of length 64, 0 = piece cannot move there, 1 = piece CAN move there.
//MUST call free on the returned array
int* getPossibleMoves(int x, int y);

void displayBoard();

void displayRSValues();

int getIndex(int x, int y);

int readReedSwitch(reedSwitch rs);

bool isPickedUp(TILE tile);

bool isPlaced(TILE tile);

int* convertToLEDarray(int* array);

//int arrayToBinaryInteger(int* array);

void movePiece(int x, int y, int x2, int y2);

int getColour(char c);

TILE getTile(int x, int y);

void setRSValue(int x, int y, int value);




#endif