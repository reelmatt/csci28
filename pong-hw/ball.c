/*
 * ==========================
 *   FILE: ./ball.c
 * ==========================
 * Purpose: Create and operate a ball object for a game of pong
 *
 * Internal functions:
 *		ball_init()			-- (re-)initializes balls vars
 *		draw_ball()			-- draws the ball in a new position
 *		rand_number()		-- generates random number between a min and max
 *		start_dir()			-- generates random starting direction
 *
 * Interface:
 *		new_ball()			-- allocates memory for a new ball
 *		ball_move()			-- move ball if enough time has passed
 *		bounce_or_lose()	-- detect when ball hits walls/paddle or misses
 *		serve()				-- inits new values for a ball and draws it
 *		get_balls_left()	-- returns the number of balls (lives) left
 *
 */

/* INCLUDES */
#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "clock.h"
#include "paddle.h"
#include "court.h"
#include "ball.h"
#include "pong.h"

/* CONSTANTS */
#define	DFL_SYMBOL	'O'
#define MAX_DELAY   10

/* BALL STRUCT */
struct ppball {
    int	remain,				// number of balls left
    	x_pos, y_pos,		// positions
        x_dir, y_dir,		// directions
        x_delay, y_delay,	// ticker count
        x_count, y_count;	// delay
    char symbol;			// ball representation
};

/*
 * ===========================================================================
 * INTERNAL FUNCTIONS
 * ===========================================================================
 */
static void ball_init(struct ppball *);
static void draw_ball(struct ppball *);
static int start_dir();
static int rand_number(int, int);

/*
 *	ball_init()
 *	Purpose: Initialize a ball struct's values
 *	  Input: bp, pointer to the ball struct to initialize
 *     Note: For the position values, +/- 1 keeps the ball within the court
 *           boundaries. The functions to retrieve the edge values return
 *           the column or row the borders are drawn; the ball should start
 *           *within* those boundaries.
 *     Note: The x_count and x_delay start with a random value between 1 and
 *           half the MAX_DELAY. A default terminal window will often be
 *           wider than it is tall, so 
 */
void ball_init(struct ppball * bp)
{
	// positions
    bp->y_pos = rand_number(get_top_edge() + 1, get_bot_edge() - 1);
	bp->x_pos = rand_number(get_left_edge() + 1, get_right_edge() - 1);

	// directions
	bp->y_dir = start_dir();
	bp->x_dir = start_dir();

	// delay
	bp->y_count = bp->y_delay = rand_number(1, MAX_DELAY);
	bp->x_count = bp->x_delay = rand_number(1, (MAX_DELAY / 2));

	// assign symbol ('O' by default)
	bp->symbol = DFL_SYMBOL;
	
	// lose one ball (life) every initialization
	bp->remain--;
	
	return;
}

/*
 *	draw_ball()
 *	Purpose: 
 *	  Input: bp, pointer to the ball struct to draw
 */
void draw_ball(struct ppball * bp)
{
	mvaddch(bp->y_pos, bp->x_pos, bp->symbol);	//print new ball
	park_cursor();
	refresh();
	
	return;
}

/*
 *	random_number()
 *	Purpose: generate a random number between min and max
 *	  Input: min, the lowest possible value
 *			 max, the highest possible value
 *	 Return: the randomly generated number
 */
int rand_number(int min, int max)
{
	return ( (rand() % (max - min)) + min );
}

/*
 *	start_dir()
 *	Purpose: Randomly pick starting direction
 *	 Return: Either -1 or 1
 */
int start_dir()
{
    if( (rand() % 2) == 0)
        return -1;
    else
        return 1;
}

/*
 * ===========================================================================
 * EXTERNAL INTERFACE
 * ===========================================================================
 */

/*
 *	new_ball()
 *	Purpose: allocate memory for a new ball struct
 *	 Return: a pointer to the struct allocated, NULL if malloc
 *			 fails.
 */
struct ppball * new_ball()
{
	struct ppball * ball = malloc(sizeof(struct ppball));

	if(ball == NULL)
	{
		// close curses
		wrap_up();
		
		// output to stderr and exit
		fprintf(stderr, "./pong: Couldn't allocate memory for a ball.\n");
		exit(1);
	}
	
