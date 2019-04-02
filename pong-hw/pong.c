#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "alarmlib.h"
#include "bounce.h"
#include "ball.h"
#include "court.h"
#include "paddle.h"
#include "clock.h"
#include "pong.h"



#define DEBUG

static void set_up();
// static void wrap_up(int);
static void up_paddle();
static void down_paddle();
struct pppaddle * new_paddle();
struct ppball * new_ball();

int balls_left = NUM_BALLS;
static struct pppaddle * paddle;	//the paddle
static struct ppball * ball;		//the ball


int main ()
{
    int c;
    set_up();
    serve(ball);
    
    while( balls_left > 0 && (c = getch()) != 'Q')
    {
        if(c == 'k')
            up_paddle();
        else if (c == 'm')
            down_paddle();

        #ifdef DEBUG
			else if(c == 'f')
				go_fast(ball);
			else if (c == 's')
				go_slow(ball);
		#endif		
    }
    
    wrap_up(0);
    return 0;
}

void up_paddle()
{
    paddle_up(paddle);
    
    if( bounce_or_lose(ball, paddle) == LOSE)
    {
    	balls_left--;
		print_balls(balls_left);
		serve(ball);
    }
    
    return;
}

void down_paddle()
{
    paddle_down(paddle);
    
    if( bounce_or_lose(ball, paddle) == LOSE)
    {
    	balls_left--;
    	print_balls(balls_left);
    	serve(ball);
    }
    
    return;
}

/*
 *	set_up()
 *	Purpose: Prepare terminal for the game
 */
void set_up()
{
	//Set up terminal
	initscr();							// turn on curses
	noecho();							// turn off echo
	cbreak();							// turn off buffering
	srand(getpid());					//seed rand() generator
	
	//Initialize structs
	clock_init();						//start the clock
	print_court();						//print court
	paddle = new_paddle();				//create a paddle
	ball = new_ball();					//create a ball

	//Signal handlers
	signal(SIGINT, SIG_IGN);			// ignore SIGINT
	signal( SIGALRM, alarm_handler );	// setup ALRM handler
	set_ticker( 1000 / TICKS_PER_SEC );	// send ALRM per tick
}


/* SIGARLM handler: decr directional counters, move when they hit 0	*/
/* note: may have too much going on in this handler			*/
void alarm_handler(int s)
{
	signal( SIGALRM , SIG_IGN );		/* dont get caught now 	*/

	clock_tick();
	ball_move(ball);
	
	if(bounce_or_lose(ball, paddle) == LOSE && balls_left > 0)
    {
    	balls_left--;
		print_balls(balls_left);
		serve(ball);
    }

	signal(SIGALRM, alarm_handler);		/* re-enable handler	*/
}


/* stop ticker and curses */
void wrap_up(int status)
{
	free(paddle);
	free(ball);
    set_ticker(0);
    endwin();
    
    if(status != 0)
    	exit(status);
}



