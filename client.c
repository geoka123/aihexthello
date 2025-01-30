#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "client.h"

/**********************************************************/
Position gamePosition;		// Position we are going to use

Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

char myColor;				// to store our color
int mySocket;				// our socket
char msg;					// used to store the received message

int alphaBeta = 0; // 0 -> no pruning, 1 -> pruning on

BState* curState;

char * agentName = "MyAgent!";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)
/**********************************************************/


int main( int argc, char ** argv )
{
	int c;
	opterr = 0;

	while( ( c = getopt ( argc, argv, "i:p:h" ) ) != -1 )
		switch( c )
		{
			case 'h':
				printf( "[-i ip] [-p port]\n" );
				return 0;
			case 'i':
				ip = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case '?':
				if( optopt == 'i' || optopt == 'p' )
					printf( "Option -%c requires an argument.\n", ( char ) optopt );
				else if( isprint( optopt ) )
					printf( "Unknown option -%c\n", ( char ) optopt );
				else
					printf( "Unknown option character -%c\n", ( char ) optopt );
				return 1;
			default:
			return 1;
		}

	connectToTarget( port, ip, &mySocket );

/**********************************************************/
// used in random
	srand( time( NULL ) );
	int i, j;
/**********************************************************/

	while( 1 )
	{

		msg = recvMsg( mySocket );

		switch ( msg )
		{
			case NM_REQUEST_NAME:		//server asks for our name
				sendName( agentName, mySocket );
				break;

			case NM_NEW_POSITION:		//server is trying to send us a new position
				getPosition( &gamePosition, mySocket );
				printPosition( &gamePosition );
				break;

			case NM_COLOR_W:			//server informs us that we have WHITE color
				myColor = WHITE;
				break;

			case NM_COLOR_B:			//server informs us that we have BLACK color
				myColor = BLACK;
				break;

			case NM_PREPARE_TO_RECEIVE_MOVE:	//server informs us that he will now send us opponent's move
				getMove( &moveReceived, mySocket );
				moveReceived.color = getOtherSide( myColor );
				doMove( &gamePosition, &moveReceived );		//play opponent's move on our position
				printPosition( &gamePosition );
				break;

			case NM_REQUEST_MOVE:		//server requests our move
				myMove.color = myColor;


				if( !canMove( &gamePosition, myColor ) )
				{
					myMove.tile[ 0 ] = NULL_MOVE;		// we have no move ..so send null move
				}
				else
				{


/**********************************************************/
// random player - not the most efficient implementation
				curState = (BState*)malloc(sizeof(BState));
				curState->cur_board = gamePosition;
				curState->currentPlayer = (myColor == BLACK) ? 1 : 0;
				curState->lastMove = moveReceived;
				
				Node* root = create_node(&curState);

				generateChildren(root);

				if (alphaBeta == 1)
					myMove = findBestMoveab(root);
				else
					myMove = findBestMove(root)

				if (!isLegalMove(&gamePosition, &myMove)) {
					fprintf("%s", "qifsha ropt");
					exit(EXIT_FAILURE);
				}

// end of random
/**********************************************************/

				}

				sendMove( &myMove, mySocket );			//send our move
				doMove( &gamePosition, &myMove );		//play our move on our position
				printPosition( &gamePosition );
				break;

			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
				return 0;
		}

	} 

	return 0;
}

Node* create_node(BState* curState) {
	Node* newNode = (Node*)malloc(sizeof(Node));
	
	if (newNode == NULL){
		fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
	}

	newNode->cur_bstate = *curState;
	newNode->cur_evaluation = 0;
	newNode->num_of_children = 0;

	for (int i=0; i<MAX_CHILDREN; i++) {
		newNode->children[i] = NULL;
	}

	return newNode;
}

void generateChildren(Node* root) {
    for (int i = 0; i < ARRAY_BOARD_SIZE; i++) {
        for (int j = 0; j < ARRAY_BOARD_SIZE; j++) {
            // Check if the cell is empty and a legal move
            if (root->cur_bstate.cur_board.board[i][j] == '2') {
                Move move;
                move.tile[0] = i;
                move.tile[1] = j;
                move.color = (root->cur_bstate.currentPlayer == 0) ? 0 : 1;  // Assign the current player's color

                // Validate if this move is legal
                if (isLegalMove(&root->cur_bstate.cur_board, &move)){
					if (&root->num_of_children < MAX_CHILDREN) {
						// Copy the current state
						BState newState = root->cur_bstate;

						// Apply the move
						newState.cur_board.board[i][j] = (root->cur_bstate.currentPlayer == 0) ? '0' : '1';

						// Switch turn
						newState.currentPlayer = (root->cur_bstate.currentPlayer == 0) ? 1 : 0;

						// Store the move position
						newState.lastMove = move;

						// Create a new child node
						root->children[root->num_of_children++] = create_node(&newState);
					}
                }
            }
        }
    }
}

void freeTree(Node* root) {
    for (int i = 0; i < root->num_of_children; i++) {
        freeTree(root->children[i]);
    }
	free(&root->cur_bstate);
    free(root);

}

