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

#define BORDER 3
#define NUM_BALLS 3

void unused_function();

void set_up();
void wrap_up();

void up_paddle();
void down_paddle();
// void bounce_or_lose();

int balls_left;
//struct ppball ball;

int main ()
{
    int c;
    set_up();  
    serve();
    
    while( (c = getch()) != 'Q')
    {
        if(c == 'f')
            go_fast();
        else if (c == 's')
            go_slow();
        else if(c == 'k')
            up_paddle();
        else if (c == 'm')
            down_paddle();
    }
    
    wrap_up();
    return 0;
}

void up_paddle()
{
    paddle_up();
   // bounce_or_lose();
}

void down_paddle()
{
    paddle_down();
   // bounce_or_lose();
}

void set_up()
{
	initscr();		/* turn on curses	*/
	noecho();		/* turn off echo	*/
	cbreak();		/* turn off buffering	*/

    balls_left = NUM_BALLS;
    
	signal(SIGINT, SIG_IGN);	/* ignore SIGINT	*/
	srand(getpid());
	print_court();
	paddle_init();
	clock_init();

	signal( SIGALRM, ball_move );
	set_ticker( 1000 / TICKS_PER_SEC );	/* send millisecs per tick */
}

/* stop ticker and curses */
void wrap_up()
{
    set_ticker(0);
    endwin();
}



