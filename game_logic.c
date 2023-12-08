#include "board.h"
#include "LED_Matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

static pthread_t game_thread_id;
static pthread_t reed_thread_id;
static pthread_mutex_t game_mutex;
static pthread_mutex_t reed_mutex;
static bool FLAG_CANCEL = false;
void stopThreads(void);

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
    INVALID_CAPTURE,
    CHECK,
    CHECKMATE
} STATE;

#define EMPTY ' '
#define WHITE 0
#define BLACK 1


TURN turn = WHITE_TURN;
int currentColour = WHITE;
STATE state = WAITING;
int* moveArr = NULL;
int* moveArr2 = NULL;
int currentX;
int currentY;
int wrongX;
int wrongY;
int capturingX;
int capturingY;


//Led arrays
const int OFF[8] = {[0 ... 7] = 0b00000000};
const int ON[8*8] = {[0 ... 63] = 0b11111111};

//Runs one iteration of the game update() loop
STATE getState(){
    return state;
}

int read_Button() //reads the value of the gpio pin by opening the file and comparing the read value with 0\n
{
    const int MAX_LENGTH1 = 1024;
    const char* pointer1 = "0\n";
    FILE * f = fopen("/sys/class/gpio/gpio75/value",  "r"); //open file
    if (f == NULL) //if the file cannot be opened exit program and print error.
    {
        printf("ERROR: Unable to open file (%s) for read\n", "/sys/class/gpio/gpio75/value");
        exit(-1);
    }
    sleep(.005);
    char buff[MAX_LENGTH1];
    fgets(buff, MAX_LENGTH1, f); //read content pin file
    sleep(1.0050);
    fclose(f); //close file
    return strcmp(buff, pointer1); //if 0\n is read return 0 otherwise return 1 (gpio pin is active low so 0 = pressed)
}

