#include "global.h"
#include "guiServer.h"
#include "comm.h"
#include "board.h"
#include "move.h"
#include "gameServer.h"
#include <gtk/gtk.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
/**********************************************************/
guint whiteTag, blackTag;
int whiteTagValid, blackTagValid;
int removedFromBlack, removedFromWhite;

GtkWidget *imageBoard[ ARRAY_BOARD_SIZE ][ ARRAY_BOARD_SIZE ];
GuiTile GuiBoard[ ARRAY_BOARD_SIZE ][ ARRAY_BOARD_SIZE ];

GtkWidget *whiteCombo, *blackCombo;
GtkWidget *playButton, *stopButton, *resetButton, *swapButton;

GtkWidget *whiteDcButton, *blackDcButton;
GtkWidget *whitePlayerName, *blackPlayerName, *whiteScore, *blackScore;

PlayerStruct externalPlayers[ MAX_EXTERNAL_PLAYERS ];

int stopFlag;

int whitePlayerValue, blackPlayerValue;

char tempMessage[ 300 ];


/**********************************************************/
int sendMsgGS( int msg, int mySocket )
{
	int value;

	value = sendMsg( msg, mySocket );

	if( value < 0 && ( msg != NM_QUIT ) )
	{

		if( externalPlayers[ whitePlayerValue ].playerSocket == mySocket )
			whiteDc();
		else if( externalPlayers[ blackPlayerValue ].playerSocket == mySocket )
			blackDc();

		return -1;
	}

	return 0;

}


/**********************************************************/
int sendMoveGS( Move * moveToSend, int mySocket )
{
	int value;

	value = sendMove( moveToSend, mySocket );

	if( value < 0 )
	{

		if( externalPlayers[ whitePlayerValue ].playerSocket == mySocket )
			whiteDc();
		else if( externalPlayers[ blackPlayerValue ].playerSocket == mySocket )
			blackDc();

		return -1;
	}

	return 0;

}


/**********************************************************/
int getMoveGS( Move * moveToGet, int mySocket )
{
	int value;

	value = getMove( moveToGet, mySocket );

	if( value < 0 )
	{

		if( externalPlayers[ whitePlayerValue ].playerSocket == mySocket )
		{

			whiteDc();
		}else if( externalPlayers[ blackPlayerValue ].playerSocket == mySocket )
			blackDc();

		return -1;
	}

	return 0;

}


/**********************************************************/
int getNameGS( char textToGet[ MAX_NAME_LENGTH + 1 ], int mySocket )
{
	int value;

	value = getName( textToGet, mySocket );

	if( value < 0 )
	{

		if( externalPlayers[ whitePlayerValue ].playerSocket == mySocket )
			whiteDc();
		else if( externalPlayers[ blackPlayerValue ].playerSocket == mySocket )
			blackDc();

		return -1;
	}

	return 0;

}


/**********************************************************/
int sendPositionGS( Position * posToSend, int mySocket )
{
	int value;

	value = sendPosition( posToSend, mySocket );

	if( value < 0 )
	{

		if( externalPlayers[ whitePlayerValue ].playerSocket == mySocket )
			whiteDc();
		else if( externalPlayers[ blackPlayerValue ].playerSocket == mySocket )
			blackDc();

		return -1;
	}

	return 0;

}


/**********************************************************/
void printMessage(char * message)
{
	GtkWidget * dialog, *label, *image, *hbox;


	stopButton_clicked();

	dialog = gtk_dialog_new_with_buttons ("Info", NULL,GTK_DIALOG_MODAL,GTK_STOCK_OK, GTK_RESPONSE_OK,NULL);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	
	gtk_window_set_resizable(GTK_WINDOW (dialog),FALSE);

	label = gtk_label_new (message);


	image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO,GTK_ICON_SIZE_DIALOG);
	hbox = gtk_hbox_new (FALSE, 5);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), image);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), label);
	
	
	gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox);
	gtk_widget_show_all (dialog);

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	if( !canMove( &gamePosition, WHITE ) && !canMove( &gamePosition, BLACK ) )
	{
		while(gtk_events_pending())
			gtk_main_iteration_do(FALSE);

		gtk_widget_set_sensitive(GTK_WIDGET(playButton), FALSE);
	}

}


/**********************************************************/
void printToGui( void )
{
	int i, j;

	for( i = 0; i < ARRAY_BOARD_SIZE; i++ )
	{
		for( j = 0; j < ARRAY_BOARD_SIZE; j++ )
			switch( gamePosition.board[ i ][ j ] )
			{
				case WHITE:
					gtk_image_set_from_file( GTK_IMAGE( imageBoard[ i ][ j ] ), "images/simple/white.jpg" );
					GuiBoard[ i ][ j ].state = ST_WHITE;
					break;
				case BLACK:
					gtk_image_set_from_file( GTK_IMAGE( imageBoard[ i ][ j ]), "images/simple/black.jpg" );
					GuiBoard[ i ][ j ].state = ST_BLACK;
					break;
				case EMPTY:
					gtk_image_set_from_file( GTK_IMAGE( imageBoard[ i ][ j ] ), "images/simple/empty.jpg" );
					GuiBoard[ i ][ j ].state = ST_EMPTY;
					break;
				case OUT_OF_BOUND:
					break;
				case ILLEGAL:
					gtk_image_set_from_file( GTK_IMAGE( imageBoard[ i ][ j ] ), "images/simple/illegal.jpg" );
					GuiBoard[ i ][ j ].state = ST_ILLEGAL;
					break;
				default:
					printf( "ERROR: Unknown character in board (printBoard)\n" );
					exit( 1 );
			}
	}


	sprintf( tempMessage, "W: %d",gamePosition.score[ WHITE ] );
	gtk_label_set_text( GTK_LABEL( whiteScore ), tempMessage );

	sprintf( tempMessage, "B: %d",gamePosition.score[ BLACK ] );
	gtk_label_set_text( GTK_LABEL( blackScore ), tempMessage );


	highlightLastMove();

}

