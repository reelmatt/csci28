/*
 * ==========================
 *   FILE: ./clock.h
 * ==========================
 * Purpose: Header file for clock.c
 */

/* CONSTANTS */
#define	TICKS_PER_SEC	50		// affects speed

/* EXTERNAL INTERFACE */
void clock_init();
void clock_tick();
int get_mins();
int get_secs();