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

static char* pointer = "0\n";
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
static int* getPossiblePawnMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]);
static int* getPossibleRookMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]);
static int* getPossibleBishopMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]);
static int* getPossibleKnightMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]);
static int* getPossibleQueenMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]);
static int* getPossibleKingMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]);
static bool isValid(int x, int y, int colour, TILE board[BOARD_SIZE][BOARD_SIZE]);
static int* getPossibleMovesWithNoCheck(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]);
static bool isKingInCheck(int kingColour, TILE board[BOARD_SIZE][BOARD_SIZE]);
static void copyBoard(TILE copyOfBoard[BOARD_SIZE][BOARD_SIZE], TILE originalBoard[BOARD_SIZE][BOARD_SIZE]);
int getColour(char c);
static bool isEnemy(int x, int y, int colour, TILE board[BOARD_SIZE][BOARD_SIZE]);
static bool isEmpty(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]);
static bool inBounds(int n);
static void initArrToZero(int* arr, int size);
int getKingX(int colour);
int getKingY(int colour);

//initialize the board
void initChessboard(){
    configPin(8, 42, 0); //configure reset button
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

//to test the prototype version conveniently
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

//makes a copy of the chessboard
static void copyBoard(TILE copyOfBoard[BOARD_SIZE][BOARD_SIZE], TILE originalBoard[BOARD_SIZE][BOARD_SIZE]){
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            copyOfBoard[i][j].piece = originalBoard[i][j].piece;
            if(copyOfBoard[i][j].piece == EMPTY){
                printf("0 ");
            } else {
                printf("%c ", copyOfBoard[i][j].piece);
            }
        }
        printf("\n");
    }
}

//cheks if the king is in check
bool isInCheck(int colour){
    return isKingInCheck(colour, board);
}

//Checks a board state and sees if the king is in check
static bool isKingInCheck(int kingColour, TILE board[BOARD_SIZE][BOARD_SIZE]){
    int kingX, kingY;
    bool foundKing = false;
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(board[i][j].piece != EMPTY && getColour(board[i][j].piece) == kingColour && toupper(board[i][j].piece) == KING){
                kingX = i;
                kingY = j;
                foundKing = true;
                break;
            }
        }
        if(foundKing){
            break;
        }
    }
    int* moves;
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            char currentPiece = board[i][j].piece;
            if(currentPiece != EMPTY && getColour(currentPiece) != kingColour){
                moves = getPossibleMovesWithNoCheck(i, j, board);
                if(moves[getIndex(kingX, kingY)] == 1){
                    return true;
                }
                free(moves); //deallocate dynamic memory
            }
        }
    }
    return false;
}

static int* getPossibleMovesWithNoCheck(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]){
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
        return getPossiblePawnMoves(x,y, board);
        break;
    case ROOK:
        return getPossibleRookMoves(x,y, board);
        break;
    case BISHOP:
        return getPossibleBishopMoves(x,y, board);
        break;
    case KNIGHT:
        return getPossibleKnightMoves(x,y, board);
        break;
    case QUEEN:
        return getPossibleQueenMoves(x,y, board);
        break;
    case KING:
        return getPossibleKingMoves(x,y, board);
        break;
    default:
        printf("ERROR: SOMETHING WRONG");
        exit(-1);
        break;
    }
}

int* getPossibleMoves(int x, int y){
    int* moves = getPossibleMovesWithNoCheck(x, y, board);
    int colour = getColour(board[x][y].piece);
    TILE boardCopy[BOARD_SIZE][BOARD_SIZE];
    copyBoard(boardCopy, board);
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(!(i == x && j == y) && moves[getIndex(i, j)] == 1){
                char tempPiece = boardCopy[i][j].piece;
                boardCopy[i][j].piece = boardCopy[x][y].piece;
                boardCopy[x][y].piece = EMPTY;
                if(isKingInCheck(colour, boardCopy)){
                    moves[getIndex(i, j)] = 0;
                }
                boardCopy[x][y].piece = boardCopy[i][j].piece;
                boardCopy[i][j].piece = tempPiece;
            }
        }
    }
    return moves;
}

