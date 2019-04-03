/*
 * ==========================
 *   FILE: ./paddle.h
 * ==========================
 * Purpose: Header file for paddle.c
 */

//Forward defintion
struct pppaddle;

//Create a new paddle
struct pppaddle * new_paddle();

//Movement
void paddle_up(struct pppaddle *);
void paddle_down(struct pppaddle *);

//Detect if object is touching the paddle
int paddle_contact(int, int, struct pppaddle *);