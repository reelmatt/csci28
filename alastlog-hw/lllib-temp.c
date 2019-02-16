#include <stdio.h>
#include <fcntl.h>
#include <lastlog.h>
#include <unistd.h>
#include "lllib.h"

#define NRECS 512
#define LLSIZE	(sizeof(struct lastlog))
#define LL_NULL ((struct lastlog *) NULL)

static char llbuf[NRECS * LLSIZE];	//buffer storage
static int num_recs;				//num in buffer
static int cur_rec;					//next rec to read
static int buf_start;				//absolute starting index of buffer
static int buf_end;					//ending index of buffer
static int ll_fd = -1;				//file descriptor
static int num_seeks;
static int ll_reload();
void debug(int, int, int, int, int);

/*
 *	ll_open()
 *	Purpose: opens the filename given for read access.
 *	 Return: int of file descriptor on success
 *			 -1 on error
 *	   Note: copied (with minor modifications), from utmplib.c file. Provided
 *			 in assignment files, also used in lecture 02.
 */
int ll_open(char *fname)
{
	ll_fd = open(fname, O_RDONLY);
	num_recs = 0;
	cur_rec = 0;
	buf_start = 0;
	buf_end = 0;
	num_seeks = 0;
	return ll_fd;
}

void debug(int a, int b, int c, int d, int e)
{
//	printf("DEBUGGING current values\n");
	printf("rec is %d, cur_rec is %d, num_recs is %d, start is %d, end is %d\n",
		   a, b, c, d, e);
}

/*
 *	ll_seek moves the pointer for where functions will read next
 *	and returns whether it was successful in doing so. A call to
 *	ll_next() must be made to actually read the record.
 */
int ll_seek(int rec)
{
	//error was returned when ll_open was called, no file to seek
	if (ll_fd == -1)
		return -1;

	//ll_read will get the correct record, no seeking required
	if (rec == cur_rec)
	{
		printf("rec == cur_rec, no seeking. They are %d and %d\n", rec, cur_rec);
		return 0;
	}

	//if rec is outside the current buffer seek to new position
	if (rec < buf_start || rec > (buf_start + num_recs - 1))
	{
//		off_t offset = rec * LLSIZE;

		off_t offset = (rec / NRECS) * LLSIZE;	//set to multiple of buffer size
		printf("offset is %lu\n", offset);
		if ( lseek(ll_fd, offset, SEEK_SET) == -1 )
				return -1;

//		buf_start = rec;

		//integer division will round to nearest multiple of NRECS
		//i.e. rec #600 -> buf_start = (600 / 512) = 1 * 512 = 512
		buf_start = (rec / NRECS) * NRECS;

        if (ll_reload() <= 0)
        {
        	printf("reload failed\n");
            return -1;
        }
        //else
        //    buf_end = buf_start + num_recs;
	  cur_rec = rec - buf_start;

          debug(rec, cur_rec, num_recs, buf_start, (buf_start + num_recs - 1));

	}
	else
	{
		cur_rec = rec - buf_start;
		printf("in current buffer, rec is %d, cur_rec is %d\n", rec, cur_rec);
	}
	return 0;
}

/*
 * starting as a copy of ll_next()
 */
struct lastlog *ll_read()
{
	//error was returned when ll_open was called
	if (ll_fd == -1)
		return LL_NULL;
	
	//first time being called, load up buffer
	if (buf_start == 0 && num_recs == 0)
		ll_reload();
	
	//if next to read == num in buffer AND reload doesn't get any more
	//ll_reload() will ALWAYS be called, UNLESS next to read != num in buffer
	//at open though, both are equal to 0 and reload WILL run
	if(cur_rec == num_recs && ll_reload() == 0)
    {
    	return LL_NULL;
/*        int num_read = ll_reload();

        if (num_read == 0)
        {
            return LL_NULL;
        }
        else
        {
            buf_end = buf_start + num_read;
        }
*/
    }
	printf("ll_read, position is %d\n", cur_rec);
	struct lastlog *llp = (struct lastlog *) &llbuf[cur_rec * LLSIZE];
	cur_rec++;

	return llp;
}

/*
 *	ll_reload()
 *	Purpose: read in NRECS to buffer
 *	   Note: copied (with minor modifications), from utmplib.c file. Provided
 *			 in assignment files, also used in lecture 02.
 */
static int ll_reload()
{
	//where to read from is set first by ll_open, then by ll_seek
	int amt_read = read(ll_fd, llbuf, (NRECS*LLSIZE));
	
	printf("ll_reload: amt_read is %d\n", amt_read);
	if (amt_read < 0 || amt_read != (NRECS*LLSIZE))
		amt_read = -1;
	
	num_recs = amt_read/LLSIZE;
	//cur_rec = 0;
	num_seeks++;
	return num_recs;
}

/*
 *	ll_close()
 *	Purpose: close the open file
 *	   Note: copied (with minor modifications), from utmplib.c file. Provided
 *			 in assignment files, also used in lecture 02.
 */
int ll_close()
{
	int value = 0;

	printf("closing file, num_seeks is %d\n", num_seeks);
	//if there is no file open, do not close it
	if (ll_fd != -1)
		value = close(ll_fd);
		
	return value;
}

