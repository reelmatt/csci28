#include <curses.h>
#include <unistd.h>
#include "court.h"
#include "clock.h"
#include "pong.h"
#include "court.h"
#include "ball.h"

#define ROW_SYMBOL '-'
#define COL_SYMBOL '|'
#define EXIT_MSG_LEN 16
static void print_row(int);
static void print_col(int);


/*
 *	print_court()
 *	Purpose: print the # balls left, time, and walls
 */
void print_court(struct ppball * bp)
{
    print_row(BORDER);                  //top row
    print_col(BORDER);                  //left edge
    print_row((LINES - BORDER - 1));    //bottom row
    
    print_balls(get_balls(bp));
	print_time(); 
    return;
}

/*
 *  print_row()
 *  Purpose: print a row
 *    Input: start, the window row to print at
 */
void print_row(int start)
{
    int i;
    move(start, BORDER);
    for(i = BORDER; i < COLS - BORDER - 1; i++)
        addch(ROW_SYMBOL);
    
    return;
}

/*
 *  print_row()
 *  Purpose: print a column
 *    Input: col, the window column to print at
 */
void print_col(int col)
{
    int i;
    for(i = BORDER + 1; i < LINES - BORDER - 1; i++)
        mvaddch(i, col, COL_SYMBOL);
    
    return; 
}

/*
 *	print_balls()
 *	Purpose: print the number of balls left to play
 *	  Input: balls, the number of balls left
 */
void print_balls(int balls)
{
    mvprintw(HEADER, BORDER, "BALLS LEFT: %2d", balls);
    return;
}

void exit_message()
{
	int y = (LINES / 2);
	int x = (COLS / 2) - (EXIT_MSG_LEN / 2);
	standout();
	mvprintw(y, x, "You lasted %.2d:%.2d", get_mins(), get_secs());
	standend();
	refresh();
	sleep(2);
}