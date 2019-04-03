/*
 * ==========================
 *   FILE: ./clock.h
 * ==========================
 */

#define	TICKS_PER_SEC	50		/* affects speed	*/

void clock_init();
void print_time();
void clock_tick();

int get_mins();
int get_secs();