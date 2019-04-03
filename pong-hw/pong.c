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
#define MIN_LINES 10
#define MIN_COLS 40

/* LOCAL FUNCTIONS */
static void set_up();
static void up_paddle();
static void down_paddle();
static void check_screen();
int balls_left = NUM_BALLS;
void new_round();

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
    
    while( get_balls(ball) >= 0 && (c = getch()) != 'Q')
    {
        if(c == 'k')
            up_paddle();
        else if (c == 'm')
            down_paddle();	
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
	srand(getpid());					// seed rand() generator
	check_screen();						// check screen size
	
	//Initialize structs
	clock_init();						// start the clock
	paddle = new_paddle();				// create a paddle
	ball = new_ball();					// create a ball
	print_court(ball);					// print court

	//Signal handlers
	signal(SIGINT, SIG_IGN);			// ignore SIGINT
	signal( SIGALRM, alarm_handler );	// setup ALRM handler
	set_ticker( 1000 / TICKS_PER_SEC );	// send an ALRM per tick
}

/*
 *	check_screen()
 *	Purpose: Check the terminal is at least MIN_COLS x MIN_LINES big
 *	  Error: If the window does not meet one or both of these dimensions,
 *			 the terminal is reset to normal, and an error message is output
 *			 to inform the user to resize and try again.
 */
void check_screen()
{
	if(LINES < MIN_LINES || COLS < MIN_COLS)
	{
		wrap_up();
		fprintf(stderr, "Terminal must be a minimum of %dx%d. "
						"Please resize and try again.\n",
						MIN_COLS, MIN_LINES);
		exit(1);
	}
}

void new_round()
{
	if(get_balls(ball) > 0)			//there are still more balls left
	{
		sleep(2);					//wait a bit
		serve(ball);				//and start again
	}
	else							//we're done, clean up and quit
	{
		wrap_up();
		exit(0);
	}
}

/*
 *	up_paddle()
 *	Purpose: Move the paddle up and check if it made contact with ball
 */
void up_paddle()
{
    paddle_up(paddle);
    
    if( bounce_or_lose(ball, paddle) == LOSE)
		new_round();
    
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
		new_round();
    
    return;
}




/* SIGARLM handler: decr directional counters, move when they hit 0	*/
/* note: may have too much going on in this handler			*/
void alarm_handler(int s)
{
	signal( SIGALRM , SIG_IGN );		/* dont get caught now 	*/

	clock_tick();
	ball_move(ball);
	
	if(bounce_or_lose(ball, paddle) == LOSE)
		new_round();
    
	signal(SIGALRM, alarm_handler);		/* re-enable handler	*/
}

/*
 *	new_round()
 *	Purpose: 
 */
// void new_round()
// {
// 	print_balls(get_balls(ball));
// 	serve(ball);
// }

/*
 *	wrap_up()
 *	Purpose: free memory, stop ticker, close curses
 */
void wrap_up()
{
	if(paddle)								//if paddle was malloc'ed
		free(paddle);						//free it

	if(ball)								//if ball was malloc'ed
		free(ball);							//free it

	if(get_mins() != 0 || get_secs() != 0)	//check the clock started
		exit_message();						//print time

    set_ticker(0);							//stop ticker
    endwin();								//close curses
    
    return;
}

/*
 *	park_cursor()
 *	Purpose: Helper function to park the cursor in lower-right of screen
 */
void park_cursor()
{
	move(LINES - 1, COLS - 1);
	return;
}