/**********************************************************/
void playRandom( void )
{
	int i,j;

	tempMove.color = gamePosition.turn;
	//random player ----
	if( !canMove( &gamePosition, gamePosition.turn ) )
	{
		tempMove.tile[ 0 ] = NULL_MOVE;	//null move
	}
	else
	{
		while( 1 )
		{
			i = rand() % ARRAY_BOARD_SIZE;
			j = rand() % ARRAY_BOARD_SIZE;

			if( gamePosition.board[ i ][ j ] == EMPTY )
			{
				tempMove.tile[ 0 ] = i;
				tempMove.tile[ 1 ] = j;
				if( isLegalMove( &gamePosition, &tempMove ) )
					break;
			}
		}
	}
	//end of random ----

	doMove( &gamePosition, &tempMove );
	printPosition( &gamePosition );
	printToGui();

	checkVictoryAndSendMove();

}

/**********************************************************/
void checkVictoryAndSendMove( void )
{
	if( !canMove( &gamePosition, WHITE ) && !canMove( &gamePosition, BLACK ) )	//if none can move..game ended
	{
		printf( "Game ended!\n" );

		if( gamePosition.score[ WHITE ] - gamePosition.score[ BLACK ] > 0 )
		{
			//white won
			sprintf(tempMessage, "WHITE WON! (%s) Score W:%d B:%d\n", externalPlayers[ whitePlayerValue ].name,gamePosition.score[ WHITE ], gamePosition.score[ BLACK ] );
		}
		else if( gamePosition.score[ WHITE ] - gamePosition.score[ BLACK ] < 0 )
		{
			//Black won
			sprintf(tempMessage, "BLACK WON! (%s) Score W:%d B:%d\n", externalPlayers[ blackPlayerValue ].name, gamePosition.score[ WHITE ], gamePosition.score[ BLACK ] );

		}
		else
			sprintf(tempMessage, "DRAW! Score W:%d B:%d\n", gamePosition.score[ WHITE ], gamePosition.score[ BLACK ] );

		printMessage(tempMessage);
		return;

	}


	//play null for manual player
	if( gamePosition.turn == WHITE && whitePlayerValue == 0 || gamePosition.turn == BLACK && blackPlayerValue == 0 )
	{
		if( !canMove( &gamePosition, gamePosition.turn ) )
		{
			tempMove.color = gamePosition.turn;
			tempMove.tile[ 0 ] = NULL_MOVE;
			doMove( &gamePosition, &tempMove );
			printPosition( &gamePosition );
			printToGui();

			checkVictoryAndSendMove();
			return;
		}
		else
			highlightPossibleMoves( gamePosition.turn );
	}



	//send move to the other player
	if( gamePosition.turn == WHITE )
	{

		if( whitePlayerValue > 1)
		{
			if( sendMsgGS( NM_PREPARE_TO_RECEIVE_MOVE, externalPlayers[ whitePlayerValue ].playerSocket ) < 0 )
				return;

			if( sendMoveGS( &tempMove, externalPlayers[ whitePlayerValue ].playerSocket ) < 0 )
				return;

			if( stopFlag == 0 )
				if( sendMsgGS( NM_REQUEST_MOVE, externalPlayers[ whitePlayerValue ].playerSocket ) < 0 )
					return;
		}
		else if(whitePlayerValue == 1 && stopFlag == 0 )
		{
			while( gtk_events_pending() )
				gtk_main_iteration_do( FALSE );
			playRandom();

		}

	}
	else if( gamePosition.turn == BLACK)
	{
		if( blackPlayerValue > 1)
		{
			if( sendMsgGS( NM_PREPARE_TO_RECEIVE_MOVE, externalPlayers[ blackPlayerValue ].playerSocket ) < 0 )
				return;

			if( sendMoveGS( &tempMove, externalPlayers[ blackPlayerValue ].playerSocket ) < 0 )
				return;

			if( stopFlag == 0 )
				if( sendMsgGS( NM_REQUEST_MOVE, externalPlayers[ blackPlayerValue ].playerSocket ) < 0 )
					return;
		}
		else if( blackPlayerValue == 1 && stopFlag == 0 )
		{
			while( gtk_events_pending() )
				gtk_main_iteration_do( FALSE );
			playRandom();

		}
	}



}


/**********************************************************/
void messageFromSocket( void )
{
	PlayerStruct * playingPlayer = NULL;
	PlayerStruct * waitingPlayer = NULL;
	int i;

	if( gamePosition.turn == WHITE )
	{
		if( whitePlayerValue > 1 )
			playingPlayer = &externalPlayers[ whitePlayerValue ];

		if( blackPlayerValue > 1 )
			waitingPlayer = &externalPlayers[ blackPlayerValue ];
	}
	else
	{
		if( blackPlayerValue > 1 )
			playingPlayer = &externalPlayers[ blackPlayerValue ];

		if( whitePlayerValue > 1 )
			waitingPlayer = &externalPlayers[ whitePlayerValue ];
	}


	if( playingPlayer == NULL )
	{
		getMoveGS( &tempMove, waitingPlayer->playerSocket );
		return;
	}

	if( getMoveGS( &tempMove, playingPlayer->playerSocket ) < 0 )
		return;

	tempMove.color = playingPlayer->color;


	if( !canMove( &gamePosition, playingPlayer->color ) )	//if that player cannot move, the only legal move is null
	{
		if( tempMove.tile[ 0 ] != NULL_MOVE )	//technical loss
		{
			sprintf( tempMessage, "Player: %s tried an illegal move and lost the game!\nIllegal move:", playingPlayer->name );
			sprintf( tempMessage + strlen(tempMessage), "( %d, %d )", tempMove.tile[ 0 ], tempMove.tile[ 1 ] );
			printMessage( tempMessage );
			return;
		}
	}
	else
	{
		if( !isLegalMove( &gamePosition, &tempMove ) )
		{
			//technical loss
			sprintf( tempMessage, "Player: %s tried an illegal move and lost the game!\nIllegal move:", playingPlayer->name );

			if( tempMove.tile[ 0 ] == NULL_MOVE )	//since we can move, null move is illegal
				sprintf( tempMessage + strlen(tempMessage), "NULL MOVE" );
			else
				sprintf( tempMessage + strlen(tempMessage), "( %d, %d )", tempMove.tile[ 0 ], tempMove.tile[ 1 ] );
			printMessage( tempMessage );
			return;
		}
	}


	//we have a legal move
	doMove( &gamePosition, &tempMove );
	printPosition( &gamePosition );
	printToGui();

	checkVictoryAndSendMove();

}

