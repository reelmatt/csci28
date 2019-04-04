/*
 * ===========================================================================
 *   FILE: ./paddle.c
 * ===========================================================================
 * Purpose: Create a paddle for pong and provide an interface to control it.
 *
 * Interface:
 *		new_paddle()		-- allocates memory and inits a new paddle
 *		paddle_up()			-- determines if room to move up, and does so
 *		paddle_down()		-- determines if room to move down, and does so
 *		paddle_contact()	-- determines if ball is touching paddle
 *
 * Internal functions:
 *		paddle_init()		-- initializes paddle's vars, and draws on screen
 *		draw_paddle()		-- draws full paddle from top-to-bottom
 *
 * Notes:
 *		The paddle.c file 
 */

/* INCLUDES */
#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include "paddle.h"
#include "ball.h"
#include "pong.h"
#include "court.h"

/* CONSTANTS */
#define	DFL_SYMBOL	'#'

/* PADDLE STRUCT */
struct pppaddle {
    char pad_char;					// char to draw with
    int pad_top, pad_bot, pad_col;	// positions of paddle
    int pad_mintop, pad_maxbot;		// boundaries
};

/*
 * ===========================================================================
 * INTERNAL FUNCTIONS
 * ===========================================================================
 */
static void draw_paddle();
static void paddle_init(struct pppaddle *, int, int);

/*
 *	draw_paddle()
 *	Purpose: Draw the paddle pointed to by pp
 *	  Input: pp, pointer to a paddle struct
 *	   Note: As mentioned in comments for new_paddle(), a MIN_LINES constant
 *			 defined in pong.c ensures a minimum court height of 3, and
 *			 therefore a paddle height of at least 1. No special cases are
 *			 needed to print a paddle char where one might not exist.
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
 *			 top, the starting (top) position of the paddle
 *			 height, how tall the paddle is
 */
void paddle_init(struct pppaddle * pp, int top, int height)
{	
	pp->pad_char = DFL_SYMBOL;
	pp->pad_mintop = BORDER;
	pp->pad_maxbot = LINES - BORDER;
	
	pp->pad_col = get_right_edge();
	pp->pad_top = top;
	pp->pad_bot = pp->pad_top + height;

    draw_paddle(pp);
    
    return;
}

/*
 * ===========================================================================
 * EXTERNAL INTERFACE
 * ===========================================================================
 */

/*
 *	new_paddle()
 *	Purpose: instantiate a new paddle struct
 *	 Return: a pointer to the paddle that was allocated and initialized
 *	   Note: The window size will be at least 11 lines tall, making the
 *			 court height at least 3 lines tall. This means paddle_height
 *			 will always be 1 char or greater. See draw_paddle() for more.
 */
struct pppaddle * new_paddle()
{
	struct pppaddle * paddle = malloc(sizeof(struct pppaddle));
	
	if(paddle == NULL)
	{
		//close curses
		wrap_up();
		
		//output to stderr and exit
		fprintf(stderr, "./pong: Couldn't allocate memory for a paddle.");
		exit(1);
	}
	
	//-1 for court height to exclude bottom row
	int court_height = get_bot_edge() - get_top_edge() - 1;

	//set paddle size to 1/3 the court size
	int paddle_height = (court_height / 3);

	//set top of paddle to mid-point minus half the paddle height
	int paddle_top = (LINES / 2) - (paddle_height / 2);
	
	paddle_init(paddle, paddle_top, paddle_height);
	return paddle;
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
 *			 pp, pointer to a paddle struct
 *	 Return: CONTACT, if the (y, x) is touching the paddle
 *			 NO_CONTACT, all other cases
 */
int paddle_contact(int y, struct pppaddle * pp)
{
	//within the vertical range of the paddle
	if(y >= pp->pad_top && y <= pp->pad_bot)
	{
		return CONTACT;        
	}
    
    return NO_CONTACT;
}