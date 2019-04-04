/*
 * ==========================
 *   FILE: ./clock.c
 * ==========================
 * Purpose: 
 *
 * Interface:
 *		new_paddle()		-- allocates memory and inits a new paddle
 *		paddle_up()			-- determines if room to move up, and does so
 *		paddle_down()		-- determines if room to move down, and does so
 *		paddle_contact()	-- determines if ball is touching paddle
 *
 * Internal functions:
 *		paddle_init()		-- initializes paddle's vars, and draws on screen
 *		draw_paddle()		-- draws full paddle from top-to-bottom
 *
 * Notes:
 *
 */

//INCLUDES
#include <curses.h>
#include "pong.h"
#include "court.h"
#include "clock.h"

//CONSTANTS
#define MINUTE 60


//STRUCT
struct timer {
    int mins, secs, ticks;
};

static struct timer clock;


/*
 * ===========================================================================
 * INTERNAL FUNCTIONS
 * ===========================================================================
 */

/*
 * ===========================================================================
 * EXTERNAL INTERFACE
 * ===========================================================================
 */

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
        if(++clock.secs == MINUTE)
        {
            clock.secs = 0;
            clock.mins++;
        }
        
        clock.ticks = 0;
        print_time();
    }
    
    return;
}