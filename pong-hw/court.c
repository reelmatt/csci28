#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include "court.h"
#include "clock.h"
#include "pong.h"
#include "court.h"
#include "ball.h"

#define ROW_SYMBOL '-'
#define COL_SYMBOL '|'

static void print_row(int, int, int);
static void print_col(int, int, int);

struct ppcourt {
	int top, bot, left, right;	//dimensions of court
};

static struct ppcourt court;
/*
struct ppcourt * new_court()
{
	struct ppcourt * court = malloc(sizeof(struct ppcourt));

	if(court == NULL)
	{
		wrap_up();
		fprintf(stderr, "./pong: Couldn't allocate memory for a court.\n");
		exit(1);
	}
	
// 	int top = BORDER;
// 	int left = BORDER;
// 	int bot = LINES - BORDER - 1;	//minus 1 because 0-indexed
// 	int right = COLS - BORDER - 1;	//minus 1 because 0-indexed
	
//	court_init(top, bot, left, right);

	return court;
}*/

void court_init(int top, int bot, int left, int right)
{
	court.top = top;
	court.bot = bot;
	court.left = left;
	court.right = right;
	
	return;
}

int get_right()
{
	return court.right;
}

int get_left()
{
	return court.left;
}

int get_top()
{
	return court.top;
}

int get_bot()
{
	return court.bot;
}


/*
 *	print_court()
 *	Purpose: print the # balls left, time, and walls
 */
void print_court(struct ppcourt * cp, struct ppball * bp)
{
	print_row(court.top, court.left, court.right);
    print_col(court.left, court.top, court.bot);
    print_row(court.bot, court.left, court.right);


//     print_row(cp->top, cp->left, cp->right);
//     print_col(cp->left, cp->top, cp->bot);
//     print_row(cp->bot, cp->left, cp->right);
    
    print_balls(get_balls(bp));
	print_time(); 
    return;
}

/*
 *  print_row()
 *  Purpose: print a row
 *    Input: start, the window row to print at
 */
void print_row(int row, int start, int end)
{
    int i;
    move(row, start);
    for(i = start; i < end; i++)
        addch(ROW_SYMBOL);
    
    return;
}

/*
 *  print_row()
 *  Purpose: print a column
 *    Input: col, the window column to print at
 */
void print_col(int col, int start, int end)
{
    int i;
    for(i = start + 1; i < end; i++)
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
    mvprintw(court.top - 1, court.left, "BALLS LEFT: %2d", balls);
    return;
}