//reads the value of the sensor
int readReedSwitch(reedSwitch rs){
    return readIntFromFile(rs.filePath);
}

//display the board in 2D format
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

//display the sensor values in 2D format
void displayRSValues(){
    for (int i = 0; i < BOARD_SIZE; i++) {
        for(int j = 0; j < BOARD_SIZE; j++){
            TILE tile = getTile(i, j);
            printf("%d ", tile.rs.value);
        }
        printf("\n");
    }
}

//checks if the piece is picked up
bool isPickedUp(TILE tile){
    if(tile.piece != EMPTY && readReedSwitch(tile.rs) == 0){
        return true;
    } else {
        return false;
    }
}

//checks if the piece is placed down on the board
bool isPlaced(TILE tile){
    if(tile.piece == EMPTY && readReedSwitch(tile.rs) == 1){
        return true;
    } else {
        return false;
    }
}

//sets sensor values manually for testing
void setRSValue(int x, int y, int value){
    board[x][y].rs.value = value;
}

//initialize the sensors
static reedSwitch initReedSwitch(int header_num, int pin_num){
    reedSwitch rs;
    char path[100];
    configPin(header_num, pin_num, 0);
    sprintf(path, "/sys/class/gpio/gpio%d/value", getGPIO(header_num, pin_num));
    rs.filePath = strdup(path);
    printf(rs.filePath);
    rs.value = readIntFromFile(rs.filePath);
    printf("\n");
    return rs;
}

//read integer from the given file
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
    return strcmp(buff, pointer);

}

//get the possible moves of pawn
static int* getPossiblePawnMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]){
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
            if(y+1 < BOARD_SIZE && board[x+1][y+1].piece != EMPTY && getColour(board[x+1][y+1].piece) == WHITE){
                result[getIndex(x+1,y+1)] = 1;
            }
        }
    }

    return result;
}

//get the possible moves of king
static int* getPossibleKingMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]){
    int kingColour = getColour(board[x][y].piece); //black = 1, white = 0
    int* result = (int*)malloc(8 * 8 * sizeof(int)); //8x8 for LED Matrix
    initArrToZero(result, 64);
    result[getIndex(x,y)] = 1;
    //Up
    if(isValid(x-1,y, kingColour, board)){
        result[getIndex(x-1,y)] = 1;
    }
    //Down
    if(isValid(x + 1, y, kingColour, board)){
        result[getIndex(x+1,y)] = 1;
    }
    //Left
    if(isValid(x,y-1, kingColour, board)){
        result[getIndex(x,y-1)] = 1;
    }
    //Right
    if(isValid(x,y+1, kingColour, board)){
        result[getIndex(x,y+1)] = 1;
    }
    //Up/left
    if(isValid(x-1,y-1, kingColour, board)){
        result[getIndex(x-1,y-1)] = 1;
    }
    //Up/Right
    if(isValid(x-1,y+1, kingColour, board)){
        result[getIndex(x-1,y+1)] = 1;
    }
    //Down/Left
    if(isValid(x+1,y-1, kingColour, board)){
        result[getIndex(x+1,y-1)] = 1;
    }
    //Down/Right
    if(isValid(x+1,y+1, kingColour, board)){
        result[getIndex(x+1,y+1)] = 1;
    }
    
    return result;
}

//initialize the passed array to all 0s
static void initArrToZero(int* arr, int size){
    for(int i = 0; i < size; i++){
        arr[i] = 0;
    }
}