/**********************************************************/
void highlightLastMove( void )
{

	if(tempMove.tile[ 0 ] == NULL_MOVE )
		return;

	if(tempMove.color == WHITE)
	{
		gtk_image_set_from_file( GTK_IMAGE( imageBoard[tempMove.tile[ 0 ]][tempMove.tile[ 1 ]] ), "images/simple/white_lmh.jpg" );
		GuiBoard[tempMove.tile[ 0 ]][tempMove.tile[ 1 ]].state = ST_WHITE_LAST_MOVE_HIGHTLIGHT;
	}
	else if(tempMove.color == BLACK)
	{
		gtk_image_set_from_file( GTK_IMAGE( imageBoard[tempMove.tile[ 0 ]][tempMove.tile[ 1 ]] ), "images/simple/black_lmh.jpg" );
		GuiBoard[tempMove.tile[ 0 ]][tempMove.tile[ 1 ]].state = ST_BLACK_LAST_MOVE_HIGHTLIGHT;
	}
	else
	{
		printf("ERROR! highlightLastMove\n");
		exit(1);
	}
}

/**********************************************************/
void unhighlightLastMove( void )
{
	int i, j;

	for( i = 0; i < ARRAY_BOARD_SIZE; i++ )
	{
		for( j = 0; j < ARRAY_BOARD_SIZE; j++ )
		{
			if( GuiBoard[ i ][ j ].state == ST_WHITE_LAST_MOVE_HIGHTLIGHT )
			{
				gtk_image_set_from_file(GTK_IMAGE(imageBoard[ i ][ j ]),"images/simple/white.jpg");
				GuiBoard[ i ][ j ].state = ST_WHITE;
			}
			else if( GuiBoard[ i ][ j ].state == ST_BLACK_LAST_MOVE_HIGHTLIGHT )
			{
				gtk_image_set_from_file(GTK_IMAGE(imageBoard[ i ][ j ]),"images/simple/black.jpg");
				GuiBoard[ i ][ j ].state = ST_BLACK;
			}
		}
	}
}


/**********************************************************/
void highlightPossibleMoves( char color )
{

	unsigned short int i,j;

	for(i=0 ; i <ARRAY_BOARD_SIZE ; i++)
		for (j=0 ; j <ARRAY_BOARD_SIZE ; j++)
		{
			if (isLegal(&gamePosition, i, j, color))
			{
				gtk_image_set_from_file(GTK_IMAGE(imageBoard[ i ][ j ]),"images/simple/possibleMove.jpg");
				GuiBoard[ i ][ j ].state = ST_EMPTY_POSSIBLE_MOVE_HIGHTLIGHT;
			}
		}
}

/**********************************************************/
void unhighlightPossibleMoves( void )		
{
	int i, j;

	for( i = 0; i < ARRAY_BOARD_SIZE; i++ )
	{
		for( j = 0; j < ARRAY_BOARD_SIZE; j++ )
		{
			if( GuiBoard[ i ][ j ].state == ST_EMPTY_POSSIBLE_MOVE_HIGHTLIGHT )
			{
				gtk_image_set_from_file(GTK_IMAGE(imageBoard[ i ][ j ]),"images/simple/empty.jpg");
				GuiBoard[ i ][ j ].state = ST_EMPTY;
			}
		}
	}

}

/**********************************************************/
void tile_selected( GtkWidget *widget, GdkEventButton *event, gpointer data )
{

	if( stopFlag == 1 )	//if game stopped then you cannot select anything
		return;

	if( !(gamePosition.turn == WHITE && whitePlayerValue == 0 || gamePosition.turn == BLACK && blackPlayerValue == 0 ) )
		return;

	GuiTile * clickedTile;

	clickedTile = ( GuiTile * ) data;

	tempMove.color = gamePosition.turn;

	unhighlightLastMove();

	tempMove.tile[ 0 ] = clickedTile->x;
	tempMove.tile[ 1 ] = clickedTile->y;

	if( isLegalMove( &gamePosition, &tempMove ) )
	{
		unhighlightPossibleMoves();
		//we have a legal move
		doMove( &gamePosition, &tempMove );
		printPosition( &gamePosition );
		printToGui();

		checkVictoryAndSendMove();
		return;
	}

}




