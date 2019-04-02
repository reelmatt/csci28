#include <curses.h>
#include "court.h"
#include "clock.h"
#include "pong.h"

#define ROW_SYMBOL '-'
#define COL_SYMBOL '|'
static void print_row();
static void print_col();

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

void print_col()
{
    int i;
    for(i = BORDER + 1; i < LINES - BORDER - 1; i++)
        mvaddch(i, BORDER, COL_SYMBOL);
    
    return; 
}

/*
 *	print_court()
 *	Purpose: print the # balls left, time, and walls
 */
void print_court()
{
    print_balls(NUM_BALLS);
    print_time(); 
    
    print_row(BORDER);                  //top row
    print_col();                        //left edge
    print_row((LINES - BORDER - 1));    //bottom row
    
    return;
}

/*
 *	print_balls()
 *	Purpose: print the number of balls left to play
 */
void print_balls(int balls)
{
    move(BORDER - 1, BORDER);
    printw("%s: %2d", "BALLS LEFT", balls);
}

void exit_message()
{
	printw("You lasted %.2d:%.2d");
}