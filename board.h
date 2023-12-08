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

void initChessboard(); //initialize the board

void initChessboardForTesting(); //to test the software without the BBG

//MUST call free on the returned array
int* getPossibleMoves(int x, int y);

bool isInCheck(int colour);

void displayBoard();

void displayRSValues(); //display the sensor values in 2D format

int getIndex(int x, int y); //given the coordinates, returns the 1D array index

int readReedSwitch(reedSwitch rs); //reads the value of the sensor

bool isPickedUp(TILE tile); //checks if the piece is picked up

bool isPlaced(TILE tile); //checks if the piece is placed down on the board

int* convertToLEDarray(int* array); //generates the LED array to display on the board

void movePiece(int x, int y, int x2, int y2); //move a piece to a certain tile

int getColour(char c); //gets color of the piece

TILE getTile(int x, int y); //returns the tile given the coordinates

void setRSValue(int x, int y, int value); //set sensor values manually for testing

int getKingX(int colour); //gets the King's x coordinate
int getKingY(int colour); //gets the King's y coordinate




#endif