/*
 * ==========================
 *   FILE: ./pong.c
 * ==========================
 * Purpose: Play a one-player pong game.
 *
 * Outline: The goal of the game is to last as long as you can. You get
 *			three balls before the game ends. To move the paddle up and
 *			down, press the 'k' and 'm' keys respectively. When the ball
 *			goes past the paddle, the game briefly pauses, then resets,
 *			serving the ball from a random position, with a random direction
 *			and speed.
 *
 * Objects: pong is written with object-oriented programming in mind. The key
 *			elements of the game exist in respective .c files, controlled by
 *			public (non-static) functions exposed in .h files. For pong, the
 *			objects include the ball and paddle, of which multiple can be
 *			instantiated (with further development, this could lead to a
 *			two-player, or multi-ball pong game). A separate object also
 *			exists for the clock, which keeps track (in minutes and seconds)
 *			how long the player has been playing. To keep the code modular,
 *			functions that exist to draw the court are also separated out
 *			into its own file.
 *
 *    Note: Some of the code (like the main loop) was copied and/or heavily
 *			inspired by code found in the assignment handout, or sample
 *			course code, such as that found in 'bounce2d.c'. See each
 *			function for any further comments.
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
#define MIN_LINES 10		//minimum terminal row size
#define MIN_COLS 40			//minimum terminal column size
#define EXIT_MSG_LEN 16		//to help center exit message

/* LOCAL FUNCTIONS */
static void set_up();
static void up_paddle();
static void down_paddle();
static void check_screen();
static void check_state();
static void exit_message();
//static void new_round();

/* LOCAL VARIABLES -- OBJECT INSTANCES */
static struct pppaddle * paddle;
static struct ppball * ball;
// static struct ppcourt * court;

/*
 *	main()
 *	Purpose: Set the stage to play pong.
 *	 Method: Start by initializing all necessary variables and structs. Then,
 *			 animate ball movement and block on keyboard input to move the
 *			 paddle.
 *	 Return: 0 on success, exit non-zero on error. When fatal error occurs,
 *			 function will call wrap_up() which will reset the terminal
 *			 settings and call exit().
 *	   Note: The structure is mostly copied from the assignment spec, with
 *			 modifications to fit the object-oriented design of the program.
 */
int main ()
{
    int c;
    set_up();
    serve(ball);
    
    while( get_balls(ball) >= 0 && (c = getch()) != 'Q')	//block on keyboard
    {
        if(c == 'k')
            up_paddle();
        else if (c == 'm')
            down_paddle();	
    }
    
    exit_message();
    wrap_up();
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
	check_screen();						// check screen size
	noecho();							// turn off echo
	cbreak();							// turn off buffering
	srand(getpid());					// seed rand() generator

	//Initialize objects
	court_init(BORDER, LINES - BORDER - 1, BORDER, COLS - BORDER - 1);
//	court = new_court();				// create a court
	paddle = new_paddle();				// create a paddle
	ball = new_ball();					// create a ball
	clock_init();						// create the clock
	print_court(NULL, ball);			// print court

	//Signal handlers
	signal(SIGINT, SIG_IGN);			// ignore SIGINT
	signal(SIGALRM, alarm_handler);		// setup ALRM handler
	set_ticker(1000 / TICKS_PER_SEC);	// send an ALRM per tick
}

/*
 *	up_paddle()
 *	Purpose: Move the paddle up and check if it made contact with ball
 */
void up_paddle()
{
    paddle_up(paddle);
    check_state();
    return;
}

/*
 *	down_paddle()
 *	Purpose: Move the paddle down and check if it made contact with ball
 */
void down_paddle()
{
    paddle_down(paddle);
    check_state();
    return;
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

/*
 *	check_state()
 *	Purpose: After ball or paddle movement, see if it is LOSE. If yes,
 *			 start a new round.
 */
void check_state()
{
	if( bounce_or_lose(ball, paddle, NULL) == LOSE)
	{
		if(get_balls(ball) > 0)			//there are still more balls left
		{
			sleep(2);					//wait a bit
			serve(ball);				//and start again
		}
		else							//we're done
		{
			set_ticker(0);				//@@TEMPORARY -- TRY AND DELETE (DUPLICATE)
			exit_message();				//print final time
			wrap_up();					//clean up...
			exit(0);					//...and quit
		}
	}
	
	return;
}

/*
 *	exit_message()
 *	Purpose: Display the final 'score'
 */
void exit_message()
{
	int y = (LINES / 2);
	int x = (COLS / 2) - (EXIT_MSG_LEN / 2);
	standout();
	mvprintw(y, x, "You lasted %.2d:%.2d", get_mins(), get_secs());
	standend();
	refresh();
	sleep(2);
}

/* SIGARLM handler: decr directional counters, move when they hit 0	*/
/* note: may have too much going on in this handler			*/
void alarm_handler(int s)
{
	signal( SIGALRM , SIG_IGN );		/* dont get caught now 	*/

	clock_tick();
	ball_move(ball);
	check_state();	
  
	signal(SIGALRM, alarm_handler);		/* re-enable handler	*/
}

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

// 	if(get_mins() != 0 || get_secs() != 0)	//check the clock started
// 		exit_message();						//print time

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


/*
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
}*/