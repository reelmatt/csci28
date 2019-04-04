/*
 * ==========================================================================
 *   FILE: ./pong.c
 * ==========================================================================
 * Purpose: Core logic to play a one-player pong game.
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
 *		    inspired by code found in the assignment handout, or sample
 *		    course code, such as that found in 'bounce2d.c'. See each
 *		    function for any further comments.
 *
 * Interface:
 *		alarm_handler()	-- signal handler; used for ball movement
 *		wrap_up()		-- closes curses and ready to return to terminal
 *		park_cursor()	-- helper function to park cursor in bottom-right
 *
 * Internal functions:
 *		main()			-- loop of the game, blocking on keyboard input
 *		set_up()		-- prepare the terminal to play, init structs and vars
 *		up_paddle()		-- wrapper to paddle function and check game state
 *		down_paddle()	-- wrapper to paddle function and check game state
 *		is_min_size()	-- ensure the terminal is large enough to play
 *		is_next_round()	-- see if there is another round to play, and do so
 *		exit_message()	-- print message about how player did when exiting 
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
#define MIN_LINES 11		//minimum terminal row size
#define MIN_COLS 40			//minimum terminal column size
#define EXIT_MSG_LEN 16		//to help center exit message

/* LOCAL VARIABLES -- OBJECT INSTANCES */
static struct pppaddle * paddle;
static struct ppball * ball;

/*
 * ===========================================================================
 * INTERNAL FUNCTIONS
 * ===========================================================================
 */
static void set_up();
static void up_paddle();
static void down_paddle();
static void is_min_size();
static void is_next_round();
static void exit_message();
static void resize_handler(int);
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
    
    while( get_balls_left(ball) >= 0 && (c = getch()) != 'Q')
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
	// Set up terminal
	initscr();							// turn on curses
	is_min_size();						// check screen size
	noecho();							// turn off echo
	cbreak();							// turn off buffering
	srand(getpid());					// seed rand() generator

	// Court dimensions
	int top = BORDER;
	int right = COLS - BORDER - 1;		// -1 because 0-indexed
	int bot = LINES - BORDER - 1;		// -1 because 0-indexed
	int left = BORDER;
	
	// Initialize objects
	court_init(top, right, bot, left);	// init a court
	paddle = new_paddle();				// create a paddle
	ball = new_ball();					// create a ball
	clock_init();						// init the clock
	print_court(NUM_BALLS);				// print court

	// Signal handling
	signal(SIGINT, SIG_IGN);			// ignore SIGINT
	signal(SIGWINCH, resize_handler);	// check if still min dimensions
	signal(SIGALRM, alarm_handler);		// setup ALRM handler
	set_ticker(1000 / TICKS_PER_SEC);	// send an ALRM per tick
	
	return;
}

/*
 *	up_paddle()
 *	Purpose: Move the paddle up and check if it made contact with ball
 */
void up_paddle()
{
    paddle_up(paddle);
    is_next_round();
    return;
}

/*
 *	down_paddle()
 *	Purpose: Move the paddle down and check if it made contact with ball
 */
void down_paddle()
{
    paddle_down(paddle);
    is_next_round();
    return;
}

/*
 *	is_min_size()
 *	Purpose: Check the terminal is at least MIN_COLS x MIN_LINES big
 *	  Error: If the window does not meet one or both of these dimensions,
 *			 the terminal is reset to normal, and an error message is output
 *			 to inform the user to resize and try again.
 */
void is_min_size()
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
 *	is_next_round()
 *	Purpose: After ball or paddle movement, see if it is LOSE. If yes,
 *			 start a new round.
 */
void is_next_round()
{
	if( bounce_or_lose(ball, paddle) == LOSE)
	{
		if(get_balls_left(ball) > 0)	//more balls left			
		{
			serve(ball);				// start again
		}
		else							// no more balls
		{
			exit_message();				// print final time
			wrap_up();					// clean up...
			exit(0);					// ...and quit
		}
	}
	
	return;
}

/*
 *	resize_handler()
 *	Purpose: On window size change, exit the program and output error message.
 */
void resize_handler(int s)
{
	wrap_up();
	fprintf(stderr, "Please don't resize once the game has started.\n");
	exit(3);
	
	return;
}

/*
 *	exit_message()
 *	Purpose: Display the final 'score'
 */
void exit_message()
{
	//Get coordinates to center message
	int y = (LINES / 2);
	int x = (COLS / 2) - (EXIT_MSG_LEN / 2);
	
	//Print time in reverse-text
	standout();
	mvprintw(y, x, "You lasted %.2d:%.2d", get_mins(), get_secs());
	standend();
	refresh();
	
	//Keep it on screen for 2 seconds
	millisleep(2000);
	
	return;
}

/*
 * ===========================================================================
 * EXTERNAL INTERFACE
 * ===========================================================================
 */

/*
 *	alarm_handler()
 *	Purpose: Handle SIGALRMs
 *	  Input: s, the type of signal received
 *	 Method: Ignore any SIGALRMs received while in the handler. Then,
 *			 update the clock, move the ball, and call is_next_round()
 *			 which will check bounce_or_lose().
 *	   Note: This handler calls a few functions which do quite a bit of
 *			 processing. This may lead to a race condition with the
 *			 paddle movement, and is not handled in this version. For
 *			 more, see the Plan document which discusses a potential
 *			 solution to this problem.
 */
void alarm_handler(int s)
{
	signal( SIGALRM , SIG_IGN );		// don't get caught

	clock_tick();						// update clock
	ball_move(ball);					// move ball
	is_next_round();					// check bounce_or_lose()
  
	signal(SIGALRM, alarm_handler);		// re-enable handler
	
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
 *	wrap_up()
 *	Purpose: free memory, stop ticker, close curses
 */
void wrap_up()
{
	if(paddle)								//if paddle was malloc'ed
		free(paddle);						//free it

	if(ball)								//if ball was malloc'ed
		free(ball);							//free it

    set_ticker(0);							//stop ticker
    endwin();								//close curses
    
    return;
}