/*
 * ===========================================================================
 *   FILE: ./court.c
 * ===========================================================================
 * Purpose: Create a court for pong, and print it to the screen.
 *
 * Interface:
 *		court_init()		-- Init the static court with row/col values
 *		print_court()		-- Print the # balls left, time, and walls
 *		print_balls()		-- Print the number of balls left to play
 *		print_time()		-- Print elapsed time
 *		get_top_edge()		-- Return the position of the top row
 *		get_right_edge()	-- Return the position of the right column
 *		get_bot_edge()		-- Return the position of the bottom row
 *		get_left_edge()		-- Return the position of the left column
 *
 * Internal functions:
 *		print_row()			-- Print a row
 *		print_col()			-- Print a column
 *
 * Notes:
 *		The court is file-scoped to court.c. The clock is responsible for
 *		storing where the borders to the game are, printing the borders,
 *		and updating the two headers tracking game progress -- BALLS LEFT
 *		and TOTAL TIME.
 */

/* INCLUDES */
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include "ball.h"
#include "clock.h"
#include "court.h"
#include "pong.h"

/* CONSTANTS */
#define ROW_SYMBOL '-'
#define COL_SYMBOL '|'
#define TIME_FORMAT "TOTAL TIME: %.2d:%.2d"	// e.g. TOTAL TIME: 02:09
#define TIME_LEN 17 						// length of outputted time string

/* COURT STRUCT */
struct ppcourt {
	int top, right, bot, left;	//dimensions of court
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
	court.bot = bot;
	court.left = left;
	
	return;
}

/*
 *	print_court()
 *	Purpose: print the # balls left, time, and walls
 *	   Note: The column has +1 added to court.top so the column
 *			 doesn't overwrite the top row.
 *     Note: There are no calls to park_cursor() or refresh() as
 *           these occur in both print_balls() and print_time()
 *           because those calls will happen more frequently than
 *           print_court(). As such, explicit calls to park_cursor()
 *           and refresh() are not necessary in this function as they
 *           will have already happened.
 */
void print_court(int balls)
{
	print_row(court.top, court.left, court.right);
    print_col(court.left, court.top + 1, court.bot);
    print_row(court.bot, court.left, court.right);
    
    print_balls(balls);
	print_time();
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
    park_cursor();
    refresh();
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