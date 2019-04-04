/*
 * ===========================================================================
 *   FILE: ./court.c
 * ===========================================================================
 * Purpose: Create a court for pong, and print it to the screen
 *
 * Interface:
 
 *		new_ball()			-- allocates memory for a new ball
 *		serve()				-- inits new values for a ball and draws it
 *		ball_move()			-- 
 *		bounce_or_lose()	-- 
 *		get_balls_left()	-- returns the number of balls (lives) left
 *
 * Internal functions:
 *		ball_init()			-- (re-)initializes balls vars
 *		start_pos()			-- generates random starting position
 *		start_dir()			-- generates random starting direction
 *		delay()				-- generates random delay value
 *		clear_ball()		-- stops ball and clears it from the screen
 *		draw_ball()			-- draws the ball in a new position
 *
 * Notes:
 *
 */

/* INCLUDES */
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include "court.h"
#include "clock.h"
#include "pong.h"
#include "court.h"
#include "ball.h"

/* CONSTANTS */
#define ROW_SYMBOL '-'
#define COL_SYMBOL '|'
#define TIME_FORMAT "TOTAL TIME: %.2d:%.2d"	// e.g. TOTAL TIME: 02:09
#define TIME_LEN 17 						// length of outputted time string

/* COURT STRUCT */
struct ppcourt {
	int top, right, left, bot;	//dimensions of court
};

static struct ppcourt court;


/*
 * ===========================================================================
 * INTERNAL FUNCTIONS
 * ===========================================================================
 */
static void print_row(int, int, int);
static void print_col(int, int, int);

/*
 *  print_row()
 *  Purpose: print a row
 *    Input: row, the window row to print at
 *			 start, the column to start at
 *			 end, the column to end at
 *	   Note: The for loop goes until i <= end in order to print the
 *			 right-most row symbol.
 */
void print_row(int row, int start, int end)
{
    int i;
    move(row, start);
    for(i = start; i <= end; i++)
        addch(ROW_SYMBOL);
    
    return;
}

/*
 *  print_row()
 *  Purpose: print a column
 *    Input: col, the window column to print at
 *			 start, the row to start at
 *			 end, the row to end at
 *	   Note: The for loop starts with +1 to skip the row 
 */
void print_col(int col, int start, int end)
{
    int i;
    for(i = start; i < end; i++)
        mvaddch(i, col, COL_SYMBOL);
    
    return; 
}

/*
 * ===========================================================================
 * EXTERNAL INTERFACE
 * ===========================================================================
 */

/*
 *	court_init()
 *	Purpose: Initialize the static court object with row/col values
 */
void court_init(int top, int right, int bot, int left)
{
	court.top = top;
	court.right = right;
	court.left = left;
	court.bot = bot;
	
	return;
}

/* get_right_edge() -- return the position of the right column */
int get_right_edge()
{
	return court.right;
}

/* get_left_edge() -- return the position of the left column */
int get_left_edge()
{
	return court.left;
}

/* get_top_edge() -- return the position of the top row */
int get_top_edge()
{
	return court.top;
}

/* get_bot_edge() -- return the position of the bottom row */
int get_bot_edge()
{
	return court.bot;
}


/*
 *	print_court()
 *	Purpose: print the # balls left, time, and walls
 *	   Note: The column has +1 added to court.top so the column
 *			 doesn't overwrite the top row.
 */
void print_court(int balls)
{
	print_row(court.top, court.left, court.right);
    print_col(court.left, court.top + 1, court.bot);
    print_row(court.bot, court.left, court.right);
    
    print_balls(balls);
	print_time(); 
	park_cursor();
	refresh();
    return;
}



/*
 *	print_time()
 *	Purpose: Print elapsed time, right-adjusted above top border
 */
void print_time()
{
	move((get_top_edge() - 1), (get_right_edge() - TIME_LEN));
    printw(TIME_FORMAT, get_mins(), get_secs());
	park_cursor();
    refresh();
    return;
}

/*
 *	print_balls()
 *	Purpose: Print the number of balls left to play, left-adjusted above
 *			 top border
 *	  Input: balls, the number of balls left
 */
void print_balls(int balls)
{
    mvprintw(court.top - 1, court.left, "BALLS LEFT: %2d", balls);
    return;
}

