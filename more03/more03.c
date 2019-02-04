/*  more03.c  - version 0.3 of more
 */
#include <stdio.h>
#include <stdlib.h>
#include "termfuncs.h"
#define  PAGELEN	24
#define  ERROR		1
#define  SUCCESS	0
#define  has_more_data(x)   (!feof(x))
#define	CTL_DEV	"/dev/tty"		/* source of control commands	*/

int  do_more(FILE *);
int  how_much_more(FILE *, int [2]);
int print_one_line(FILE *, int);

int main( int ac , char *av[] )
{
	FILE	*fp;			/* stream to view with more	*/
	int	result = SUCCESS;	/* return status from main	*/

	if ( ac == 1 )
		result = do_more( stdin );
	else
		while ( result == SUCCESS && --ac )
			if ( (fp = fopen( *++av , "r" )) != NULL ) {
				result = do_more( fp ) ; 
				fclose( fp );
			}
			else
				result = ERROR;
	return result;
}

/*  do_more -- show a page of text, then call how_much_more() for instructions
 *      args: FILE * opened to text to display
 *      rets: SUCCESS if ok, ERROR if not
 */
int do_more( FILE *fp )
{
	int rows_cols[2] = { PAGELEN, 80 };		/* set defaults */
	
	if ( get_term_size(rows_cols) != 0 )
		return ERROR;

	int	space_left = rows_cols[0] ;			/* space left on screen */
	FILE* fp_tty = fopen( CTL_DEV, "r" );	/* connect to stream to keyboard	*/

	while ( has_more_data( fp ) ) 		/* more input	*/
	{
		if ( space_left <= 1 ) 			/* screen full?	*/
		{				
			int reply = how_much_more(fp_tty, rows_cols);	/* ask user	*/
			if ( reply == 0 )		/*    n: done   */
				break;
			space_left = reply;		/* reset count	*/
			printf("\033[1K\033[7D");	/* clear line, set cursor at start of line */
		}
		
		space_left -= print_one_line(fp, rows_cols[1]);	/* print line and count it */	
	}
	
	fclose( fp_tty );			/* disconnect keyboard	*/
	return SUCCESS;				/* EOF => done		*/
}

/*  print_one_line(fp, cols) -- copy data from input to stdout until \n or EOF
 *       args: fp -- file with data
 *             cols -- number of columns in the terminal
 *     return: the number of, adjusted, rows that were printed to the screen
 */
int print_one_line( FILE *fp, int cols )
{
	int	c;
	int i = 0;
	int rows = 1;

	while( ( c = getc(fp) ) != EOF && c != '\n' )
	{
		putchar( c ) ;
		i++;             /* increment for each char placed */
		
        /* if chars exceed cols, reset and add to row counter */
		if(i > cols)
		{
			rows++;
			i = 0;
		}
	}
	
	putchar('\n');
	
	return rows;
}

/*  how_much_more -- ask user how much more to show
 *      args: none
 *      rets: number of additional lines to show: 0 => all done
 *	note: space => screenful, 'q' => quit, '\n' => one line
 */
int how_much_more(FILE *fp, int rows_cols[2])
{
	int	c;

	printf("\033[7m more? \033[m");		/* reverse on a vt100	*/	
	while( (c = rawgetc(fp)) != EOF )	/* get user input	*/
	{
		if ( get_term_size(rows_cols) != 0 )
			fprintf(stderr, "Problem getting term size.\n");
	
		if ( c == 'q' )			/* q -> N		*/
			return 0;
		if ( c == ' ' )			/* ' ' => next page	*/
			return rows_cols[0];		/* how many to show	*/
		if ( c == '\n' )		/* Enter key => 1 line	*/
			return 1;		
	}
	return 0;
}
