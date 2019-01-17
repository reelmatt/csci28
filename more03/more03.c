/*  more02.c  - version 0.2 of more
 *	read and print 24 lines then pause for a few special commands
 *	v02: reads user control cmds from /dev/tty
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
int  how_much_more(FILE *, int);
void print_one_line(FILE *);

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
	int rows = PAGELEN;
	int cols = 80;
	int rows_cols[2];
	if ( get_term_size(rows_cols) == 0 )
	{
		rows = rows_cols[0];
		cols = rows_cols[1];
	}
		
	
//	printf("window rows == %d\nwindow cols == %d\n", rows_cols[0], rows_cols[1]);

	int	space_left = rows ;		/* space left on screen */
	int	reply;				/* user request		*/
	FILE	*fp_tty;			/* stream to keyboard	*/

	fp_tty = fopen( CTL_DEV, "r" );		/* connect to keyboard	*/
	while ( has_more_data( fp ) ) {		/* more input	*/
		if ( space_left <= 1 ) {		/* screen full?	*/
			reply = how_much_more(fp_tty, rows);	/* ask user	*/
			if ( reply == 0 )		/*    n: done   */
				break;
			space_left = reply;		/* reset count	*/
			printf("\033[7D");		//go back 7 columns (start of line)
		}
		
		print_one_line( fp );
		space_left--;				/* count it	*/
	}
	fclose( fp_tty );			/* disconnect keyboard	*/
	return SUCCESS;				/* EOF => done		*/
}

/*  print_one_line(fp) -- copy data from input to stdout until \n or EOF */
void print_one_line( FILE *fp )
{
	int	c;

	while( ( c = getc(fp) ) != EOF && c != '\n' )
		putchar( c ) ;
	putchar('\n');
}

/*  how_much_more -- ask user how much more to show
 *      args: none
 *      rets: number of additional lines to show: 0 => all done
 *	note: space => screenful, 'q' => quit, '\n' => one line
 */
int how_much_more(FILE *fp, int rows)
{
	int	c;

	printf("\033[7m more? \033[m");		/* reverse on a vt100	*/
	//while( (c = getc(fp)) != EOF )		/* get user input	*/
	while( (c = rawgetc(fp)) != EOF )
	{
		if ( c == 'q' )			/* q -> N		*/
			return 0;
		if ( c == ' ' )			/* ' ' => next page	*/
			return rows;		/* how many to show	*/
		if ( c == '\n' )		/* Enter key => 1 line	*/
			return 1;		
	}
	return 0;
}