/**********************************************************/
void whiteCombo_changed( void )
{

	whitePlayerValue = gtk_combo_box_get_active( GTK_COMBO_BOX( whiteCombo ) );

	//determine if disconnect should be enabled
	if( whitePlayerValue < 2 )
		gtk_widget_set_sensitive( GTK_WIDGET( whiteDcButton ), FALSE );

	//if en entry is missing from the combo box
	if( blackTagValid == TRUE && gtk_combo_box_get_active( GTK_COMBO_BOX( whiteCombo ) ) >= removedFromWhite )
		whitePlayerValue = gtk_combo_box_get_active( GTK_COMBO_BOX( whiteCombo ) ) + 1;



	if( whiteTagValid == TRUE )
	{
		//restore the one we had back into the combobox now that we selected different agent
		gdk_input_remove( whiteTag );
		whiteTagValid = FALSE;
		gtk_combo_box_insert_text( GTK_COMBO_BOX( blackCombo ), removedFromBlack, externalPlayers[ removedFromBlack ].name );
	}

	while( gtk_events_pending() )
		gtk_main_iteration_do( FALSE );

	if( gtk_combo_box_get_active( GTK_COMBO_BOX( whiteCombo ) ) > 1 )
	{

		externalPlayers[ whitePlayerValue ].color = WHITE;

		if( sendMsgGS( NM_NEW_POSITION, externalPlayers[ whitePlayerValue ].playerSocket ) < 0 )
			return;
		if( sendPositionGS( &gamePosition, externalPlayers[ whitePlayerValue ].playerSocket ) < 0 )
			return;

		if( sendMsgGS( NM_COLOR_W, externalPlayers[ whitePlayerValue ].playerSocket) < 0 )
			return;

		whiteTag=gdk_input_add( externalPlayers[ whitePlayerValue ].playerSocket, GDK_INPUT_READ, ( GdkInputFunction )messageFromSocket, NULL);
		whiteTagValid = TRUE;

		while( gtk_events_pending() )
			gtk_main_iteration_do( FALSE );

		//remove same choise from the other combo
		removedFromBlack = whitePlayerValue;
		gtk_combo_box_remove_text( GTK_COMBO_BOX( blackCombo ), removedFromBlack );

		gtk_widget_set_sensitive( GTK_WIDGET( whiteDcButton ), TRUE );
	}

	//change label with player's name
	gchar *text = gtk_combo_box_get_active_text(GTK_COMBO_BOX( whiteCombo ) );
	gtk_label_set_text( GTK_LABEL( whitePlayerName ), text );
	g_free( text );


}


/**********************************************************/
void blackCombo_changed( void )
{

	blackPlayerValue = gtk_combo_box_get_active( GTK_COMBO_BOX( blackCombo ) );

	//determine if disconnect should be enabled
	if( blackPlayerValue < 2 )
		gtk_widget_set_sensitive( GTK_WIDGET( blackDcButton ), FALSE );

	//if en entry is missing from the combo box
	if( whiteTagValid == TRUE && gtk_combo_box_get_active( GTK_COMBO_BOX( blackCombo ) ) >= removedFromBlack )
		blackPlayerValue = gtk_combo_box_get_active( GTK_COMBO_BOX( blackCombo ) ) + 1;




	if( blackTagValid == TRUE )
	{
		//restore the one we had back into the combobox now that we selected different agent
		gdk_input_remove( blackTag );
		blackTagValid = FALSE;
		gtk_combo_box_insert_text( GTK_COMBO_BOX( whiteCombo ), removedFromWhite, externalPlayers[ removedFromWhite ].name );
	}

	while( gtk_events_pending() )
		gtk_main_iteration_do( FALSE );

	if( gtk_combo_box_get_active( GTK_COMBO_BOX( blackCombo ) ) > 1 )
	{

		externalPlayers[ blackPlayerValue ].color = BLACK;

		if( sendMsgGS( NM_NEW_POSITION, externalPlayers[ blackPlayerValue ].playerSocket ) < 0 )
			return;
		if( sendPositionGS( &gamePosition, externalPlayers[ blackPlayerValue ].playerSocket ) < 0 )
			return;

		if( sendMsgGS( NM_COLOR_B, externalPlayers[ blackPlayerValue ].playerSocket ) < 0 )
			return;

		blackTag = gdk_input_add( externalPlayers[ blackPlayerValue ].playerSocket, GDK_INPUT_READ, ( GdkInputFunction )messageFromSocket, NULL);
		blackTagValid = TRUE;

		while( gtk_events_pending() )
			gtk_main_iteration_do( FALSE );

		//remove same choise from the other combo
		removedFromWhite = blackPlayerValue;
		gtk_combo_box_remove_text( GTK_COMBO_BOX( whiteCombo ), removedFromWhite );

		gtk_widget_set_sensitive( GTK_WIDGET( blackDcButton ), TRUE);
	}

	//change label with player's name
	gchar *text =  gtk_combo_box_get_active_text( GTK_COMBO_BOX( blackCombo ) );
	gtk_label_set_text( GTK_LABEL( blackPlayerName ), text );
	g_free( text );

}

/**********************************************************/
void playButton_clicked( void )
{

	stopFlag = 0;
	gtk_widget_set_sensitive( GTK_WIDGET( stopButton ), TRUE );
	gtk_widget_set_sensitive( GTK_WIDGET( playButton ), FALSE );
	gtk_widget_set_sensitive( GTK_WIDGET( resetButton ), FALSE );
	gtk_widget_set_sensitive( GTK_WIDGET( swapButton ), FALSE );

	gtk_widget_set_sensitive( GTK_WIDGET( whiteCombo ), FALSE );
	gtk_widget_set_sensitive( GTK_WIDGET( blackCombo ), FALSE );

	gtk_widget_set_sensitive( GTK_WIDGET( whiteDcButton ), FALSE );
	gtk_widget_set_sensitive( GTK_WIDGET( blackDcButton ), FALSE );

	//start the game depending on the player
	if( gamePosition.turn == WHITE )
	{
		if( whitePlayerValue == 1 )	//random
		{
			while( gtk_events_pending() )
				gtk_main_iteration_do( FALSE );
			playRandom();
			return;

		}
		else if( whitePlayerValue > 1 )	//external
		{
			sendMsgGS( NM_REQUEST_MOVE, externalPlayers[ whitePlayerValue ].playerSocket );
			return;
		}
	}
	else
	{

		if( blackPlayerValue == 1 )	//random
		{
			while( gtk_events_pending() )
				gtk_main_iteration_do( FALSE );
			playRandom();
			return;
		}
		else if( blackPlayerValue > 1 )	//external
		{
			sendMsgGS( NM_REQUEST_MOVE, externalPlayers[ blackPlayerValue ].playerSocket );
			return;
		}
	}

	//play null for manual player if he has no move available
	if( gamePosition.turn == WHITE && whitePlayerValue == 0 || gamePosition.turn == BLACK && blackPlayerValue == 0 )
	{
		if( !canMove( &gamePosition, gamePosition.turn ) )
		{
			tempMove.color = gamePosition.turn;
			tempMove.tile[ 0 ] = NULL_MOVE;
			doMove( &gamePosition, &tempMove );
			printPosition( &gamePosition );
			printToGui();

			checkVictoryAndSendMove();
			return;
		}
		else
			highlightPossibleMoves( gamePosition.turn );
	}

}


