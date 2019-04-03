/*
 * ==========================
 *   FILE: ./paddle.c
 * ==========================
 */

//INCLUDES
#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include "paddle.h"
#include "ball.h"
#include "pong.h"
#include "court.h"
#include "bounce.h"

//CONSTANTS
#define	DFL_SYMBOL	'#'

//PADDLE STRUCT
struct pppaddle {
    int pad_top, pad_bot, pad_col;
    char pad_char;
    int pad_mintop, pad_maxbot;
};

//STATIC FUNCTIONS
static void draw_paddle();
static void paddle_init(struct pppaddle *, int, int);

/*
 *	new_paddle()
 *	Purpose: instantiate a new paddle struct
 */
struct pppaddle * new_paddle()
{
	struct pppaddle * temp = malloc(sizeof(struct pppaddle));
	
	if(temp == NULL)
	{
		wrap_up();
		fprintf(stderr, "./pong: Couldn't allocate memory for a paddle.");
		exit(1);
	}
	
	int court_height = LINES - (BORDER * 2);	//get court height
	int size = (court_height / 3);				//set paddle size to 1/3
	int start_pos = (LINES / 2) - (size / 2);	//set start to the mid-point
	
	paddle_init(temp, start_pos, size);
	
	return temp;
}

/*
 *	draw_paddle()
 *	Purpose: Draw the paddle pointed to by pp
 *	  Input: pp, pointer to a paddle struct
 */
void draw_paddle(struct pppaddle * pp)
{
    int i;

    for(i = pp->pad_top; i < pp->pad_bot; i++)
        mvaddch(i, pp->pad_col, pp->pad_char);

	park_cursor();
    refresh();
    return; 
}

/*
 *	paddle_init()
 *	Purpose: instantiate a new paddle struct
 *	  Input: pp, pointer to a paddle struct
 *			 pos, the starting (top) position of the paddle
 *			 range, how tall the paddle is
 */
void paddle_init(struct pppaddle * pp, int pos, int range)
{	
	pp->pad_char = DFL_SYMBOL;
	pp->pad_mintop = BORDER;
	pp->pad_maxbot = LINES - BORDER;
	
	pp->pad_col = COLS - BORDER - 1;
	pp->pad_top = pos;
	pp->pad_bot = pp->pad_top + range;

    draw_paddle(pp);
    
    return;
}


/*
 *	paddle_up()
 *	Purpose: check the position of a paddle, and move up if space
 *	  Input: pp, pointer to a paddle struct
 *	 Method: Check if the top-most part of the paddle is at the border.
 *			 If there is room to move, BLANK the bottom-most char and draw
 *			 a new one at the new top value.
 */
void paddle_up(struct pppaddle * pp)
{
	//If moved by 1, would it be at 'mintop'?
    if( (pp->pad_top - 1) > pp->pad_mintop)
    {
        mvaddch(pp->pad_bot - 1, pp->pad_col, BLANK);
        --pp->pad_top;
        --pp->pad_bot;
        mvaddch(pp->pad_top, pp->pad_col, DFL_SYMBOL);

		park_cursor();
		refresh();
    }
}

/*
 *	paddle_down()
 *	Purpose: check the position of a paddle, and move down if space
 *	  Input: pp, pointer to a paddle struct
 *	 Method: Check if the bottom-most part of the paddle is at the border.
 *			 If there is room to move, BLANK the top-most char and draw
 *			 a new one at the new bottom value.
 */
void paddle_down(struct pppaddle * pp)
{
	//If moved by 1, would it be at 'maxbot'?
    if( (pp->pad_bot + 1) < pp->pad_maxbot)
    {
        mvaddch(pp->pad_top, pp->pad_col, BLANK);
        ++pp->pad_top;
        ++pp->pad_bot;
        mvaddch(pp->pad_bot - 1, pp->pad_col, DFL_SYMBOL);
        
		park_cursor();
		refresh();
    }
}

/*
 *	paddle_contact()
 *	Purpose: Determine if a ball's current (y, x) position hits a paddle
 *	  Input: y, ball's vertical coordinate
 *			 x, ball's horizontal coordinate
 *			 pp, pointer to a paddle struct
 *	 Return: CONTACT, if the (y, x) is touching the paddle
 *			 NO_CONTACT, all other cases
 */
int paddle_contact(int y, int x, struct pppaddle * pp)
{
    //in the right-most col
	//if(x == (pp->pad_col))
	//{
		//within the vertical range of the paddle
		if(y >= pp->pad_top && y <= pp->pad_bot)
		{
			return CONTACT;        
		}
   // }
    
    return NO_CONTACT;
}