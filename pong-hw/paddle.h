/*
 * ==========================
 *   FILE: ./paddle.h
 * ==========================
 * Purpose: Header file for paddle.c
 */

/* OPAQUE STRUCT */
struct pppaddle;

/* EXTERNAL INTERFACE */
struct pppaddle * new_paddle();
void paddle_up(struct pppaddle *);
void paddle_down(struct pppaddle *);
int paddle_contact(int, struct pppaddle *);