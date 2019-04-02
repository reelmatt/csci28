#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include "paddle.h"
#include "ball.h"
#include "pong.h"
#include "court.h"
#include "bounce.h"

//#define DEBUG
#define	DFL_SYMBOL	'#'
#define PAD_SIZE 5

struct pppaddle {
    int pad_top, pad_bot, pad_col;
    char pad_char;
    int pad_mintop, pad_maxbot;
};

//static struct pppaddle paddle;
static void draw_paddle();
// static void refresh_paddle(struct pppaddle *);




void draw_paddle(struct pppaddle * pp)
{
    int i;

    for(i = pp->pad_top; i < pp->pad_bot; i++)
        mvaddch(i, pp->pad_col, pp->pad_char);

	#ifdef DEBUG
		move(0, 30);
		printw("pad_top = %.2d, pad_bot = %.2d", pp->pad_top, pp->pad_bot);
	#endif

	move(LINES-1, COLS-1);						//park cursor
    refresh();
    return; 
}

/*
 *	new_paddle()
 *	Purpose: instantiate a new paddle struct
 */
struct pppaddle * new_paddle()
{
	struct pppaddle * temp = malloc(sizeof(struct pppaddle));
	
	if(temp == NULL)
	{
		perror("couldn't make a paddle");
		wrap_up(1);
	}
	
	paddle_init(temp);
	
	return temp;
}

void paddle_init(struct pppaddle * pp)
{	
	pp->pad_char = DFL_SYMBOL;
	pp->pad_mintop = BORDER;
	pp->pad_maxbot = LINES - BORDER;
	pp->pad_col = COLS - BORDER;
	pp->pad_top = (LINES / 2) - (PAD_SIZE / 2);
	pp->pad_bot = pp->pad_top + PAD_SIZE;

    draw_paddle(pp);
    
    return;
}

// void refresh_paddle(struct pppaddle * pp)
// {
// 	
// }

/*
 *	paddle_up()
 *	Purpose: check the position of a paddle, and move up if space
 *	  Input: pp, pointer to a paddle struct
 *	 Method: Check if the top-most part of the paddle is at the border,
 *			 If there is room to move, 
 */
void paddle_up(struct pppaddle * pp)
{
    if( (pp->pad_top - 1) > pp->pad_mintop)
    {
        mvaddch(pp->pad_bot - 1, pp->pad_col, BLANK);
        --pp->pad_top;
        --pp->pad_bot;
        mvaddch(pp->pad_top, pp->pad_col, DFL_SYMBOL);
        move(LINES-1, COLS-1);						//park cursor
//         draw_paddle(pp);
    }
}

void paddle_down(struct pppaddle * pp)
{
    if( (pp->pad_bot + 1) < pp->pad_maxbot)
    {
        mvaddch(pp->pad_top, pp->pad_col, BLANK);
        ++pp->pad_top;
        ++pp->pad_bot;
        mvaddch(pp->pad_bot - 1, pp->pad_col, DFL_SYMBOL);
        move(LINES-1, COLS-1);						//park cursor
//         draw_paddle(pp);
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
    if(y >= pp->pad_top && y <= pp->pad_bot)
    {
        return CONTACT;        
    }
    
    return NO_CONTACT;
}