//get the possible moves of rook
static int* getPossibleRookMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]){
    int rookColour = getColour(board[x][y].piece); //black = 1, white = 0
    int* result = (int*)malloc(8 * 8 * sizeof(int));
    initArrToZero(result, 64);
    result[getIndex(x,y)] = 1;
    //check where it can move to the right

    for(int right_y = y+1; right_y < BOARD_SIZE; right_y++){
        if(isEmpty(x, right_y, board)){
            result[getIndex(x,right_y)] = 1;
        } else if (isEnemy(x, right_y, rookColour, board)){
            result[getIndex(x, right_y)] = 1;
            break;
        } else {
            break;
        }
    }

    //check where it can move to the left
    for(int left_y = y-1; left_y >= 0; left_y--){
        if(isEmpty(x, left_y, board)){
            result[getIndex(x,left_y)] = 1;
        } else if (isEnemy(x, left_y, rookColour, board)){
            result[getIndex(x, left_y)] = 1;
            break;
        } else {
            break;
        }
    }

    //check where it can move up
    for(int up_x = x-1; up_x >= 0; up_x--){
        if(isEmpty(up_x, y, board)){
            result[getIndex(up_x,y)] = 1;
        } else if (isEnemy(up_x, y, rookColour, board)){
            result[getIndex(up_x, y)] = 1;
            break;
        } else {
            break;
        }
    }

    //check where it can move down
    for(int down_x = x+1; down_x < BOARD_SIZE; down_x++){
        if(isEmpty(down_x, y, board)){
            result[getIndex(down_x,y)] = 1;
        } else if (isEnemy(down_x, y, rookColour, board)){
            result[getIndex(down_x, y)] = 1;
            break;
        } else {
            break;
        }    
    }
    return result;
}

//get the possible moves of bishop
static int* getPossibleBishopMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]){
    int bishopColour = getColour(board[x][y].piece); //black = 1, white = 0
    int* result = (int*)malloc(8 * 8 * sizeof(int));
    initArrToZero(result, 64);
    result[getIndex(x,y)] = 1;
    int i,j;
    //check the NE diagonal
    for(i = x-1, j = y+1; i >= 0 && j < BOARD_SIZE; i--, j++){
        if(isEmpty(i,j, board)){
            result[getIndex(i,j)] = 1;
        } else if (isEnemy(i,j, bishopColour, board)){
            result[getIndex(i,j)] = 1;
            break;
        } else {
            break;
        }
    }

    //check the NW diagonal
    for(i = x-1, j = y-1; i >= 0 && j >= 0; i--, j--){
        if(isEmpty(i,j, board)){
            result[getIndex(i,j)] = 1;
        } else if (isEnemy(i,j, bishopColour, board)){
            result[getIndex(i,j)] = 1;
            break;
        } else {
            break;
        }
    }

    //check the SE diagonal
    for(i = x+1, j = y+1; i < BOARD_SIZE && j < BOARD_SIZE; i++, j++){
        if(isEmpty(i,j, board)){
            result[getIndex(i,j)] = 1;
        } else if (isEnemy(i,j, bishopColour, board)){
            result[getIndex(i,j)] = 1;
            break;
        } else {
            break;
        }
    }

    //check the SW diagonal
    for(i = x+1, j = y-1; i < BOARD_SIZE && j >= 0; i++, j--){
        if(isEmpty(i,j, board)){
            result[getIndex(i,j)] = 1;
        } else if (isEnemy(i,j, bishopColour, board)){
            result[getIndex(i,j)] = 1;
            break;
        } else {
            break;
        }
    }

    return result;
}

//get the possible moves of queen
static int* getPossibleQueenMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]){
    int* combinedArray = (int*)malloc(8 * 8 * sizeof(int));
    initArrToZero(combinedArray, 64);
    int* bishopArray = getPossibleBishopMoves(x,y, board);
    int* rookArray = getPossibleRookMoves(x,y, board);

    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            int ind = getIndex(i,j);
            combinedArray[ind] = (bishopArray[ind] || rookArray[ind]);
        }
    }

    //free dynamically allocate arrays
    free(bishopArray);
    free(rookArray);
    return combinedArray;
}

