/*
 * ==========================
 *   FILE: ./court.h
 * ==========================
 * Purpose: Header file for court.c
 */
#define BORDER 3
#define HEADER (BORDER - 1)

struct ppball;
struct ppcourt;

struct ppcourt * new_court();
void print_court(struct ppcourt *, struct ppball *);
void print_balls(int);
int get_right();
int get_left();
int get_top();
int get_bot();
void court_init(int, int, int, int);