/**********************************************************/
void stopButton_clicked( void )
{

	stopFlag = 1;
	gtk_widget_set_sensitive( GTK_WIDGET( stopButton ), FALSE );
	gtk_widget_set_sensitive( GTK_WIDGET( playButton ), TRUE );
	gtk_widget_set_sensitive( GTK_WIDGET( resetButton ), TRUE );
	gtk_widget_set_sensitive( GTK_WIDGET( swapButton ), TRUE );

	gtk_widget_set_sensitive( GTK_WIDGET( whiteCombo ), TRUE );
	gtk_widget_set_sensitive( GTK_WIDGET( blackCombo ), TRUE );

	if( whitePlayerValue > 1 )		//only in case we have external player
		gtk_widget_set_sensitive( GTK_WIDGET( whiteDcButton ), TRUE );

	if( blackPlayerValue > 1 )		//only in case we have external player
		gtk_widget_set_sensitive( GTK_WIDGET( blackDcButton ), TRUE );

}

/**********************************************************/
void resetButton_clicked( void )
{


	gtk_widget_set_sensitive( GTK_WIDGET( playButton ), TRUE );

	tempMove.tile[ 0 ] = NULL_MOVE;
	initPosition( &gamePosition );
	printToGui();

	//inform external players (if they are playing)
	if( whitePlayerValue > 1 )
	{
		if( sendMsgGS( NM_NEW_POSITION, externalPlayers[ whitePlayerValue ].playerSocket ) < 0 )
			return;
		if( sendPositionGS( &gamePosition, externalPlayers[ whitePlayerValue ].playerSocket ) < 0 )
			return;
	}

	if( blackPlayerValue > 1 )
	{
		if( sendMsgGS( NM_NEW_POSITION, externalPlayers[ blackPlayerValue ].playerSocket ) < 0 )
			return;
		if( sendPositionGS( &gamePosition, externalPlayers[ blackPlayerValue ].playerSocket ) < 0 )
			return;
	}

}

/**********************************************************/
void closeConnections( void )
{

	int i;

	for( i = 2; i < MAX_EXTERNAL_PLAYERS; i++ )	//all except human and random
	{
		if( externalPlayers[ i ].connected == 1 )
			sendMsgGS( NM_QUIT, externalPlayers[ i ].playerSocket );
	}

}


/**********************************************************/
void quit( void )
{
	closeConnections();
	gtk_main_quit();
}

/**********************************************************/
void swapPlayers( void )
{

	int tempValueWhite, tempValueBlack;

	tempValueWhite = blackPlayerValue;
	tempValueBlack = whitePlayerValue;

	gtk_combo_box_set_active( GTK_COMBO_BOX( whiteCombo ), 0);
	gtk_combo_box_set_active( GTK_COMBO_BOX( blackCombo ), 0);

	if( tempValueWhite > tempValueBlack )	//in order to be correct we have to insert the one with the highest value first.
	{
		gtk_combo_box_set_active( GTK_COMBO_BOX( whiteCombo ), tempValueWhite );
		gtk_combo_box_set_active( GTK_COMBO_BOX( blackCombo ), tempValueBlack );
	}
	else
	{
		gtk_combo_box_set_active( GTK_COMBO_BOX( blackCombo ), tempValueBlack );
		gtk_combo_box_set_active( GTK_COMBO_BOX( whiteCombo ), tempValueWhite );
	}
}

/**********************************************************/
void takeCareDc( void )		//clear the gaps inside externalPlayers array
{
	int i;


	for( i = 1; i < MAX_EXTERNAL_PLAYERS; i++ )
	{
		if( externalPlayers[ i - 1 ].connected == 0 && externalPlayers[ i ].connected == 1 )
		{

			if( removedFromBlack == i )
				removedFromBlack = i-1;
			if( removedFromWhite == i )
				removedFromWhite = i-1;
			if( whitePlayerValue == i )
				whitePlayerValue = i-1;
			if( blackPlayerValue == i )
				blackPlayerValue = i-1;
			externalPlayers[ i ].connected = 0;
			externalPlayers[ i - 1 ].connected = 1;

			strcpy( externalPlayers[ i - 1 ].name, externalPlayers[ i ].name );

			externalPlayers[ i - 1 ].color = externalPlayers[ i ].color;
			externalPlayers[ i - 1 ].playerSocket = externalPlayers[ i ].playerSocket;


		}
	}

}

/**********************************************************/
void whiteDc( void )
{
	int tempValue;

	tempValue = whitePlayerValue;

	gtk_combo_box_set_active( GTK_COMBO_BOX( whiteCombo ), 0 );
	sendMsgGS( NM_QUIT, externalPlayers[ tempValue ].playerSocket );

	externalPlayers[ tempValue ].connected = 0;

	while( gtk_events_pending() )
		gtk_main_iteration_do( FALSE );

	gtk_combo_box_remove_text( GTK_COMBO_BOX( blackCombo ), tempValue );

	if( blackTagValid == TRUE && tempValue >= removedFromWhite )
		gtk_combo_box_remove_text( GTK_COMBO_BOX( whiteCombo ), tempValue - 1 );
	else
		gtk_combo_box_remove_text( GTK_COMBO_BOX( whiteCombo ), tempValue );

	takeCareDc();

}

/**********************************************************/
void blackDc( void )
{
	int tempValue;

	tempValue = blackPlayerValue;

	gtk_combo_box_set_active( GTK_COMBO_BOX( blackCombo ), 0 );
	sendMsgGS( NM_QUIT, externalPlayers[ tempValue ].playerSocket );

	externalPlayers[ tempValue ].connected = 0;

	while( gtk_events_pending() )
		gtk_main_iteration_do( FALSE );

	gtk_combo_box_remove_text( GTK_COMBO_BOX( whiteCombo ), tempValue );

	if( whiteTagValid == TRUE && tempValue >= removedFromBlack )
		gtk_combo_box_remove_text( GTK_COMBO_BOX( blackCombo ), tempValue - 1 );
	else
		gtk_combo_box_remove_text( GTK_COMBO_BOX( blackCombo ), tempValue );

	takeCareDc();

}



