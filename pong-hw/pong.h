/*
 * ==========================
 *   FILE: ./pong.h
 * ==========================
 * Purpose: Header file for pong.c
 */

/* GAME CONSTANTS */
#define NUM_BALLS 3
#define	BLANK ' '

/* EXTERNAL INTERFACE - HELPER FUNCTIONS */
void alarm_handler(int);
void wrap_up();
void park_cursor();