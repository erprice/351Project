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

void gameUpdate();

void reedSwitchUpdate();

STATE getState();

void freeArrays();
#endif