/*

   Basic Snake game.

   NOTE: Controls are not 100% responsive.

*/

#define BOARD_LENGTH 24
#define BOARD_SIZE ( BOARD_LENGTH * BOARD_LENGTH )
#define BOARD_Y 100

#define SQUARE_FREE 0
#define SQUARE_ROCK 1
#define SQUARE_FOOD 2
#define SQUARE_SNAKEHEAD 3
#define SQUARE_SNAKEMOVELEFT 4
#define SQUARE_SNAKEMOVERIGHT 5
#define SQUARE_SNAKEMOVEUP 6
#define SQUARE_SNAKEMOVEDOWN 7
#define SQUARE_CRASH 8
#define SQUARE_WIDTH 8

#define DIRECTION_UP 0
#define DIRECTION_DOWN 1
#define DIRECTION_LEFT 2
#define DIRECTION_RIGHT 3

#define FOOD_POINTS 5

class Game {
   CDC* dc;
   I8 board[ BOARD_LENGTH ][ BOARD_LENGTH ];
   I64 shuffle[ BOARD_SIZE ];
   I64 snake_row;
   I64 snake_col;
   I64 snake_tail_row;
   I64 snake_tail_col;
   I64 direction;
   I64 score;
   I64 high_score;
   I64 last_key;
   I64 shuffle_size;
   I64 shuffle_left;
   Bool done;
   Bool quit;
   Bool restart;
   Bool game_restarted;
   Bool direction_changed;
};

U0 Draw( Game* game ) {
   DCFill();
   // Board.
   I64 start_x =
      ( Fs->pix_width / 2 ) -
      ( BOARD_LENGTH * SQUARE_WIDTH / 2 );
   I64 i;
   I64 k;
   I64 y = BOARD_Y;
   game->dc->color = LTGRAY;
   GrRect( game->dc, start_x, y, BOARD_LENGTH * SQUARE_WIDTH,
      BOARD_LENGTH * SQUARE_WIDTH );
   for ( i = 0; i < BOARD_LENGTH; ++i ) {
      for ( k = 0; k < BOARD_LENGTH; ++k ) {
         if ( game->board[ i ][ k ] != SQUARE_FREE ) {
            switch ( game->board[ i ][ k ] ) {
            case SQUARE_ROCK:
               game->dc->color = BLACK;
               break;
            case SQUARE_FOOD:
               game->dc->color = YELLOW;
               break;
            case SQUARE_SNAKEHEAD:
               game->dc->color = LTGREEN;
               break;
            case SQUARE_SNAKEMOVEUP:
            case SQUARE_SNAKEMOVEDOWN:
            case SQUARE_SNAKEMOVELEFT:
            case SQUARE_SNAKEMOVERIGHT:
               game->dc->color = GREEN;
               break;
            case SQUARE_CRASH:
               game->dc->color = RED;
               break;
            }
            GrRect( game->dc, start_x + k * SQUARE_WIDTH, y, SQUARE_WIDTH,
               SQUARE_WIDTH );
         }
      }
      y += SQUARE_WIDTH;
   }
   // HUD.
   y += 5;
   if ( game->done ) {
      GrPrint( game->dc, start_x, y,
        "Score: %d (Game over)", game->score );
   }
   else {
      GrPrint( game->dc, start_x, y,
        "Score: %d", game->score );
   }
   y += FONT_HEIGHT + 5;
   if ( game->high_score > 0 ) {
      GrPrint( game->dc, start_x, y,
         "High score: %d", game->high_score );
      y += FONT_HEIGHT + 5;
   }
   y += 10;
   if ( game->done ) {
      GrPrint( game->dc, start_x, y,
         "To play again, press Enter" );
      y += FONT_HEIGHT + 5;
   }
   GrPrint( game->dc, start_x, y,
      "To quit, press Esc" );
}

U0 SpawnFood( Game* game ) {
   I8* board = &game->board ( I8* );
   while ( TRUE ) {
      I64 pick = RandU64() % game->shuffle_left;
      I64 picked_square = game->shuffle[ pick ];
      game->shuffle[ pick ] = game->shuffle[ game->shuffle_left - 1 ];
      --game->shuffle_left;
      if ( game->shuffle_left == 0 ) {
         game->shuffle_left = game->shuffle_size;
      }
      if ( board[ picked_square ] == SQUARE_FREE ) {
         board[ picked_square ] = SQUARE_FOOD;
         return;
      }
   }
}

U0 Move( Game* game ) {
   I64 square;
   I64 snake_row = game->snake_row;
   I64 snake_col = game->snake_col;
   switch ( game->direction ) {
   case DIRECTION_UP:
      square = SQUARE_SNAKEMOVEUP;
      --snake_row;
      break;
   case DIRECTION_DOWN:
      square = SQUARE_SNAKEMOVEDOWN;
      ++snake_row;
      break;
   case DIRECTION_LEFT:
      square = SQUARE_SNAKEMOVELEFT;
      --snake_col;
      break;
   case DIRECTION_RIGHT:
      square = SQUARE_SNAKEMOVERIGHT;
      ++snake_col;
   }
   game->board[ game->snake_row ][ game->snake_col ] = square;
   game->snake_row = snake_row;
   game->snake_col = snake_col;
   // Tail.
   if ( game->board[ snake_row ][ snake_col ] != SQUARE_FOOD ) {
      I64 snake_tail_row = game->snake_tail_row;
      I64 snake_tail_col = game->snake_tail_col;
      switch ( game->board[ snake_tail_row ][ snake_tail_col ] ) {
      case SQUARE_SNAKEMOVEUP:
         --snake_tail_row;
         break;
      case SQUARE_SNAKEMOVEDOWN:
         ++snake_tail_row;
         break;
      case SQUARE_SNAKEMOVELEFT:
         --snake_tail_col;
         break;
      case SQUARE_SNAKEMOVERIGHT:
         ++snake_tail_col;
      }
      game->board[ game->snake_tail_row ][ game->snake_tail_col ] =
         SQUARE_FREE;
      game->snake_tail_row = snake_tail_row;
      game->snake_tail_col = snake_tail_col;
   }
   switch ( game->board[ snake_row ][ snake_col ] ) {
   case SQUARE_FREE:
      game->board[ snake_row ][ snake_col ] = SQUARE_SNAKEHEAD;
      break;
   case SQUARE_FOOD:
      game->board[ snake_row ][ snake_col ] = SQUARE_SNAKEHEAD;
      game->score += FOOD_POINTS;
      SpawnFood( game );
      break;
   default:
      game->board[ snake_row ][ snake_col ] = SQUARE_CRASH;
      game->done = TRUE;
   }
}

