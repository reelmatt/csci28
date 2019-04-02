#include <curses.h>
#include "court.h"
#include "clock.h"

#define SECOND 60

struct timer {
    int mins, secs, ticks;
};

static struct timer clock;

void clock_init()
{
    clock.mins = 0;
    clock.secs = 0;
    clock.ticks = 0;
    
    return;
}

void print_time()
{
    move(BORDER -1 , BORDER + 20);
    printw("%s: %.2d:%.2d", "TOTAL TIME", clock.mins, clock.secs);
    move(LINES-1, COLS-1);						//park cursor
    refresh();
    return;
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
    if(++clock.ticks == TICKS_PER_SEC)
    {
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