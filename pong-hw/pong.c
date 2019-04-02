/*
 * ==========================
 *   FILE: ./pong.c
 * ==========================
 * Purpose: Play a one-player pong game.
 *
 * Outline: sttyl with no arguments will print the current values for options
 *			it knows about. Special characters you can change are erase and
 *			kill. Other attributes can be set (turned on) using the name, or
 *			unset (turned off) by adding a '-' before the attribute. See usage
 *			below for examples.
 *
 * Usage:	./sttyl							-- no options, prints current vals
 *			./sttyl -echo onlcr erase ^X	-- turns off echo, turns on onlcr
 *											   and sets the erase char to ^X
 *
 * Tables: sttyl is a table-driven program. The tables are defined below.
 *		There is a single table that contains structs for each of the four
 *		flag types in termios: c_iflag, c_oflag, c_cflag, and c_lflag. There
 *		is a separate table that contains structs storing the special
 *		characters. The table of flags contains an offset corresponding to
 *		the position in a termios struct where the bit-mask for the flag can
 *		be found. To read how this is implemented and works, read the Plan
 *		document.
 */

/* INCLUDES */
#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "alarmlib.h"
#include "ball.h"
#include "clock.h"
#include "court.h"
#include "paddle.h"
#include "pong.h"

/* CONSTANTS */
#define DEBUG


/* LOCAL FUNCTIONS */
static void set_up();
static void up_paddle();
static void down_paddle();
int balls_left = NUM_BALLS;

static struct pppaddle * paddle;	//the paddle
static struct ppball * ball;		//the ball

/*
 *	main()
 *	Purpose: Set the stage to play pong.
 *	 Method: Start by initializing all necessary variables and structs. Then,
 *			 animate ball movement and block on keyboard input to move the
 *			 paddle.
 *	 Return: 0 on success, exit non-zero on error. When fatal error occurs,
 *			 function will call wrap_up() which will reset the terminal
 *			 settings and call exit().
 */
int main ()
{
    int c;
    set_up();
    serve(ball);
    
    while( balls_left >= 0 && (c = getch()) != 'Q')
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
	set_ticker( 1000 / TICKS_PER_SEC );	// send an ALRM per tick
}

/*
 *	up_paddle()
 *	Purpose: 
 */
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

/*
 *	down_paddle()
 *	Purpose: 
 */
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
		sleep(2);
		serve(ball);
    }

	signal(SIGALRM, alarm_handler);		/* re-enable handler	*/
}


/* stop ticker and curses */
void wrap_up(int status)
{
	if(paddle)
		free(paddle);

	if(ball)
		free(ball);

    set_ticker(0);
    endwin();
    
    if(status != 0)
    	exit(status);
}