//get the possible moves of knight
static int* getPossibleKnightMoves(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]){
    int knightColour = getColour(board[x][y].piece); //black = 1, white = 0
    int* result = (int*)malloc(8 * 8 * sizeof(int));
    initArrToZero(result, 64);
    result[getIndex(x,y)] = 1;

    if(isValid(x-1, y-2, knightColour, board)){
        result[getIndex(x-1, y-2)] = 1;
    }
    if(isValid(x-1, y+2, knightColour, board)){
        result[getIndex(x-1,y+2)] = 1;
    }
    if(isValid(x-2,y-1,knightColour, board)){
        result[getIndex(x-2,y-1)] = 1;
    }
    if(isValid(x-2,y+1,knightColour, board)){
        result[getIndex(x-2,y+1)] = 1;
    }
    if(isValid(x+1,y-2,knightColour, board)){
        result[getIndex(x+1,y-2)] = 1;
    }
    if(isValid(x+1,y+2,knightColour, board)){
        result[getIndex(x+1,y+2)] = 1;
    }
    if(isValid(x+2,y-1,knightColour, board)){
        result[getIndex(x+2,y-1)] = 1;
    }
    if(isValid(x+2,y+1,knightColour, board)){
        result[getIndex(x+2,y+1)] = 1;
    }
    return result;
}

//manually places pieces for testing in a 3 x 3 board
static void placePieces3x3(){
    char pieceList[3*3] = {
        'K', ' ', ' ',
        ' ', ' ', ' ',
        ' ', ' ', 'k'
    };
    int k = 0;

    whiteKingX = 0;
    whiteKingY = 0;
    blackKingX = 2;
    blackKingY = 2;

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

//initialize pieces in a 5 x 5 board
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
    displayBoard();
}

//checks if the move is valid
static bool isValid(int x, int y, int colour, TILE board[BOARD_SIZE][BOARD_SIZE]){
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

//checks if the board is empty
static bool isEmpty(int x, int y, TILE board[BOARD_SIZE][BOARD_SIZE]){
    if(x < 0 || x > BOARD_SIZE - 1 || y < 0 || y > BOARD_SIZE - 1){
        return false;
    } else if (board[x][y].piece == EMPTY){
        return true;
    } else {
        return false;
    }
}
 
//checks if the piece is a valid enemy
static bool isEnemy(int x, int y, int colour, TILE board[BOARD_SIZE][BOARD_SIZE]){
    if(x < 0 || x > BOARD_SIZE - 1 || y < 0 || y > BOARD_SIZE - 1){
        return false;
    } else if (getColour(board[x][y].piece) != colour){
        return true;
    } else {
        return false;
    }
}

//get the colour of the piece
int getColour(char c){
    //Uppercase = WHITE, Lowercase = BLACK 
    char capsC = toupper(c);
    if(capsC != PAWN && capsC != ROOK && capsC != BISHOP && capsC != QUEEN && capsC != KNIGHT && capsC != KING){
        printf("WARNING: getColour called on non-piece: %c\n", c);
        exit(-1);
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

//generates the LED array to display on the board
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

//gets the tile using coordinates
TILE getTile(int x, int y){
    if(x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE){
        return board[x][y];
    } else {
        printf("ERROR: OUT OF BOUNDS");
        exit(-1);
    }
}

//move a piece to a certain tile
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

//checks if nummber is within the board size 
static bool inBounds(int n){
    if(n >= 0 && n < BOARD_SIZE){
        return true;
    } else {
        return false;
    }
}

//get the x coordinate of king
int getKingX(int colour){
    if(colour == WHITE){
        return whiteKingX;
    } else if (colour == BLACK) {
        return blackKingX;
    } else {
        printf("ERROR");
        exit(1);
    }
}

//get the y coordinate of king
int getKingY(int colour){
    if(colour == WHITE){
        return whiteKingY;
    } else if (colour == BLACK) {
        return blackKingY;
    } else {
        printf("ERROR");
        exit(1);
    }
}