	ball->remain = NUM_BALLS;		//start with NUM_BALLS remaining
	return ball;
}

/*
 *	ball_move()
 *	Purpose: Move ball if enough time has passed
 *	  Input: bp, pointer to a ball struct
 *	 Method: The ball has a given delay and a count to keep track when to
 *			 refresh next. If the counter reaches 0, that indicates it is
 *			 time to move the x or y position of the ball. After updating
 *			 the struct values, the counter is reset to the delay and will
 *			 move again after the next delay period.
 *     Note: Most of the main logic in this function was copied, unchanged,
 *           from the bounce2d.c file on the course site.
 */
void ball_move(struct ppball * bp)
{
	int moved = 0 ;
	int y_cur = bp->y_pos;				// old spot
	int x_cur = bp->x_pos;				// old spot

	// Check vertical counters
	if ( bp->y_delay > 0 && --bp->y_count == 0 )
	{
		bp->y_pos += bp->y_dir;			// move ball
		bp->y_count = bp->y_delay;		// reset counter
		moved = 1;
	}

	// Check horizontal counters
	if ( bp->x_delay > 0 && --bp->x_count == 0 )
	{
		bp->x_pos += bp->x_dir;			// move ball
		bp->x_count = bp->x_delay;		// reset counter
		moved = 1;
	}

	if (moved)
	{
		mvaddch(y_cur, x_cur, BLANK);	// remove old ball
		draw_ball(bp);					// draw new one
	}
	
	return;
}

/* 
 *	bounce_or_lose()
 *	Purpose: Detect when a ball hits outer walls/paddle, or misses
 *	  Input: bp, pointer to a ball struct
 *			 pp, pointer to a paddle struct
 *	 Return: BOUNCE, if hits walls or paddle
 *			 NO_CONTACT, if no bounce/contact
 *			 LOSE, if goes out of play
 *	   Note: Values are +/- 1 from the edges to account for where the border
 *			 is, and what will cause this function to return BOUNCE of LOSE.
 *			 We want to detect when the ball is just inside the borders and
 *			 whether or not that will bounce. If the +/- 1 values were not
 *			 present, the ball would overwrite where the borders are before
 *			 bouncing or not.
 *     Note: Most of the main logic in this function was copied, unchanged,
 *           from the bounce2d.c file on the course site. Changes were made
 *           for detecting bounces on the right-side of the court (where
 *           the paddle is).
 */
int bounce_or_lose(struct ppball *bp, struct pppaddle *pp)
{
	int	return_val = NO_CONTACT;

	if ( bp->y_pos == (get_top_edge() + 1) )			// top
	{
	    bp->y_dir = 1;
	    return_val = BOUNCE;
	}
	else if ( bp->y_pos == (get_bot_edge() - 1) )		// bottom
	{
		bp->y_dir = -1;
		return_val = BOUNCE;
	}

    if ( bp->x_pos == (get_left_edge() + 1) )			// left
    {
		bp->x_dir = 1;
		return_val = BOUNCE;
    }
	else if ( bp->x_pos == (get_right_edge() - 1) )		// right
	{
	    if( paddle_contact(bp->y_pos, pp) == CONTACT )	// it hit the paddle
	    {
            // new, random, delay (keep horizontal movement faster)
            bp->x_delay = rand_number(1, (MAX_DELAY / 2));
            bp->y_delay = rand_number(1, MAX_DELAY);
            bp->x_dir = -1;
            return_val = BOUNCE;	    
	    }
	    else
	    {
	    	mvaddch(bp->y_pos, bp->x_pos, BLANK);		// remove old ball
	        return_val = LOSE;
	    }
	}

	return return_val;
}

/*
 *	get_balls()
 *	Purpose: Public function to access number of balls remaining
 *	  Input: bp, pointer to a ball struct
 *	 Return: Number of balls remaining in ball pointed to by 'bp'
 */
int get_balls_left(struct ppball * bp)
{
	return bp->remain;
}

/*
 *	serve()
 *	Purpose: Initialize a ball with new values and draw it
 *	  Input: bp, pointer to the ball struct to serve
 */
void serve(struct ppball * bp)
{   
    ball_init(bp);
    draw_ball(bp);
	print_balls(bp->remain);
    
    return;
}