/**********************************************************/
void new_connection( void )
{
	int i;
	int tempSocket;

	//find empty slot
	for( i = 2; i < MAX_EXTERNAL_PLAYERS; i++ )
	{
		if( externalPlayers[ i ].connected == 0 )
			break;
	}


	if( i == MAX_EXTERNAL_PLAYERS )	//connection limit reached! reject!
	{
		printf(" ERROR!!! MAX PLAYERS LIMIT REACHED!!! REJECTING CONNECTION...CHANGE MAX_EXTERNAL_PLAYERS\n\n");
		tempSocket = acceptConnection( serverSocket );
		if( tempSocket < 0 )
			return;
		close( tempSocket );
		return;
	}


	externalPlayers[ i ].playerSocket = acceptConnection( serverSocket );

	if( externalPlayers[ i ].playerSocket < 0 )
		return;

	externalPlayers[ i ].connected = 1;

	if( sendMsgGS( NM_REQUEST_NAME, externalPlayers[ i ].playerSocket ) < 0 )
		return;
	if( getNameGS( externalPlayers[ i ].name, externalPlayers[ i ].playerSocket ) < 0 )
		return;
	//insert to comboboxes
	gtk_combo_box_insert_text( GTK_COMBO_BOX( whiteCombo ), i, externalPlayers[ i ].name );
	gtk_combo_box_insert_text( GTK_COMBO_BOX( blackCombo ), i, externalPlayers[ i ].name );
}


/**********************************************************/
GdkPixbuf *create_pixbuf( const gchar * filename )
{
	GdkPixbuf *pixbuf;
	GError *error = NULL;
	pixbuf = gdk_pixbuf_new_from_file( filename, &error );
	if( !pixbuf ) {
		fprintf( stderr, "%s\n", error->message );
	g_error_free( error );
	}

	return pixbuf;
}




