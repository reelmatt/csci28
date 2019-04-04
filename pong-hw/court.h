/*
 * ==========================
 *   FILE: ./court.h
 * ==========================
 * Purpose: Header file for court.c
 */

/* Opaque structs */
struct ppball;
struct ppcourt;

/* CONSTANTS */
#define BORDER 3

void court_init(int, int, int, int);
void print_court(int);
void print_balls(int);
void print_time();
int get_top_edge();
int get_right_edge();
int get_bot_edge();
int get_left_edge();