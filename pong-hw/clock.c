#include <curses.h>

#define BORDER 3
#define SECOND 60

struct timer {
    int mins, secs, ticks;
};

static struct timer clock;

// void clock_init();
// void show_time();
// void clock_tick();


void clock_init()
{
    clock.mins = 0;
    clock.secs = 0;
    clock.ticks = 0;
    
    return;
}

void show_time()
{
    move(BORDER - 1, BORDER);
    printw("\t\t\t%s: %.2d:%.2d", "TOTAL TIME", clock.mins, clock.secs);
    refresh();
    return;
}

void clock_tick()
{
    if(++clock.ticks == SECOND)
    {
        if(++clock.secs == 60)
        {
            clock.secs = 0;
            clock.mins++;
        }
        
        clock.ticks = 0;
        
        show_time();
    }
    
    
    return;
}