#define LOSE -1
#define NO_CONTACT 0
#define CONTACT 1
#define BOUNCE 1

struct ppball;
struct pppaddle;

void go_slow(struct ppball *);
void go_fast(struct ppball *);

void ball_init(struct ppball *);
void serve(struct ppball *);
void ball_move(struct ppball *);
struct ppball * new_ball();

int bounce_or_lose(struct ppball *, struct pppaddle *);
// int bounce_or_lose(struct ppball *);