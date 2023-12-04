#include "board.h"
#include "config.h"
#include "pinMap.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WHITE 0
#define BLACK 1

char* pointer = "0\n";
static int whiteKingX, whiteKingY;
static int blackKingX, blackKingY;

static TILE board[BOARD_SIZE][BOARD_SIZE];

typedef enum {
    EMPTY = ' ',
    PAWN = 'P',
    ROOK = 'R',
    BISHOP = 'B',
    KNIGHT = 'N',
    QUEEN = 'Q',
    KING = 'K',
} Piece;

//helper functions
static reedSwitch initReedSwitch(int header_num, int pin_num);
static void placePieces5x5();
static void placePieces3x3();
static int readIntFromFile(char* filePath);
static int* getPossiblePawnMoves(int x, int y);
static int* getPossibleRookMoves(int x, int y);
static int* getPossibleBishopMoves(int x, int y);
static int* getPossibleKnightMoves(int x, int y);
static int* getPossibleQueenMoves(int x, int y);
static int* getPossibleKingMoves(int x, int y);
static bool isValid(int x, int y, int colour);
int getColour(char c);
static bool isEnemy(int x, int y, int colour);
static bool isEmpty(int x, int y);
static bool inBounds(int n);
static void initArrToZero(int* arr, int size);
int getKingX(int colour);
int getKingY(int colour);

void initChessboard(){
    int n = 0;
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            board[i][j].piece = EMPTY;
            board[i][j].rs = initReedSwitch(GPIO_pinList[n].header_num, GPIO_pinList[n].pin_num);
            //printf("p%d.%d\n setup", GPIO_pinList[n].header_num, GPIO_pinList[n].pin_num);
            n++;
        }
    }
    if(BOARD_SIZE == 3){
        placePieces3x3();
    }
    if(BOARD_SIZE == 5){
        placePieces5x5();
    }
}

void initChessboardForTesting(){
    //Uppercase = WHITE, Lowercase = BLACK
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            board[i][j].piece = EMPTY;
        }
    }
    
    if(BOARD_SIZE == 3){
        placePieces3x3();
    }
    if(BOARD_SIZE == 5){
        placePieces5x5();
    }
}

int* getPossibleMoves(int x, int y){
    if(x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE){
        printf("ERROR: INDEX IS OUT OF BOUNDS");
        exit(-1);
    }
    switch (toupper(board[x][y].piece))
    {
    case EMPTY:
        printf("ERROR: EMPTY TILE");
        exit(-1);
        break;
    case PAWN:
        return getPossiblePawnMoves(x,y);
        break;
    case ROOK:
        return getPossibleRookMoves(x,y);
        break;
    case BISHOP:
        return getPossibleBishopMoves(x,y);
        break;
    case KNIGHT:
        return getPossibleKnightMoves(x,y);
        break;
    case QUEEN:
        return getPossibleQueenMoves(x,y);
        break;
    case KING:
        return getPossibleKingMoves(x,y);
        break;
    default:
        printf("ERROR: SOMETHING WRONG");
        exit(-1);
        break;
    }
}

int readReedSwitch(reedSwitch rs){
    return readIntFromFile(rs.filePath);
}

void displayBoard(){
    for (int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++){
            char piece = board[i][j].piece;
            if(piece == EMPTY){
                piece = '0';
            }
            printf("%c ", piece);
        }
        printf("\n");
    }
}

void displayRSValues(){
    for (int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++){
            TILE tile = getTile(i, j);
            printf("%d ", tile.rs.value);
        }
        printf("\n");
    }
}

bool isPickedUp(TILE tile){
    if(tile.piece != EMPTY && readReedSwitch(tile.rs) == 0){
        return true;
    } else {
        return false;
    }
}

bool isPlaced(TILE tile){
    if(tile.piece == EMPTY && readReedSwitch(tile.rs) == 1){
        return true;
    } else {
        return false;
    }
}


void setRSValue(int x, int y, int value){
    board[x][y].rs.value = value;
}

static reedSwitch initReedSwitch(int header_num, int pin_num){
    reedSwitch rs;
    char path[100];
    configPin(header_num, pin_num, 0);
    sprintf(path, "/sys/class/gpio/gpio%d/value", getGPIO(header_num, pin_num));
    rs.filePath = strdup(path);
    printf(rs.filePath);
    // rs.value = 0;
    rs.value = readIntFromFile(rs.filePath);
    printf("\n");
    return rs;
}

