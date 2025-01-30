#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define MAX_CHILDREN 5
#define MAX_DEPTH 5

typedef struct {
    Position cur_board; // Game board
    int currentPlayer; // 1 or 0 to indicate current player / 1 is black and 0 is white
    Move lastMove;
} BState;

typedef struct Node{
    BState cur_bstate;
    struct Node* children[MAX_CHILDREN];
    int num_of_children;
    int cur_evaluation;
} Node;

Node* create_node(BState* curState);

int evaluateBoard(BState* state);

Move findBestMove(Node* root);

Move findBestMoveab(Node* root);