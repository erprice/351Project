#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

void startThreads();

void stopThreads();

typedef enum {
    WAITING, //Waiting for piece to be picked up
    PICKED_UP,
    INVALID_PLACEMENT,
    WRONG_TURN
} STATE;

void gameUpdate(); //keeps track of all the moves and captures

void reedSwitchUpdate(); //checks the sensor values consistently and update the switch values

STATE getState(); //gets the current state of the game

void freeArrays(); //deallocates dynamic array
#endif