static int readIntFromFile(char* filePath){

    FILE *pFile = fopen(filePath, "r");
    //int num;
    if (pFile == NULL) {
        printf("ERROR: Unable to open file (%s) for read\n", filePath);
        exit(-1);
    }
    char buff[100];
    fgets(buff, 100, pFile);
    fclose(pFile);
    //printf(filePath);
    return strcmp(buff, pointer);

    /*
    if (fscanf(pFile, "%d", &num) == 1) {
        // Successfully read an integer from the file
        fclose(pFile);

        return num;
    } else {
        // Failed to read an integer
        fprintf(stderr, "Error reading integer from the file.\n");
        exit(-1);
    }
    */
}

static int* getPossiblePawnMoves(int x, int y){
    int pawnColour = getColour(board[x][y].piece); //black = 1, white = 0
    int* result = (int*)malloc(8 * 8 * sizeof(int)); //8x8 for LED Matrix
    initArrToZero(result, 64);
    result[getIndex(x,y)] = 1;

    if(pawnColour == WHITE){
        if(x-1 >= 0){
            //Check space above
            if(board[x-1][y].piece == EMPTY){
                result[getIndex(x-1,y)] = 1;
            }
            //Check NW tile
            if(y-1 >= 0 && board[x-1][y-1].piece != EMPTY && getColour(board[x-1][y-1].piece) == BLACK){
                result[getIndex(x-1,y-1)] = 1;
            }
            //Check NE tile
            if(y+1 < BOARD_SIZE && board[x-1][y+1].piece != EMPTY && getColour(board[x-1][y+1].piece) == BLACK){
                result[getIndex(x-1,y+1)] = 1;
            }
        }
    }

    if(pawnColour == BLACK){
        if(x+1 < BOARD_SIZE){
            //Check space below
            if(board[x+1][y].piece == EMPTY){
                result[getIndex(x+1,y)] = 1;
            }
            //Check SW tile
            if(y-1 >= 0 && board[x+1][y-1].piece != EMPTY && getColour(board[x+1][y-1].piece) == WHITE){
                result[getIndex(x+1,y-1)] = 1;
            }
            //Check SE tile
            //printf("%d\n", board[x+1][y+1].piece);
            //printf("%d\n", getColour(board[x+1][y+1].piece));
            //printf("%d\n", getColour(board[x+1][y+1].piece) == WHITE);
            //(y+1 < BOARD_SIZE && board[x+1][y+1].piece != EMPTY && getColour(board[x+1][y+1].piece == WHITE));
            if(y+1 < BOARD_SIZE && board[x+1][y+1].piece != EMPTY && getColour(board[x+1][y+1].piece) == WHITE){
                result[getIndex(x+1,y+1)] = 1;
            }
        }
    }

    return result;
}

static int* getPossibleKingMoves(int x, int y){
    int kingColour = getColour(board[x][y].piece); //black = 1, white = 0
    int* result = (int*)malloc(8 * 8 * sizeof(int)); //8x8 for LED Matrix
    initArrToZero(result, 64);
    result[getIndex(x,y)] = 1;
    //Up
    if(isValid(x-1,y, kingColour)){
        result[getIndex(x-1,y)] = 1;
    }
    //Down
    if(isValid(x + 1, y, kingColour)){
        result[getIndex(x+1,y)] = 1;
    }
    //Left
    if(isValid(x,y-1, kingColour)){
        result[getIndex(x,y-1)] = 1;
    }
    //Right
    if(isValid(x,y+1, kingColour)){
        result[getIndex(x,y+1)] = 1;
    }
    //Up/left
    if(isValid(x-1,y-1, kingColour)){
        result[getIndex(x-1,y-1)] = 1;
    }
    //Up/Right
    if(isValid(x-1,y+1, kingColour)){
        result[getIndex(x-1,y+1)] = 1;
    }
    //Down/Left
    if(isValid(x+1,y-1, kingColour)){
        result[getIndex(x+1,y-1)] = 1;
    }
    //Down/Right
    if(isValid(x+1,y+1, kingColour)){
        result[getIndex(x+1,y+1)] = 1;
    }
    
    return result;
}

static void initArrToZero(int* arr, int size){
    for(int i = 0; i < size; i++){
        arr[i] = 0;
    }
}