int evaluateBoard(BState* state){
	int state_to_return = 0;
	int black_count = 0;
	int white_count = 0;

	for (int i=0; i<ARRAY_BOARD_SIZE; i++){
		for (int j=0; j<ARRAY_BOARD_SIZE; j++) {
			if (state->cur_board.board[i][j] == 1)
				black_count++;
			else if (state->cur_board.board[i][j] == 0)
				white_count++;
		}
	}

	if (myColor == BLACK)
		state_to_return = black_count - white_count;
	else if (myColor == WHITE)
		state_to_return = white_count - black_count;
	
	return state_to_return;
}

int minimaxab(Node* node, int depth, int alpha, int beta, int isMaximizing) {
    if (depth == 0) {
        return evaluateBoard(&node->cur_bstate);
    }

    // Generate children only once per node
    if (node->num_of_children == 0) {
        generateChildren(node);
    }

    // If no children exist after generation, return evaluation (no valid moves)
    if (node->num_of_children == 0) {
        return evaluateBoard(&node->cur_bstate);
    }

    if (isMaximizing) {
        int maxEval = -10000;  // Negative infinity
        for (int i = 0; i < node->num_of_children; i++) {
            int eval = minimaxab(node->children[i], depth - 1, alpha, beta, 0);
            if (eval > maxEval) {
                maxEval = eval;
            }
            // Alpha-beta pruning
            if (eval > alpha) {
                alpha = eval;
            }
            if (beta <= alpha) {
                break;  // Beta cut-off (Pruning)
            }
        }
        return maxEval;
    } else {
        int minEval = 10000;  // Positive infinity
        for (int i = 0; i < node->num_of_children; i++) {
            int eval = minimaxab(node->children[i], depth - 1, alpha, beta, 1);
            if (eval < minEval) {
                minEval = eval;
            }
            // Alpha-beta pruning
            if (eval < beta) {
                beta = eval;
            }
            if (beta <= alpha) {
                break;  // Alpha cut-off (Pruning)
            }
        }
        return minEval;
    }
}


int minimax(Node* node, int depth, int isMaximizing) {
    if (depth == 0) {
        return evaluateBoard(&node->cur_bstate);
    }

    // Only generate children once per node
    if (node->num_of_children == 0) {
        generateChildren(node);
    }

    // If still no children after generation, return evaluation (no valid moves)
    if (node->num_of_children == 0) {
        return evaluateBoard(&node->cur_bstate);
    }

    // If maximizing player
    if (isMaximizing) {
        int bestEval = -10000;  // Negative infinity
        for (int i = 0; i < node->num_of_children; i++) {
            int eval = minimax(node->children[i], depth - 1, 0);
            if (eval > bestEval) {
                bestEval = eval;
            }
            if (bestEval == 10000) break;  // Prune if best move found
        }
        return bestEval;
    }
    // If minimizing player
    else {
        int bestEval = 10000;  // Positive infinity
        for (int i = 0; i < node->num_of_children; i++) {
            int eval = minimax(node->children[i], depth - 1, 1);
            if (eval < bestEval) {
                bestEval = eval;
            }
            if (bestEval == -10000) break;  // Prune if worst move found
        }
        return bestEval;
    }

}

Move findBestMove(Node* root) {
    Move bestMove;
    bestMove.tile[0] = -1;  // Default invalid move
    bestMove.tile[1] = -1;
    bestMove.color = myColor;

    if (root->num_of_children == 0) {
        return bestMove;  // No valid moves available
    }

    int bestValue = -10000;  // Negative infinity

    for (int i = 0; i < root->num_of_children; i++) {
        int moveValue = minimax(root->children[i], MAX_DEPTH, 0);
        
        if (moveValue > bestValue) {
            bestValue = moveValue;
            bestMove = root->children[i]->cur_bstate.lastMove;  // Copy lastMove directly
        }
    }

    return bestMove;
}

Move findBestMoveab(Node* root) {
    Move bestMove;
    bestMove.tile[0] = -1;  // Default invalid move
    bestMove.tile[1] = -1;
    bestMove.color = myColor;

    // Ensure children are generated
    if (root->num_of_children == 0) {
        generateChildren(root);
    }

    // If still no children after generation, return an invalid move
    if (root->num_of_children == 0) {
        return bestMove;
    }

    int bestValue = -10000;  // Negative infinity
    int alpha = -10000, beta = 10000;

    for (int i = 0; i < root->num_of_children; i++) {
        int moveValue = minimaxab(root->children[i], MAX_DEPTH, alpha, beta, 0);

        if (moveValue > bestValue) {
            bestValue = moveValue;
            bestMove = root->children[i]->cur_bstate.lastMove;  // Directly assign the move
        }

        // Update alpha with the best value found so far
        if (moveValue > alpha) {
            alpha = moveValue;
        }

        // Alpha-Beta Pruning: If the best value is already guaranteed, stop searching
        if (beta <= alpha) {
            break;
        }
    }

    return bestMove;
}