void gameUpdate(){
    system("clear");
    displayRSValues();
    displayBoard();
    if(turn == WHITE_TURN){
        printf("WHITE TURN\n");
        currentColour = WHITE;
    } else if (turn == BLACK_TURN){
        printf("BLACK TURN\n");
        currentColour = BLACK;
    } else {
        exit(1);
    }
    switch (state){
    case WAITING: //Waits for player to pick up a piece
        printf("WAITING\n");
        for(int i = 0; i < BOARD_SIZE; i++){
            for(int j = 0; j < BOARD_SIZE; j++){
                TILE currentTile = getTile(i, j);
                if(currentTile.piece != EMPTY && getColour(currentTile.piece) == currentColour && currentTile.rs.value == 0){
                    //Piece picked up
                    state = PICKED_UP;
                    currentX = i;
                    currentY = j;
                    if(moveArr != NULL){
                        free(moveArr);
                        moveArr = NULL;
                    }
                    if(moveArr2 != NULL){
                        free(moveArr2);
                        moveArr2 = NULL;
                    }
                    moveArr = getPossibleMoves(currentX, currentY);
                    if(moveArr == NULL){
                        printf("moveArr = NULL");
                        exit(1);
                    }
                    moveArr2 = convertToLEDarray(moveArr);
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
    case PICKED_UP: //player has picked up a piece. Must then find the possible moves.
        printf("PICKED_UP\n");
        for(int i = 0; i < BOARD_SIZE; i++){
            for(int j = 0; j < BOARD_SIZE; j++){
                TILE currentTile = getTile(i, j);
                if(i == currentX && j == currentY && currentTile.rs.value == 1){
                    //IF PIECE IS PLACED BACK DOWN
                    printf("PLACED BACK DOWN");
                    state = WAITING;
                    reset_Display();
                    //if placed back on its own tile
                } else if(currentTile.piece == EMPTY && currentTile.rs.value == 1){
                    //Moving piece to empty tile
                    if(moveArr[getIndex(i, j)] == 1){
                        movePiece(currentX, currentY, i, j);
                        if(isInCheck(!currentColour)){
                            state = CHECK;
                        } else {
                            state = WAITING;
                        }
                        turn = !turn;
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
                    //Capturing piece
                    if(moveArr[getIndex(i, j)] == 1){
                        state = CAPTURING;
                        capturingX = i;
                        capturingY = j;
                    } else { //if it is an invalid capture
                        state = INVALID_CAPTURE;
                        wrongX = i;
                        wrongY = j;
                        
                    }

                } 
            }
        }
        break;
    case CAPTURING: //An opposite colour piece has been picked up (assumed to be a capture)
        {
        printf("CAPTURING\n");
        TILE temp = getTile(capturingX, capturingY);
        if(temp.rs.value == 1){
            movePiece(currentX, currentY, capturingX, capturingY);
            turn = !turn;
            state = WAITING;
            reset_Display();
        }
        break;
        }
    case WRONG_TURN: //Wrong colour piece picked up
        {
        printf("WRONG_TURN\n");
        //LIGHT UP ALL LEDS AS WARNING
        displayFromArr(ON);
        TILE currentTile;
        currentTile = getTile(currentX, currentY);
        if(currentTile.rs.value == 1){
            state = WAITING;
            reset_Display();
        }
        break;
        }
    case INVALID_PLACEMENT: //Placed a piece in the wrong spot
        {
        printf("INVALID_PLACEMENT\n");
        //LIGHT UP ALL LEDS AS WARNING
        displayFromArr(ON);
        TILE tile = getTile(wrongX, wrongY);
        if(tile.rs.value == 0){
            state = PICKED_UP;
            displayFromArr(moveArr2);
        }
        break;
        }
    case INVALID_CAPTURE: //Wrong piece picked up for capture
        {
        printf("INVALID_CAPTURE\n");
        //LIGHT UP ALL LEDS AS WARNING
        displayFromArr(ON);
        TILE tile = getTile(wrongX, wrongY);
        TILE currentTile = getTile(currentX, currentY);
        if(tile.rs.value == 1 && currentTile.rs.value == 0){ //PICKED UP
            state = PICKED_UP;
            displayFromArr(moveArr2);
        } else if (tile.rs.value == 1 && currentTile.rs.value == 1){ //WAITING
            state = WAITING;
            displayFromArr(OFF);
        }
        break;
        }
    case CHECK: //Check if a player is in checkmate
        {
        printf("CHECK\n");
        bool isCheckMate = true;
        int* tempMoves;
        for(int i = 0; i < BOARD_SIZE; i++){
            for(int j = 0; j < BOARD_SIZE; j++){
                TILE currentTile = getTile(i, j);
                if(currentTile.piece != EMPTY && getColour(currentTile.piece) == currentColour){
                    tempMoves = getPossibleMoves(i, j);
                    int numMoves = 0;
                    for(int ii = 0; ii < BOARD_SIZE; ii++){
                        for(int jj = 0; jj < BOARD_SIZE; jj++){
                            if(tempMoves[getIndex(ii, jj)] == 1){
                                numMoves++;
                                if(numMoves > 1){
                                    isCheckMate = false;
                                    break;
                                }
                            }
                        }
                        if(!isCheckMate){
                            break;
                        }
                    }
                    if(!isCheckMate){break;}
                }
                if(!isCheckMate){break;}
            }
            if(!isCheckMate){break;}
        }
        if(isCheckMate){
            state = CHECKMATE;
            stopThreads(); //ends the game
        } else {
            state = WAITING;
        }
        break;
        }
    case CHECKMATE: //Game is over. Close the program
        printf("CHECKMATE\n");
        exit(1);
        break;
    default:
        printf("default\n");
        exit(1);
    }
}

//Updates the hall effect sensors
void reedSwitchUpdate(){
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            TILE currentTile = getTile(i, j);
            setRSValue(i, j, readReedSwitch(currentTile.rs));
        }
    }
}

//frees the global arrays
void freeArrays(){
    if(moveArr != NULL){
        free(moveArr);
    } else if (moveArr2 != NULL){
        free(moveArr2);
    }
}

// function for the thread to run (gameUpdate()) 
static void* gameFunction(void* arg){
    while(1){
        gameUpdate();
        if(FLAG_CANCEL){
            break;
        }
    }
    freeArrays();
    return NULL;
}

//function for the thread to run (reedUpdate())
static void* reedFunction(void* arg){
    while(1){

        reedSwitchUpdate();
        if(FLAG_CANCEL){
            break;
        }
    }
    return NULL;
}

//starts the threads
void startThreads(void){
    reset_Display();
    pthread_create(&reed_thread_id, NULL, reedFunction, NULL); //thread for checking sensor vales
    pthread_mutex_init(&reed_mutex, NULL);
    pthread_create(&game_thread_id, NULL, gameFunction, NULL);// thread for running the game
    pthread_mutex_init(&reed_mutex, NULL);
}

//exits the game
void stopThreads(void){
    reset_Display();
    FLAG_CANCEL = true;
    pthread_mutex_destroy(&reed_mutex);
    pthread_mutex_destroy(&game_mutex);
    exit(1);
}