static int* getPossibleRookMoves(int x, int y){
    int rookColour = getColour(board[x][y].piece); //black = 1, white = 0
    int* result = (int*)malloc(8 * 8 * sizeof(int));
    initArrToZero(result, 64);
    result[getIndex(x,y)] = 1;
    //check where it can move to the right

    for(int right_y = y+1; right_y < BOARD_SIZE; right_y++){
        if(isEmpty(x, right_y)){
            result[getIndex(x,right_y)] = 1;
        } else if (isEnemy(x, right_y, rookColour)){
            result[getIndex(x, right_y)] = 1;
            break;
        } else {
            break;
        }
    }

    //check where it can move to the left
    for(int left_y = y-1; left_y >= 0; left_y--){
        if(isEmpty(x, left_y)){
            result[getIndex(x,left_y)] = 1;
        } else if (isEnemy(x, left_y, rookColour)){
            result[getIndex(x, left_y)] = 1;
            break;
        } else {
            break;
        }
    }

    //check where it can move up
    for(int up_x = x-1; up_x >= 0; up_x--){
        if(isEmpty(up_x, y)){
            result[getIndex(up_x,y)] = 1;
        } else if (isEnemy(up_x, y, rookColour)){
            result[getIndex(up_x, y)] = 1;
            break;
        } else {
            break;
        }
    }

    //check where it can move down
    for(int down_x = x+1; down_x < BOARD_SIZE; down_x++){
        if(isEmpty(down_x, y)){
            result[getIndex(down_x,y)] = 1;
        } else if (isEnemy(down_x, y, rookColour)){
            result[getIndex(down_x, y)] = 1;
            break;
        } else {
            break;
        }    
    }
    return result;
}

static int* getPossibleBishopMoves(int x, int y){
    int bishopColour = getColour(board[x][y].piece); //black = 1, white = 0
    int* result = (int*)malloc(8 * 8 * sizeof(int));
    initArrToZero(result, 64);
    result[getIndex(x,y)] = 1;
    int i,j;
    //check the NE diagonal
    for(i = x-1, j = y+1; i >= 0 && j < BOARD_SIZE; i--, j++){
        if(isEmpty(i,j)){
            result[getIndex(i,j)] = 1;
        } else if (isEnemy(i,j, bishopColour)){
            result[getIndex(i,j)] = 1;
            break;
        } else {
            break;
        }
    }

    //check the NW diagonal
    for(i = x-1, j = y-1; i >= 0 && j >= 0; i--, j--){
        if(isEmpty(i,j)){
            result[getIndex(i,j)] = 1;
        } else if (isEnemy(i,j, bishopColour)){
            result[getIndex(i,j)] = 1;
            break;
        } else {
            break;
        }
    }

    //check the SE diagonal
    for(i = x+1, j = y+1; i < BOARD_SIZE && j < BOARD_SIZE; i++, j++){
        if(isEmpty(i,j)){
            result[getIndex(i,j)] = 1;
        } else if (isEnemy(i,j, bishopColour)){
            result[getIndex(i,j)] = 1;
            break;
        } else {
            break;
        }
    }

    //check the SW diagonal
    for(i = x+1, j = y-1; i < BOARD_SIZE && j >= 0; i++, j--){
        if(isEmpty(i,j)){
            result[getIndex(i,j)] = 1;
        } else if (isEnemy(i,j, bishopColour)){
            result[getIndex(i,j)] = 1;
            break;
        } else {
            break;
        }
    }

    return result;
}

static int* getPossibleQueenMoves(int x, int y){
    int* combinedArray = (int*)malloc(8 * 8 * sizeof(int));
    initArrToZero(combinedArray, 64);
    int* bishopArray = getPossibleBishopMoves(x,y);
    int* rookArray = getPossibleRookMoves(x,y);

    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            int ind = getIndex(i,j);
            combinedArray[ind] = (bishopArray[ind] || rookArray[ind]);
        }
    }

    free(bishopArray);
    free(rookArray);
    return combinedArray;
}

static int* getPossibleKnightMoves(int x, int y){
    int knightColour = getColour(board[x][y].piece); //black = 1, white = 0
    int* result = (int*)malloc(8 * 8 * sizeof(int));
    initArrToZero(result, 64);
    result[getIndex(x,y)] = 1;

    if(isValid(x-1, y-2, knightColour)){
        result[getIndex(x-1, y-2)] = 1;
    }
    if(isValid(x-1, y+2, knightColour)){
        result[getIndex(x-1,y+2)] = 1;
    }
    if(isValid(x-2,y-1,knightColour)){
        result[getIndex(x-2,y-1)] = 1;
    }
    if(isValid(x-2,y+1,knightColour)){
        result[getIndex(x-2,y+1)] = 1;
    }
    if(isValid(x+1,y-2,knightColour)){
        result[getIndex(x+1,y-2)] = 1;
    }
    if(isValid(x+1,y+2,knightColour)){
        result[getIndex(x+1,y+2)] = 1;
    }
    if(isValid(x+2,y-1,knightColour)){
        result[getIndex(x+2,y-1)] = 1;
    }
    if(isValid(x+2,y+1,knightColour)){
        result[getIndex(x+2,y+1)] = 1;
    }
    return result;
}
static void placePieces3x3(){
    char pieceList[3*3] = {
        'Q', ' ', ' ',
        ' ', ' ', 'k',
        ' ', ' ', ' '
    };
    int k = 0;

    for (int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            if(pieceList[k] == EMPTY){
                board[i][j].rs.value = 0;
            } else {
                board[i][j].rs.value = 1;
            }
            board[i][j].piece = pieceList[k];
            k++;
        }
    }
}

