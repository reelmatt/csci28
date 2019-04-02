#include <curses.h>
#include <signal.h>
#include "bounce.h"
#include <stdlib.h>
#include <unistd.h>
#include "clock.h"
#include "paddle.h"
#include "court.h"
#include "ball.h"
#include "pong.h"

#define	DFL_SYMBOL	'O'
#define	X_INIT		10		/* starting col		*/
#define	Y_INIT		10		/* starting row		*/
#define MAX_DELAY   10

//#define DEBUG

static int start_dir();
static int delay();
void clear_ball(struct ppball *);
static void draw_ball(struct ppball *);

//DEBUGGING FUNCTIONS
void print_ball_loc(struct ppball *);
void print_ball_dir(struct ppball *);

/*
 *	struct for the ball
 */
struct ppball {
    int	x_pos, x_dir,
        y_pos, y_dir,
        y_delay, y_count,
        x_delay, x_count;
    char symbol;
} ;

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
		perror("couldn't make a ball");
		wrap_up(1);
	}
	
	return temp;
}

/*
 *	ball_init()
 *	Purpose: Initialize a ball struct's values
 *	  Input: bp, pointer to the ball struct to initialize
 */
void ball_init(struct ppball * bp)
{
    bp->y_pos = start_pos();
	bp->x_pos = start_pos();
	bp->y_dir = start_dir();
	bp->x_dir = start_dir();
	bp->y_count = bp->y_delay = delay();
	bp->x_count = bp->x_delay = delay();
	bp->symbol = DFL_SYMBOL;
	
	return;
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
	move(LINES-1, COLS-1);						//park cursor
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
 *	  Input: bp, pointer to the ball struct
 *			 pp, pointer to the paddle struct
 *	 Return: BOUNCE, if hits walls or paddle
 *			 NO_CONTACT, if no bounce/contact
 *			 LOSE, if goes out of play
 */
int bounce_or_lose(struct ppball *bp, struct pppaddle *pp)
{
	int	return_val = NO_CONTACT;

	if ( bp->y_pos == (BORDER + 1) )                	//top
	{
	    bp->y_dir = 1;
	    return_val = BOUNCE;
	}
	else if ( bp->y_pos == (LINES - 1 - BORDER - 1) )   //bottom
	{
		bp->y_dir = -1;
		return_val = BOUNCE;
	}

    if ( bp->x_pos == (BORDER + 1) )                    //left
    {
		bp->x_dir = 1;
		return_val = BOUNCE;
    }
	else if ( bp->x_pos == (COLS - BORDER - 1) )        //right
	{
	    if( paddle_contact(bp->y_pos, bp->x_pos, pp) == CONTACT )
	    {
            bp->x_dir = -1;
            bp->x_delay = delay();
            bp->y_delay = delay();
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

void clear_ball(struct ppball * bp)
{
	bp->y_delay = 0;
	bp->x_delay = 0;
	mvaddch(bp->y_pos, bp->x_pos, BLANK);				//remove old ball
}

/* DEBUGGING FUNCTIONS */
void go_fast(struct ppball * bp)
{
    bp->x_delay--;
    bp->y_delay--;
    
    print_ball_dir(bp);
}

/* DEBUGGING FUNCTION */
void go_slow(struct ppball * bp)
{
    bp->x_delay++;
    bp->y_delay++;
    
    print_ball_dir(bp);
}

void print_ball_loc(struct ppball * bp)
{
    move(0, 1);
    printw("Y_pos = %2d and X_pos = %2d", bp->y_pos, bp->x_pos);
    refresh();
}

void print_ball_dir(struct ppball *bp)
{
	move(1, 1);
    printw("Y_DELAY = %.2d and X_DELAY = %.2d", bp->y_dir, bp->x_dir);
}