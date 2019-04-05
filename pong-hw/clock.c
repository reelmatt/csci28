/*
 * ===========================================================================
 *   FILE: ./clock.c
 * ===========================================================================
 * Purpose: Create a static clock in the file, that is accessed via other
 *			files to print/update time.
 *
 * Interface:
 *		clock_init()	-- Initialize clock struct to zeroes
 *		clock_tick()	-- Update timer struct every second
 *		get_mins()		-- Access the 'mins' value in the clock
 *		get_secs()		-- Access the 'secs' value in the clock
 *
 * Notes:
 *		The clock is file-scoped to clock.c. It is updated via a call to
 *		clock_tick() every time a SIGALRM fires. The other functions are
 *		used to print a running clock, and an exit message with the final
 *		play time.
 */

/* INCLUDES */
#include "clock.h"
#include "court.h"

/* CONSTANTS */
#define MINUTE 60

/* CLOCK STRUCT */
struct timer {
    int mins, secs, ticks;
};

static struct timer clock;

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
 *	clock_tick()
 *	Purpose: Update timer struct every second
 *	 Method: Once the number of ticks equals TICKS_PER_SEC, increment
 *			 the number of seconds. When it reaches 60 seconds, reset
 *			 to 0 and increment the number of minutes.
 */
void clock_tick()
{
	// enough ticks for a second
    if(++clock.ticks == TICKS_PER_SEC)
    {
    	// enough seconds for a min
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