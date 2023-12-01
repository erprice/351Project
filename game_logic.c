#include "board.h"
#include "LED_Matrix.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    WHITE_TURN,
    BLACK_TURN,
} TURN;

typedef enum {
    WAITING, //Waiting for piece to be picked up
    PICKED_UP,
    INVALID_PLACEMENT,
    WRONG_TURN
} STATE;

#define EMPTY ' '
#define WHITE 0
#define BLACK 1


TURN turn = WHITE_TURN;
int currentColour = WHITE;
STATE state = WAITING;
int* moveArr;
int currentX;
int currentY;

const int OFF[8*8] = {0};
const int ON[8*8] = {[0 ... 63] = 1};

//Runs one iteration of the game update() loop
STATE getState(){
    return state;
}

void gameUpdate(){
    switch (state){
    case WAITING:
        displayFromArr(OFF); //Turn off all leds
        //TODO: LEDs off
        for(int i = 0; i < BOARD_SIZE; i++){
            for(int j = 0; j < BOARD_SIZE; j++){
                TILE currentTile = getTile(i, j);
                //printf("Piece: %c, Val: %d\n", currentTile.piece, currentTile.rs.value);
                if(currentTile.piece != EMPTY && getColour(currentTile.piece) == currentColour && currentTile.rs.value == 0){
                    //Piece picked up
                    state = PICKED_UP;
                    currentX = i;
                    currentY = j;
                    moveArr = getPossibleMoves(currentX, currentY);
                    for (int g = 0; g < 64; ++g) {
                        printf("%d ", moveArr[g]);
                        if((g+1) % 8 == 0){
                            printf("\n");
                        }
                    }
                    printf("\n");
                    displayFromArr(moveArr);
                    //light up LEDs
                    break;
                } else if (currentTile.piece != EMPTY && getColour(currentTile.piece) != currentColour && currentTile.rs.value == 0){
                    state = WRONG_TURN;
                    currentX = i;
                    currentY = j;
                }
            }
            if(state == PICKED_UP){
                break;
            }
        }
        break;
    case PICKED_UP:
        for(int i = 0; i < BOARD_SIZE; i++){
            for(int j = 0; j < BOARD_SIZE; j++){
                TILE currentTile = getTile(i, j);
                if(currentTile.piece == EMPTY && currentTile.rs.value == 1){
                    //Moving piece to empty tile
                    if(moveArr[getIndex(i, j)] == 1){
                        movePiece(currentX, currentY, i, j);
                        free(moveArr);
                        turn = !turn; //change turns
                        currentColour = !currentColour;
                        state = WAITING;
                    } else {
                        state = INVALID_PLACEMENT;
                        printf("INVALID PLACEMENT\n");                   
                        //Not valid placement
                    }
                } else if (currentTile.piece != EMPTY && getColour(currentTile.piece) != currentColour && currentTile.rs.value == 0){
                    //Capturing a piece
                    if(moveArr[getIndex(i, j)] == 1){
                        movePiece(currentX, currentY, i, j);
                        free(moveArr);
                        turn = !turn; //change turns
                        currentColour = !currentColour; //Change colour
                        state = WAITING;
                    } else {
                        state = INVALID_PLACEMENT;
                        printf("INVALID PLACEMENT\n");
                        //Not valid placement
                    }
                } else if(i == currentX && j == currentY && currentTile.rs.value == 1){
                    state = WAITING;
                    //TODO: if placed back on its own tile
                }
            }
        }
        break;
    case WRONG_TURN:
        {
        TILE currentTile;
        //TODO: LIGHT UP ALL LEDS AS WARNING
        currentTile = getTile(currentX, currentY);
        if(currentTile.rs.value == 1){
            state = WAITING;
        }
        break;
        }
    case INVALID_PLACEMENT:
        exit(1);
        //TODO
        break;
    }
}

void reedSwitchUpdate(){
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            TILE currentTile = getTile(i, j);
            setRSValue(i, j, readReedSwitch(currentTile.rs));
        }
    }
}