U0 ReadInput( Game* game ) {
   game->direction_changed = FALSE;
   I64 scan_code;
   I64 msg = ScanMsg( NULL, &scan_code );
   if ( msg == MSG_KEY_DOWN ) {
      I64 key = scan_code & 0xFF;
      if ( key != game->last_key ) {
         switch ( key ) {
         case SC_CURSOR_UP:
            // Going backwards is forbidden.
            if ( game->direction != DIRECTION_DOWN ) {
               game->direction = DIRECTION_UP;
               game->direction_changed = TRUE;
            }
            break;
         case SC_CURSOR_DOWN:
            if ( game->direction != DIRECTION_UP ) {
               game->direction = DIRECTION_DOWN;
               game->direction_changed = TRUE;
            }
            break;
         case SC_CURSOR_LEFT:
            if ( game->direction != DIRECTION_RIGHT ) {
               game->direction = DIRECTION_LEFT;
               game->direction_changed = TRUE;
            }
            break;
         case SC_CURSOR_RIGHT:
            if ( game->direction != DIRECTION_LEFT ) {
               game->direction = DIRECTION_RIGHT;
               game->direction_changed = TRUE;
            }
            break;
         case SC_ESC:
            game->quit = TRUE;
            break;
         case SC_ENTER:
            if ( game->done ) {
               game->restart = TRUE;
            }
         }
         game->last_key = key;
      }
   }
   else if ( msg == MSG_KEY_UP ) {
      if ( game->last_key == SC_ENTER ) {
         game->last_key = 0;
      }
   }
}

U0 RunStage( Game* game ) {
   SpawnFood( game );
   U64 time = 0;
   while ( TRUE ) {
      ReadInput( game );
      if ( game->quit ) {
         break;
      }
      if ( time % 100 == 0 || game->direction_changed ) {
         Move( game );
         Draw( game );
         if ( game->done ) {
            break;
         }
         if ( game->direction_changed ) {
            time += 100 - ( time % 100 );
         }
      }
      Sleep( 10 );
      time += 10;
   }
   // Update high score.
   if ( game->score > game->high_score ) {
      game->high_score = game->score;
      Draw( game );
   }
}

U0 InitStage( Game* game ) {
   // Board.
   I64 i;
   I64 k;
   for ( i = 0; i < BOARD_LENGTH; ++i ) {
      for ( k = 0; k < BOARD_LENGTH; ++k ) {
         game->board[ i ][ k ] = SQUARE_FREE;
      }
   }
   // Snake.
   game->board[ 19 ][ 16 ] = SQUARE_SNAKEHEAD;
   game->board[ 20 ][ 16 ] = SQUARE_SNAKEMOVEUP;
   game->board[ 21 ][ 16 ] = SQUARE_SNAKEMOVEUP;
   game->snake_row = 19;
   game->snake_col = 16;
   game->snake_tail_row = 21;
   game->snake_tail_col = 16;
   // Border.
   for ( i = 0; i < BOARD_LENGTH; ++i ) {
      game->board[ 0 ][ i ] = SQUARE_ROCK;
      game->board[ i ][ 0 ] = SQUARE_ROCK;
      game->board[ BOARD_LENGTH - 1 ][ BOARD_LENGTH - 1 - i ] = SQUARE_ROCK;
      game->board[ BOARD_LENGTH - 1 - i ][ BOARD_LENGTH - 1 ] = SQUARE_ROCK;
   }
   // Other.
   game->direction = DIRECTION_UP;
   game->direction_changed = FALSE;
   game->score = 0;
   game->last_key = 0;
   game->done = FALSE;
   game->restart = FALSE;
   // Shuffle. Initialize only once.
   if ( game->shuffle_size == 0 ) {
      I8* board = &game->board;
      for ( i = 0; i < BOARD_SIZE; ++i ) {
         if ( board[ i ] != SQUARE_ROCK ) {
            game->shuffle[ game->shuffle_size ] = i;
            ++game->shuffle_size;
         }
      }
   }
   game->shuffle_left = game->shuffle_size;
}

U0 Run( Game* game ) {
   while ( TRUE ) {
      InitStage( game );
      RunStage( game );
      while ( ! game->restart ) {
         ReadInput( game );
         if ( game->quit ) {
            return;
         }
         Sleep( 10 );
      }
   }
}

U0 InitGame( Game* game, CDC* dc ) {
   game->dc = dc;
   game->quit = FALSE;
   game->high_score = 0;
   game->shuffle_size = 0;
}

U0 Snake() {
   WinMax();
   WinBorder();
   DocClear();
   Game game;
   InitGame( &game, gr.dc );
   Run( &game );
   FlushMsgs();
   DCFill();
}

Snake();