static void placePieces5x5(){
    char pieceList[5*5] = {
        'r', 'n', 'b', 'q', 'k',
        'p', 'p', 'p', 'p', 'p',
        ' ', ' ', ' ', ' ', ' ',
        'P', 'P', 'P', 'P', 'P',
        'R', 'N', 'B', 'Q', 'K'
    };
    int k = 0;
    whiteKingX = 4;
    whiteKingY = 4;
    blackKingX = 0;
    blackKingY = 4;

    for (int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            if(pieceList[k] == EMPTY){
                board[i][j].rs.value = 0;
            } else {
                board[i][j].rs.value = 1;
            }
            board[i][j].piece = pieceList[k];
            k++;
        }
    }
}

static bool isValid(int x, int y, int colour){
    if(x < 0 || x > BOARD_SIZE - 1 || y < 0 || y > BOARD_SIZE - 1){
        return false;
    } else if (board[x][y].piece == EMPTY){
        return true;
    } else if (getColour(board[x][y].piece) != colour){
        return true;
    } else {
        return false;
    }
}

static bool isEmpty(int x, int y){
    if(x < 0 || x > BOARD_SIZE - 1 || y < 0 || y > BOARD_SIZE - 1){
        return false;
    } else if (board[x][y].piece == EMPTY){
        return true;
    } else {
        return false;
    }
}

static bool isEnemy(int x, int y, int colour){
    if(x < 0 || x > BOARD_SIZE - 1 || y < 0 || y > BOARD_SIZE - 1){
        return false;
    } else if (getColour(board[x][y].piece) != colour){
        return true;
    } else {
        return false;
    }
}

int getColour(char c){
    //Uppercase = WHITE, Lowercase = BLACK 
    char capsC = toupper(c);
    if(capsC != PAWN && capsC != ROOK && capsC != BISHOP && capsC != QUEEN && capsC != KNIGHT && capsC != KING){
        printf("WARNING: getColour called on non-piece");
    }
    if(islower(c)){
        return BLACK;
    } else {
        return WHITE;
    }
}

int getIndex(int x, int y){
    return x*8 + y;
}

int* convertToLEDarray(int* array){
    int* binaryArray = (int*)malloc(8 * sizeof(int));
    for(int i = 0; i < 8; i++){
        int result = 0;
        for(int j = i*8; j < i*8 + 8; j++){
            result = (result << 1) | array[j];
        }
        binaryArray[i] = result;
    }
    return binaryArray;
}

TILE getTile(int x, int y){
    if(x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE){
        return board[x][y];
    } else {
        printf("ERROR: OUT OF BOUNDS");
        exit(-1);
    }
}

void movePiece(int x, int y, int x2, int y2){
    if(board[x][y].piece == EMPTY){
        printf("ERROR: CANNOT MOVE EMPTY TILE");
        exit(-1);
    }
    if(inBounds(x) && inBounds(y) && inBounds(x2) && inBounds(y2)){
        if(toupper(board[x][y].piece) == KING){
            if(getColour(board[x][y].piece) == WHITE){
                whiteKingX = x2;
                whiteKingY = y2;
            } else if (getColour(board[x][y].piece) == BLACK){
                blackKingX = x2;
                blackKingY = y2;
            } else {
                exit(1);
            }
        }
        board[x2][y2].piece = board[x][y].piece;
        board[x][y].piece = EMPTY;
    } 
}

static bool inBounds(int n){
    if(n >= 0 && n < BOARD_SIZE){
        return true;
    } else {
        return false;
    }
}

int getKingX(int colour){
    if(colour == WHITE){
        return whiteKingX;
    } else if (colour == BLACK) {
        return blackKingX;
    }
}

int getKingY(int colour){
    if(colour == WHITE){
        return whiteKingY;
    } else if (colour == BLACK) {
        return blackKingY;
    }
}
/*
read the switches
if ANY tile is picked up
then check for 2 cases:
    ANOTHER tile is picked up -> we are capturing:
        wait until that tile reads 1
        move the piece to that spot
    OR ANOTHER tile is placed -> we are moving to an empty spot:
        then move the piece to that spot

*/

