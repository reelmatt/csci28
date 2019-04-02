struct pppaddle;

void paddle_up(struct pppaddle *);
void paddle_down(struct pppaddle *);
void paddle_init(struct pppaddle *, int, int);
int paddle_contact(int, int, struct pppaddle *);
struct pppaddle * new_paddle();