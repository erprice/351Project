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
    WRONG_TURN,
    CAPTURING,
    CHECK,
    CHECKMATE
} STATE;

#define EMPTY ' '
#define WHITE 0
#define BLACK 1


TURN turn = WHITE_TURN;
int currentColour = WHITE;
STATE state = WAITING;
int* moveArr;
int* moveArr2;
int currentX;
int currentY;
int wrongX;
int wrongY;
int capturingX;
int capturingY;

const int OFF[8] = {[0 ... 7] = 0b00000000};
const int ON[8*8] = {[0 ... 63] = 0b11111111};

//Runs one iteration of the game update() loop
STATE getState(){
    return state;
}

void gameUpdate(){
    switch (state){
    case WAITING:
        reset_Display(); //Turn off all leds
        printf("RESET DISPLAY\n");
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
                    moveArr2 = convertToLEDarray(moveArr);
                    for (int g = 0; g < 64; ++g) {
                        printf("%d ", moveArr[g]);
                        if((g+1) % 8 == 0){
                            printf("\n");
                        }
                    }
                    printf("\n");
                    displayFromArr(moveArr2);
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
                if(i == currentX && j == currentY && currentTile.rs.value == 1){
                    printf("PLACED BACK DOWN");
                    state = WAITING;
                    reset_Display();
                    //TODO: if placed back on its own tile
                } else if(currentTile.piece == EMPTY && currentTile.rs.value == 1){
                    //Moving piece to empty tile
                    if(moveArr[getIndex(i, j)] == 1){
                        movePiece(currentX, currentY, i, j);

                        free(moveArr);
                        turn = !turn; //change turns
                        currentColour = !currentColour;
                        state = WAITING;
                        reset_Display();
                    } else {
                        state = INVALID_PLACEMENT;
                        displayFromArr(ON);
                        wrongX = i;
                        wrongY = j;
                        printf("INVALID PLACEMENT\n");                   
                        //Not valid placement
                    }
                } else if (currentTile.piece != EMPTY && getColour(currentTile.piece) != currentColour && currentTile.rs.value == 0){
                    state = CAPTURING;
                    capturingX = i;
                    capturingY = j;

                    // //wait until it reads 1 again
                    // //Capturing a piece
                    // if(moveArr[getIndex(i, j)] == 1){
                    //     movePiece(currentX, currentY, i, j);
                    //     free(moveArr);
                    //     turn = !turn; //change turns
                    //     currentColour = !currentColour; //Change colour
                    //     state = WAITING;
                    //     reset_Display();
                    // } else {
                    //     state = INVALID_PLACEMENT;
                    //     printf("INVALID PLACEMENT\n");
                    //     //Not valid placement
                    // }
                } 
            }
        }
        break;
    case CAPTURING:
        {
        TILE temp = getTile(capturingX, capturingY);
        if(temp.rs.value == 1){
            movePiece(currentX, currentY, capturingX, capturingY);
            free(moveArr);
            free(moveArr2);
            turn = !turn;
            currentColour = !currentColour;
            state = WAITING;
            reset_Display();
        }
        break;
        }
    case WRONG_TURN:
        {
        TILE currentTile;
        //TODO: LIGHT UP ALL LEDS AS WARNING
        currentTile = getTile(currentX, currentY);
        if(currentTile.rs.value == 1){
            state = WAITING;
            reset_Display();
        }
        break;
        }
    case INVALID_PLACEMENT:
        {
        displayFromArr(ON);
        TILE tile = getTile(wrongX, wrongY);
        if(tile.rs.value == 0){
            state = PICKED_UP;
            displayFromArr(moveArr2);
        }
        break;
        }
    default:
        exit(1);
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