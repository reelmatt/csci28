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

//Foward declarations for structs
struct ppball;
struct pppaddle;
struct ppcourt;

//Public functions
void serve(struct ppball *);
void ball_move(struct ppball *);
struct ppball * new_ball();
int bounce_or_lose(struct ppball *, struct pppaddle *, struct ppcourt *);
int get_balls(struct ppball *);