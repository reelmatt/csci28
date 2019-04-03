/*
 * ==========================
 *   FILE: ./ball.c
 * ==========================
 */

//INCLUDES
#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "clock.h"
#include "paddle.h"
#include "court.h"
#include "ball.h"
#include "pong.h"

//CONSTANTS
#define	DFL_SYMBOL	'O'
#define MAX_DELAY   10


static void ball_init(struct ppball *);
static int start_pos();
static int start_dir();
static int delay();
void clear_ball(struct ppball *);
static void draw_ball(struct ppball *);

//struct for the ball
struct ppball {
    int	remain,				//number of balls left
    	x_pos, y_pos,		//positions
        x_dir, y_dir,		//directions
        x_delay, y_delay,	//ticker count
        x_count, y_count;	//delay
    char symbol;			//ball representation
};

/*
 *	new_ball()
 *	Purpose: allocate memory for a new ball struct
 *	 Return: a pointer to the struct allocated, NULL if malloc
 *			 fails.
 */
struct ppball * new_ball()
{
	struct ppball * temp = malloc(sizeof(struct ppball));

	if(temp == NULL)
	{
		wrap_up();
		fprintf(stderr, "./pong: Couldn't allocate memory for a ball.\n");
		exit(1);
	}
	
	temp->remain = NUM_BALLS;		//start with NUM_BALLS remaining
	return temp;
}

/*
 *	ball_init()
 *	Purpose: Initialize a ball struct's values
 *	  Input: bp, pointer to the ball struct to initialize
 */
void ball_init(struct ppball * bp)
{
	//positions
    bp->y_pos = start_pos();
	bp->x_pos = start_pos();

	//directions
	bp->y_dir = start_dir();
	bp->x_dir = start_dir();

	//delay
	bp->y_count = bp->y_delay = delay();
	bp->x_count = bp->x_delay = delay();

	//assign symbol ('O' by default)
	bp->symbol = DFL_SYMBOL;
	
	//lose one ball 'life' every initialization
	bp->remain--;
	
	return;
}

/*
 *	get_balls()
 *	Purpose: Public function to access number of balls remaining
 *	  Input: bp, pointer to a ball struct
 */
int get_balls(struct ppball * bp)
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

/*
 *	refresh_ball()
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
 *	ball_move()
 *	Purpose: Move ball if enough time has passed
 *	  Input: bp, pointer to a ball struct
 *	 Method: The ball has a given delay and a count to keep track when to
 *			 refresh next. If the counter reaches 0, that indicates it is
 *			 time to move the x or y position of the ball. After updating
 *			 the struct values, the counter is reset to the delay and will
 *			 move again after the next delay period.
 */
void ball_move(struct ppball * bp)
{
	int moved = 0 ;
	int y_cur = bp->y_pos ;		// old spot
	int x_cur = bp->x_pos ;		// old spot

	//Check vertical counters
	if ( bp->y_delay > 0 && --bp->y_count == 0 )
	{
		bp->y_pos += bp->y_dir;			// move ball
		bp->y_count = bp->y_delay;		// reset counter
		moved = 1;
	}

	//Check horizontal counters
	if ( bp->x_delay > 0 && --bp->x_count == 0 )
	{
		bp->x_pos += bp->x_dir;			// move ball
		bp->x_count = bp->x_delay;		// reset counter
		moved = 1;
	}

	if (moved)
	{
		mvaddch(y_cur, x_cur, BLANK);				//remove old ball
		draw_ball(bp);
	}
	return;
}

/* 
 *	bounce_or_lose()
 *	Purpose: if a ball hits walls, change its direction
 *	  Input: bp, pointer to a ball struct
 *			 pp, pointer to a paddle struct
 *	 Return: BOUNCE, if hits walls or paddle
 *			 NO_CONTACT, if no bounce/contact
 *			 LOSE, if goes out of play
 */
int bounce_or_lose(struct ppball *bp, struct pppaddle *pp, struct ppcourt *cp)
{
	int	return_val = NO_CONTACT;

	if ( bp->y_pos == (get_top() + 1) )			//top
	{
	    bp->y_dir = 1;
	    return_val = BOUNCE;
	}
	else if ( bp->y_pos == (get_bot() - 1) )	//bottom
	{
		bp->y_dir = -1;
		return_val = BOUNCE;
	}

    if ( bp->x_pos == (get_left() + 1) )		//left
    {
		bp->x_dir = 1;
		return_val = BOUNCE;
    }
	else if ( bp->x_pos == (get_right() - 1) )	//right
	{
	    if( paddle_contact(bp->y_pos, bp->x_pos, pp) == CONTACT )
	    {
            bp->x_dir = -1;			//bounce direction
            bp->x_delay = delay();	//new, random, delay
            bp->y_delay = delay();	//new, random, delay
            return_val = BOUNCE;	    
	    }
	    else
	    {
            clear_ball(bp);
	        return_val = LOSE;
	    }
	}

	return return_val;
}

/*
 *	clear_ball()
 *	Purpose: clear old ball from screen when it goes out of bounds
 *	  Input: bp, pointer to a ball struct
 */
void clear_ball(struct ppball * bp)
{
	bp->y_delay = 0;
	bp->x_delay = 0;
	mvaddch(bp->y_pos, bp->x_pos, BLANK);				//remove old ball
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
 *	start_pos()
 *	Purpose: Randomly pick starting position
 *	 Return: A position within the BORDER of the playing screen
 */
int start_pos()
{
	int pos = rand() % (LINES - BORDER);
	
	return (BORDER + 1 + pos);
}

/*
 *	delay()
 *	Purpose: Randomly pick a delay value
 *	 Return: A number between 0 to MAX_DELAY (non-inclusve)
 */
int delay()
{
    return 1 + (rand() % (MAX_DELAY - 1));
}

/*
 *	random_number()
 *	Purpose: generate a random number between min and max
 *	  Input: min, the lowest possible value
 *			 max, the highest possible value
 *	 Return: the randomly generated number
 */
int random_number(int min, int max)
{
	return ( min + (rand() % max) );
}