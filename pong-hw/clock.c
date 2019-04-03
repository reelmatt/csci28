/*
 * ==========================
 *   FILE: ./clock.c
 * ==========================
 */

//INCLUDES
#include <curses.h>
#include "pong.h"
#include "court.h"
#include "clock.h"

//CONSTANTS
#define SECOND 60
#define TIME_LEN 17 //length of outputted time string
#define TIME_FORMAT "TOTAL TIME: %.2d:%.2d"

//STRUCT
struct timer {
    int mins, secs, ticks;
};

static struct timer clock;

/*
 *	clock_init()
 *	Purpose: Initialize clock struct to zeroes
 */
void clock_init()
{
    clock.mins = 0;
    clock.secs = 0;
    clock.ticks = 0;
    
    return;
}

/*
 *	print_time()
 *	Purpose: Print elapsed time, right-adjusted above top border
 */
void print_time()
{
    move(HEADER, (COLS - BORDER - 1 - TIME_LEN));
    printw(TIME_FORMAT, clock.mins, clock.secs);
	park_cursor();
    refresh();
    return;
}

/*
 *	get_mins()
 *	Purpose: Public function to access 'mins' value in timer struct
 *	 Return: The current value of 'clock.mins'
 */
int get_mins()
{
	return clock.mins;
}

/*
 *	get_secs()
 *	Purpose: Public function to access 'secs' value in timer struct
 *	 Return: The current value of 'clock.secs'
 */
int get_secs()
{
	return clock.secs;
}

/*
 *	clock_tick()
 *	Purpose: Update timer struct every second
 *	 Method: Once the number of ticks equals TICKS_PER_SEC, increment
 *			 the number of seconds. When it reaches 60 seconds, reset
 *			 to 0 and increment the number of minutes.
 */
void clock_tick()
{
	//enough ticks for a second
    if(++clock.ticks == TICKS_PER_SEC)
    {
    	//enough seconds for a min
        if(++clock.secs == 60)
        {
            clock.secs = 0;
            clock.mins++;
        }
        
        clock.ticks = 0;
        print_time();
    }
    
    return;
}