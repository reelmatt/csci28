/*
 *  court.h header file
 */

#define BORDER 3
#define HEADER (BORDER - 1)

struct ppball;

void print_court(struct ppball *);
void print_balls(int);
void exit_message();