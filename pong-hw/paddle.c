#include <curses.h>
#include <signal.h>
#include "paddle.h"
#include "bounce.h"

#define	DFL_SYMBOL	'#'
#define PAD_SIZE 5

struct pppaddle {
    int pad_top, pad_bot, pad_col;
    char pad_char;
    int pad_mintop, pad_maxbot;
};

static struct pppaddle paddle;
static void draw_paddle();


void draw_paddle()
{
    int i;

    for(i = paddle.pad_top; i < paddle.pad_bot; i++)
        mvaddch(i, paddle.pad_col, paddle.pad_char);

    move(0, 30);
    printw("pad_top = %.2d, pad_bot = %.2d", paddle.pad_top, paddle.pad_bot);

    refresh();
    return; 
}

void paddle_init()
{
    paddle.pad_char = DFL_SYMBOL;
    paddle.pad_mintop = 3;
    paddle.pad_maxbot = LINES - 3;
    paddle.pad_col = COLS - 3;
    paddle.pad_top = (LINES / 2) - (PAD_SIZE / 2);
    paddle.pad_bot = paddle.pad_top + PAD_SIZE;
    
    draw_paddle();
}

void paddle_up()
{
    if( (paddle.pad_top - 1) > paddle.pad_mintop)
    {
        mvaddch(paddle.pad_bot - 1, paddle.pad_col, BLANK);
        --paddle.pad_top;
        --paddle.pad_bot;
        draw_paddle();
    }
}

void paddle_down()
{
    if( (paddle.pad_bot + 1) < paddle.pad_maxbot)
    {
        mvaddch(paddle.pad_top, paddle.pad_col, BLANK);
        ++paddle.pad_top;
        ++paddle.pad_bot;
        draw_paddle();
    }
}

int paddle_contact(int y, int x)
{
    //in the right-most col
    if(y >= paddle.pad_top && y <= paddle.pad_bot)
    {

        return 1;
//         if(y > paddle.pad_mintop && y < paddle.pad_maxbot)
//         {
//             return 1;
//         }
        
        
    }
    
    return 0;
}