#include <curses.h>

#define BORDER 3

void print_row();
void print_col();
void update_header();

/*
 *  print_row()
 *  Purpose: print a row
 *    Input: start, the window row to print at
 */
void print_row(int start)
{
    int i;
    
    move(start, BORDER);
    for(i = BORDER; i < COLS - BORDER - 1; i++)
        addch('-');
    
    return;
}

void print_col()
{
    int i;
    for(i = BORDER + 1; i < LINES - BORDER - 1; i++)
        mvaddch(i, BORDER, '|');
    
    return; 
}

void print_court()
{
    update_header();
    print_row(BORDER);                  //top row
    print_col();                        //left edge
    print_row((LINES - BORDER - 1));    //bottom row
    
    return;
}

void update_header()
{
    move(BORDER - 1, BORDER);
    //printw("%s: %2d\t%s:  %2d:%2d", "BALLS LEFT", 3, "TOTAL TIME", 2, 19);
    printw("%s: %2d", "BALLS LEFT", 3);
}
