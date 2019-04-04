/*
 * ==========================
 *   FILE: ./ball.h
 * ==========================
 * Purpose: Header file for ball.c
 */


//CONSTANTS
#define LOSE -1
#define NO_CONTACT 0
#define CONTACT 1
#define BOUNCE 1

//Opaque structs
struct ppball;
struct pppaddle;

//Public functions
struct ppball * new_ball();
void ball_move(struct ppball *);
int bounce_or_lose(struct ppball *, struct pppaddle *);
int get_balls_left(struct ppball *);
void serve(struct ppball *);