/**********************************************************/
int main( int argc, char *argv[] )
{

	GtkWidget *window;

	GtkWidget *table;
	GtkWidget *tableRows[ ARRAY_BOARD_SIZE ];
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *vboxSideOptions;
	GtkWidget *hboxMainButtons;

	GtkWidget *quitButton;
	GtkWidget *sep1, *sep2;

	GtkWidget *hboxPlayerNamesAndSwap;

	GtkWidget *whiteSelect, *blackSelect;

	GtkWidget *hPlayerSelection, *vWhiteSelection, *vBlackSelection, *hScoreBox;

	GtkWidget *eventBoard[ ARRAY_BOARD_SIZE ][ ARRAY_BOARD_SIZE ];
	GtkWidget *dummyImageFrame[ (HEX_BOARD_RADIUS * 3 + 1) * (HEX_BOARD_RADIUS * 4 + 3) ];

	gtk_init( &argc, &argv );

	window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_position( GTK_WINDOW( window ), GTK_WIN_POS_CENTER );
	gtk_window_set_default_size( GTK_WINDOW( window ), 400, 400 );
	gtk_window_set_title( GTK_WINDOW( window ), "TUC HexThello v0.98b" );
	gtk_window_set_resizable( GTK_WINDOW( window ), FALSE );
	gtk_window_set_icon( GTK_WINDOW( window ), create_pixbuf( "images/hexthello.ico" ) );

	gtk_container_set_border_width( GTK_CONTAINER( window ), 0 );

	hbox = gtk_hbox_new( FALSE, 0 );
	gtk_container_add( GTK_CONTAINER( window ), hbox );

	table = gtk_table_new( HEX_BOARD_RADIUS * 4 + 3, 1, FALSE );

	//create board with event boxes and images
	int i = 0;
	int j = 0;
	int gapPerRow = HEX_BOARD_RADIUS;
	int hexPerRow = HEX_BOARD_RADIUS + 1;
	int dummyIndex = 0;
	int outerArrayIndex = 0;
	int innerArrayIndex = 0;
	int count;

	for( outerArrayIndex = 0; outerArrayIndex < HEX_BOARD_RADIUS * 4 + 3; outerArrayIndex++ )
	{
		tableRows[ outerArrayIndex ] = gtk_table_new( 1, HEX_BOARD_RADIUS * 3 + 1, FALSE );

		if( outerArrayIndex % 2 == 0 )
		{
			if( outerArrayIndex != 0 )
			{
				if( outerArrayIndex <= HEX_BOARD_RADIUS * 2 )
					gapPerRow--;
			}
			
			for( count = gapPerRow; count > 0; count-- )
			{
				dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/gap_small.jpg" );
				gtk_table_attach_defaults( GTK_TABLE( tableRows[ outerArrayIndex ] ),GTK_WIDGET( dummyImageFrame[ dummyIndex ]  ), innerArrayIndex, innerArrayIndex + 1, 0, 1 );
				dummyIndex++;
				innerArrayIndex++;
			}
			
			for( count = hexPerRow; count > 0; count-- )
			{
				if( outerArrayIndex == 0)
					dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/up_0_0.jpg" );
				else if( outerArrayIndex == HEX_BOARD_RADIUS * 4 + 2 )
					dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/down_0_0.jpg" );
				else if( outerArrayIndex < HEX_BOARD_RADIUS * 2 + 2 )
				{
					if( count == hexPerRow )
						dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/up_0_1.jpg" );
					else if( count == 1 )
						dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/up_1_0.jpg" );
					else
						dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/up_1_1.jpg" );
				}
				else
				{
					if( count == hexPerRow )
						dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/down_0_1.jpg" );
					else if( count == 1 )
						dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/down_1_0.jpg" );
					else
						dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/down_1_1.jpg" );
				}

				gtk_table_attach_defaults( GTK_TABLE( tableRows[ outerArrayIndex ] ),GTK_WIDGET( dummyImageFrame[ dummyIndex ]  ), innerArrayIndex, innerArrayIndex + 1, 0, 1 );
				dummyIndex++;
				innerArrayIndex++;
			}
			
			for( count = gapPerRow; count > 0; count-- )
			{
				dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/gap_small.jpg" );
				gtk_table_attach_defaults( GTK_TABLE( tableRows[ outerArrayIndex ] ),GTK_WIDGET( dummyImageFrame[ dummyIndex ]  ), innerArrayIndex, innerArrayIndex + 1, 0, 1 );
				dummyIndex++;
				innerArrayIndex++;
			}

			if( outerArrayIndex > HEX_BOARD_RADIUS * 2 )
				hexPerRow--;

			innerArrayIndex = 0;
			gtk_table_attach_defaults( GTK_TABLE( table ),GTK_WIDGET( tableRows[ outerArrayIndex ] ), 0, 1, outerArrayIndex, outerArrayIndex + 1 );

		}
		else
		{
			if( outerArrayIndex > HEX_BOARD_RADIUS * 2 + 1)
				gapPerRow++;

			for( count = gapPerRow; count > 0; count-- )
			{
				dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/gap_large.jpg" );
				gtk_table_attach_defaults( GTK_TABLE( tableRows[ outerArrayIndex ] ),GTK_WIDGET( dummyImageFrame[ dummyIndex ]  ), innerArrayIndex, innerArrayIndex + 1, 0, 1 );
				dummyIndex++;
				innerArrayIndex++;
			}
			
			if( i < HEX_BOARD_RADIUS )
				j = HEX_BOARD_RADIUS - i;
			
			for( count = hexPerRow; count > 0; count-- )
			{
				eventBoard[ i ][ j ] = gtk_event_box_new();
				g_signal_connect( G_OBJECT( eventBoard[ i ][ j ] ), "button-press-event", G_CALLBACK( tile_selected ), ( gpointer )( &GuiBoard[ i ][ j ] ) );
				imageBoard[ i ][ j ] = gtk_image_new_from_file( "images/simple/empty.jpg" );
				GuiBoard[ i ][ j ].x = i;
				GuiBoard[ i ][ j ].y = j;
				GuiBoard[ i ][ j ].state = ST_EMPTY;
				gtk_container_add( GTK_CONTAINER( eventBoard[ i ][ j ] ), imageBoard[ i ][ j ] );
				gtk_table_attach_defaults( GTK_TABLE( tableRows[ outerArrayIndex ] ),GTK_WIDGET( eventBoard[ i ][ j ]  ), innerArrayIndex, innerArrayIndex + 1, 0, 1 );
				innerArrayIndex++;
				j++;
			}
			
			for( count = gapPerRow; count > 0; count-- )
			{
				dummyImageFrame[ dummyIndex ] = gtk_image_new_from_file( "images/simple/gap_large.jpg" );
				gtk_table_attach_defaults( GTK_TABLE( tableRows[ outerArrayIndex ] ),GTK_WIDGET( dummyImageFrame[ dummyIndex ]  ), innerArrayIndex, innerArrayIndex + 1, 0, 1 );
				dummyIndex++;
				innerArrayIndex++;
			}
			
			innerArrayIndex = 0;
			j = 0;
			gtk_table_attach_defaults( GTK_TABLE( table ),GTK_WIDGET( tableRows[ outerArrayIndex ] ), 0, 1, outerArrayIndex, outerArrayIndex + 1 );
			i++;
			if( outerArrayIndex <= HEX_BOARD_RADIUS * 2 )
				hexPerRow++;

		}


	}

	gtk_container_add( GTK_CONTAINER( hbox ), table );

	vboxSideOptions = gtk_vbox_new( FALSE, 0 );

	gtk_container_add( GTK_CONTAINER( hbox ), vboxSideOptions );

	hboxMainButtons = gtk_hbox_new( FALSE, 0 );

	gtk_box_pack_start( GTK_BOX( vboxSideOptions ), hboxMainButtons, FALSE, FALSE, 5 );

	//upper 4 buttons
	playButton = gtk_button_new_with_label( "Play" );
	stopButton = gtk_button_new_with_label( "Stop" );
	resetButton = gtk_button_new_with_label( "Reset" );
	quitButton = gtk_button_new_with_label( "Quit" );

	gtk_box_pack_start( GTK_BOX( hboxMainButtons ), playButton, TRUE, TRUE, 5 );
	gtk_box_pack_start( GTK_BOX( hboxMainButtons ), stopButton, TRUE, TRUE, 5 );
	gtk_box_pack_start( GTK_BOX( hboxMainButtons ), resetButton, TRUE, TRUE, 5 );
	gtk_box_pack_start( GTK_BOX( hboxMainButtons ), quitButton, TRUE, TRUE, 5 );

	g_signal_connect( G_OBJECT( playButton ), "clicked", G_CALLBACK( playButton_clicked ), NULL );
	g_signal_connect( G_OBJECT( stopButton ), "clicked", G_CALLBACK( stopButton_clicked ), NULL );
	g_signal_connect( G_OBJECT( resetButton ), "clicked", G_CALLBACK( resetButton_clicked ), NULL );
	g_signal_connect( G_OBJECT( quitButton ), "clicked", G_CALLBACK( quit ), NULL );

	//seperator 1
	sep1 = gtk_hseparator_new();
	gtk_box_pack_start( GTK_BOX( vboxSideOptions ), sep1, FALSE, FALSE, 5 );

	//names and swap
	hboxPlayerNamesAndSwap = gtk_hbox_new( TRUE, 0 );
	gtk_box_pack_start( GTK_BOX( vboxSideOptions ), hboxPlayerNamesAndSwap, FALSE, FALSE, 5 );

	//label for white's name
	whitePlayerName = gtk_label_new( "Human" );
	//button to swap players
	swapButton = gtk_button_new_with_label( "Swap\n  <->" );
	g_signal_connect( G_OBJECT( swapButton ), "clicked", G_CALLBACK( swapPlayers ), NULL );
	//label for black's name
	blackPlayerName = gtk_label_new("Human");

	gtk_box_pack_start( GTK_BOX( hboxPlayerNamesAndSwap ), whitePlayerName, FALSE, FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( hboxPlayerNamesAndSwap ), swapButton, FALSE, FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( hboxPlayerNamesAndSwap ), blackPlayerName, FALSE, FALSE, 5 );

	//scores
	hScoreBox = gtk_hbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( vboxSideOptions ), hScoreBox, FALSE, FALSE, 5 );

	//whiteScore and blackScore
	whiteScore = gtk_label_new( "W: 0" );
	blackScore = gtk_label_new( "B: 0" );

	//whiteScore and blackScore inside hScoreBox
	gtk_box_pack_start( GTK_BOX( hScoreBox ), whiteScore, FALSE, FALSE, 20 );
	gtk_box_pack_end( GTK_BOX( hScoreBox ), blackScore, FALSE, FALSE, 20 );

	//seperator 2
	sep2 = gtk_hseparator_new();

	gtk_box_pack_start( GTK_BOX( vboxSideOptions ), sep2, FALSE, FALSE, 5 );


	hPlayerSelection = gtk_hbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( vboxSideOptions ), hPlayerSelection, FALSE, FALSE, 5 );

	vWhiteSelection = gtk_vbox_new( FALSE, 0 );
	gtk_box_pack_start( GTK_BOX( hPlayerSelection ), vWhiteSelection, FALSE, FALSE, 5 );
	vBlackSelection = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_end( GTK_BOX( hPlayerSelection ), vBlackSelection, FALSE, FALSE, 5 );

	//whiteSelect and blackSelect labels
	whiteSelect = gtk_label_new( "Select White:" );
	blackSelect = gtk_label_new( "Select Black:" );

	//whiteSelect and blackSelect inside vWhiteSelection and vBlackSelection
	gtk_box_pack_start( GTK_BOX( vWhiteSelection ), whiteSelect, FALSE, FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vBlackSelection ), blackSelect, FALSE, FALSE, 5 );

	//whiteCombo
	whiteCombo = gtk_combo_box_new_text();
	gtk_combo_box_append_text( GTK_COMBO_BOX( whiteCombo ), "Human" );
	gtk_combo_box_append_text( GTK_COMBO_BOX( whiteCombo ), "Random" );
	gtk_combo_box_set_active( GTK_COMBO_BOX( whiteCombo ), 0 );

	//blackCombo
	blackCombo = gtk_combo_box_new_text();
	gtk_combo_box_append_text( GTK_COMBO_BOX( blackCombo), "Human" );
	gtk_combo_box_append_text( GTK_COMBO_BOX( blackCombo), "Random" );
	gtk_combo_box_set_active( GTK_COMBO_BOX( blackCombo), 0 );

	//whiteCombo and blackCombo sizes
	gtk_widget_set_size_request( whiteCombo, 120, 30 );
	gtk_widget_set_size_request( blackCombo, 120, 30 );

	//whiteCombo and blackCombo inside vWhiteSelection and vBlackSelection
	gtk_box_pack_start( GTK_BOX( vWhiteSelection ), whiteCombo, FALSE, FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vBlackSelection ), blackCombo, FALSE, FALSE, 5 );


	//whiteCombo and blackCombo signals
	g_signal_connect( G_OBJECT( whiteCombo ), "changed", G_CALLBACK( whiteCombo_changed ), NULL );
	g_signal_connect( G_OBJECT( blackCombo ), "changed", G_CALLBACK( blackCombo_changed ), NULL );

	//whiteDcButton
	whiteDcButton = gtk_button_new_with_label( "Disconnect" );

	//blackDcButton
	blackDcButton = gtk_button_new_with_label( "Disconnect" );

	//whiteDcButton and blackDcButton signals
	g_signal_connect( G_OBJECT( whiteDcButton ), "clicked", G_CALLBACK( whiteDc ), NULL );
	g_signal_connect( G_OBJECT( blackDcButton ), "clicked", G_CALLBACK( blackDc ), NULL );

	//whiteDcButton and blackDcButton inside vWhiteSelection and vBlackSelection
	gtk_box_pack_start( GTK_BOX( vWhiteSelection ), whiteDcButton, FALSE, FALSE, 5 );
	gtk_box_pack_start( GTK_BOX( vBlackSelection ), blackDcButton, FALSE, FALSE, 5 );

	//destroy signal
	g_signal_connect_swapped( G_OBJECT( window ), "destroy", G_CALLBACK( quit ), G_OBJECT( window ) );

	gtk_widget_show_all( window );


	//srand needed for random player
	srand( time( NULL ) );


	//initilizations

	for( i = 0; i < MAX_EXTERNAL_PLAYERS; i++ )
	{
		if( i == 0)		//human
		{
			externalPlayers[ i ].connected = 1;
			sprintf( externalPlayers[ i ].name, "Human" );
		}
		else if( i == 1 )	//random
		{
			externalPlayers[ i ].connected = 1;
			sprintf( externalPlayers[ i ].name, "Random" );
		}
		else	//external
			externalPlayers[ i ].connected = 0;
	}

	stopFlag = TRUE;

	gtk_widget_set_sensitive( GTK_WIDGET( stopButton ), FALSE);
	gtk_widget_set_sensitive( GTK_WIDGET( whiteDcButton ), FALSE);
	gtk_widget_set_sensitive( GTK_WIDGET( blackDcButton ), FALSE);

	whitePlayerValue = 0;
	blackPlayerValue = 0;

	whiteTagValid = FALSE;
	blackTagValid = FALSE;


	tempMove.tile[ 0 ] = NULL_MOVE;
	initPosition( &gamePosition );
	printToGui();

	listenToSocket( port, &serverSocket );

	//listen for connections
	gdk_input_add( serverSocket, GDK_INPUT_READ, ( GdkInputFunction )new_connection, NULL );

	gtk_main();

	return 0;
}




