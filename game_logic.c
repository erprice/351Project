#include "board.h"

typedef enum {
    WHITE_TURN,
    BLACK_TURN,
} TURN;

typedef enum {
    WAITING, //Waiting for piece to be picked up
    PICKED_UP,
} STATE;

#define EMPTY ' '
#define WHITE 0
#define BLACK 1

void gameFunction(){
    TURN turn = WHITE_TURN;
    int currentColour = WHITE;
    STATE state = WAITING;
    int* moveArr;
    int currentX;
    int currentY;
    while(1){
        if(turn == WHITE_TURN){
            currentColour = WHITE;
        } else {
            currentColour = BLACK;
        }
        switch (state){
        case WAITING:
            for(int i = 0; i < BOARD_SIZE; i++){
                for(int j = 0; j < BOARD_SIZE; j++){
                    TILE currentTile = getTile(i, j);
                    if(currentTile.piece != EMPTY && getColour(currentTile.piece) == currentColour && currentTile.rs.value == 0){
                        //Piece picked up
                        state = PICKED_UP;
                        currentX = i;
                        currentY = j;
                        moveArr = getPossibleMoves(currentX, currentY);
                        //light up LEDs
                        break;
                    }
                }
                if(state == PICKED_UP){
                    break;
                }
            }
            break;
        case PICKED_UP:
            //light up LEDS
            for(int i = 0; i < BOARD_SIZE; i++){
                for(int j = 0; j < BOARD_SIZE; j++){
                    TILE currentTile = getTile(i, j);
                    if(currentTile.piece == EMPTY && currentTile.rs.value == 1){
                        //Moving piece to empty tile
                        if(moveArr[getIndex(i, j)] == 1){
                            movePiece(currentX, currentY, i, j);
                            turn = (turn == WHITE_TURN)? BLACK_TURN : WHITE_TURN;//change turns
                            state = WAITING;
                        } else {
                            //Not valid placement
                        }
                    } else if (currentTile.piece != EMPTY && getColour(currentTile.piece) != currentColour && currentTile.rs.value == 0){
                        if(moveArr[getIndex(i, j)] == 1){
                            movePiece(currentX, currentY, i, j);
                            turn = (turn == WHITE_TURN)? BLACK_TURN : WHITE_TURN;//change turns
                            state = WAITING;
                        } else {
                            //Not valid placement
                        }
                    } else if(i == currentX && j == currentY && currentTile.rs.value == 1){
                        state = WAITING;
                        //TODO: if placed back on its own tile
                    }
                }
            }
        }
    }

    /*
                for(int i = 0; i < BOARD_SIZE; i++){
                for(int j = 0; j < BOARD_SIZE; j++){
                    TILE currentTile = getTile(i, j);
                    if(currentTile.piece != EMPTY && getColour(currentTile.piece) == currentColour && currentTile.rs.value == 0){
                        //Piece picked up
                        break;
                    }
                }
            }
    */
}

void reedSwitchUpdate(){
    while (1)
    {
        for(int i = 0; i < BOARD_SIZE; i++){
            for(int j = 0; j < BOARD_SIZE; j++){
                TILE currentTile = getTile(i, j);
                currentTile.rs.value = readReedSwitch(currentTile.rs);
            }
        }
    }
    
}