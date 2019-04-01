#include <curses.h>
#include <signal.h>
#include "bounce.h"
#include <stdlib.h>
#include <unistd.h>
#include "clock.h"
#include "paddle.h"

#define BORDER 3
#define	DFL_SYMBOL	'O'
#define	X_INIT		10		/* starting col		*/
#define	Y_INIT		10		/* starting row		*/
#define MAX_DELAY   10


#define OUT -1
#define NO_CONTACT 0
#define BOUNCE 1

static int start_dir();
static int delay();

/**
 **	the only object in town
 **/
struct ppball {
    int	x_pos, x_dir,
        y_pos, y_dir,
        y_delay, y_count,
        x_delay, x_count;
    char symbol;
} ;


struct ppball ball;
static int bounce_or_lose(struct ppball *);
void print_ball_loc();

void go_fast()
{
    /*if(c == 'f')
        ball.x_delay--;
    else if (c == 's')
        ball.x_delay++;
    else if (c == 'F')
        ball.y_delay--;
    else if (c == 'S')
        ball.y_delay++;*/

    ball.x_delay--;
    ball.y_delay--;
    move(1, 1);
    printw("Y_DELAY = %d and X_DELAY = %d", ball.y_delay, ball.x_delay);
}

void go_slow()
{
    ball.x_delay++;
    ball.y_delay++;
    move(1, 1);
    printw("Y_DELAY = %d and X_DELAY = %d", ball.y_delay, ball.x_delay);
}

int start_dir()
{
    if( (rand() % 2) == 0)
        return -1;
    else
        return 1;
}

int delay()
{
    return 1 + (rand() % (MAX_DELAY - 1));
}

void print_ball_loc()
{
    move(0, 1);
    printw("Y_pos = %2d and X_pos = %2d", ball.y_pos, ball.x_pos);
    refresh();
}

void init_ball()
{
    ball.y_pos = Y_INIT;
	ball.x_pos = X_INIT;
	ball.y_count = ball.y_delay = delay() ;
	ball.x_count = ball.x_delay = delay() ;
	ball.y_dir = start_dir();
	ball.x_dir = start_dir();
	ball.symbol = DFL_SYMBOL ;
	
    
    move(1, 1);
    printw("Y_DELAY = %d and X_DELAY = %d", ball.y_delay, ball.x_delay);
}

void serve()
{
    init_ball();
    move(1, 1);
    printw("Y_DELAY = %d and X_DELAY = %d", ball.y_delay, ball.x_delay);
    mvaddch(ball.y_pos, ball.x_pos, ball.symbol);
    
    refresh();
}

/* SIGARLM handler: decr directional counters, move when they hit 0	*/
/* note: may have too much going on in this handler			*/

void ball_move(int s)
{
	int	y_cur, x_cur, moved;

	signal( SIGALRM , SIG_IGN );		/* dont get caught now 	*/
	
	clock_tick();
	
	y_cur = ball.y_pos ;		/* old spot		*/
	x_cur = ball.x_pos ;
	moved = 0 ;

	if ( ball.y_delay > 0 && --ball.y_count == 0 ){
		ball.y_pos += ball.y_dir ;	/* move	*/
		ball.y_count = ball.y_delay  ;	/* reset*/
		moved = 1;
	}

	if ( ball.x_delay > 0 && --ball.x_count == 0 ){
		ball.x_pos += ball.x_dir ;	/* move	*/
		ball.x_count = ball.x_delay  ;	/* reset*/
		moved = 1;
	}


	if ( moved ){
		mvaddch(y_cur, x_cur, BLANK);
		mvaddch(ball.y_pos, ball.x_pos, ball.symbol);
		bounce_or_lose( &ball );
		move(LINES-1, COLS-1);		/* park cursor	*/
		refresh();
	}
	signal(SIGALRM, ball_move);		/* re-enable handler	*/
}

/* bounce_or_lose: if ball hits walls, change its direction
 *   args: address to ppball
 *   rets: 1 if a bounce happened, 0 if not, -1 if out of court on right
 */
int bounce_or_lose(struct ppball *bp)
{
	int	return_val = 0 ;
    print_ball_loc();
	if ( bp->y_pos == (BORDER + 1) )                //top
	    bp->y_dir = 1 , return_val = BOUNCE ;
	else if ( bp->y_pos == (LINES - BORDER - 2) )   //bottom
		bp->y_dir = -1 , return_val = BOUNCE;

    if ( bp->x_pos == (BORDER + 1) )                    //left
    {
		bp->x_dir = 1;
		return_val = BOUNCE;
    }
	else if ( bp->x_pos == (COLS - BORDER - 1) )        //right
	{
	    if( paddle_contact(bp->y_pos, bp->x_pos) == 1 )
	    {
            bp->x_dir = -1;
            return_val = BOUNCE;	    
	    }
	    else
	    {
            ball.y_delay = 0;
            ball.x_delay = 0;
	        return_val = OUT;
	    }
	}

    move(0, 1);
    printw("Y_pos = %.3d and X_pos = %.3d", ball.y_pos, ball.x_pos);
//     move(0, 1);
//     printw("Y_dir = %2d and X_dir = %2d", ball.y_dir, ball.x_dir);

	return return_val;
}