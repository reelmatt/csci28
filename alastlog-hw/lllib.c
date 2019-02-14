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
static int buf_start;				//starting index of buffer
static int buf_end;					//ending index of buffer
static int ll_fd = -1;				//file descriptor

static int ll_reload();
void debug(int, int, int, int, int);

/*
 * ll_open:
 *		opens the filename given for read access.
 * Returns:
 *		int of file descriptor on success
 *		-1 on error
 */
int ll_open(char *fname)
{
	ll_fd = open(fname, O_RDONLY);
	num_recs = 0;
	cur_rec = 0;
	buf_start = 0;
	buf_end = 0;
		
	return ll_fd;
}

void debug(int a, int b, int c, int d, int e)
{
	printf("\nrec is %d, cur_rec is %d, num_recs is %d, start is %d, end is %d\n",
		   a, b, c, d, e);
}

/*
 *	ll_seek moves the pointer for where functions will read next
 *	and returns whether it was successful in doing so. A call to
 *	ll_next() must be made to actually read the record.
 */
int ll_seek(int rec)
{
	//debug(rec, cur_rec, num_recs, buf_start, buf_end);
	
	//error was returned when ll_open was called, no file to seek
	if (ll_fd == -1)
		return -1;

	//ll_read will get the correct record, no seeking required
	if (rec == cur_rec || rec > 65000)
		return 0;
	
	if (rec > buf_start && rec < buf_end)
		cur_rec = rec - buf_start;
	else
	{
		if (rec > buf_end)
		{
			//lseek pointer already at buf_end, add
			off_t offset = (rec - buf_end) * LLSIZE; //add rec bytes to SEEK_CUR pos

			if ( lseek(ll_fd, offset, SEEK_CUR) == -1 )
				return -1;
		}
		else
		{
			off_t offset = rec * LLSIZE; //move rec bytes away from start, or SEEK_SET

			if ( lseek(ll_fd, offset, SEEK_SET) == -1 )
				return -1;
		}
		
		buf_start = rec;
		
        int num_read = ll_reload();

        if (ll_reload() == 0)
            return -1;
        else
            buf_end = buf_start + num_recs;
	}
	
	
/*	
	//record requested is past the end of the current buffer
	if (rec > buf_end)
	{
		//lseek pointer already at buf_end, add
		off_t offset = (rec - buf_end) * LLSIZE; //add rec bytes to the SEEK_CUR position

		if ( lseek(ll_fd, offset, SEEK_CUR) == -1 )
			return -1;
		
		buf_start = rec;
		
        int num_read = ll_reload();

        if (ll_reload() == 0)
            return -1;
        else
            buf_end = buf_start + num_recs;
	}
	//record requested is before the start of the current buffer
	else if (rec < buf_start)
	{
		off_t offset = rec * LLSIZE; //move rec bytes away from start, or SEEK_SET

		if ( lseek(ll_fd, offset, SEEK_SET) == -1 )
			return -1;
		
		buf_start = rec;
        int num_read = ll_reload();

        if (num_read == 0)
        {
            return -1;
        }
        else
        {
            buf_end = buf_start + num_read;
        }
	}
	//record requested is located in the current buffer, no seeking required
	else
	{
        cur_rec = rec - buf_start;     //just update vars to match appropriate places
		
		//buf_start, buf_end, and num_recs DO NOT CHANGE because buffer stays the same
	}
*/		
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
	
	//if next to read == num in buffer AND reload doesn't get any more
	//ll_reload() will ALWAYS be called, UNLESS next to read != num in buffer
	//at open though, both are equal to 0 and reload WILL run
	if(cur_rec == num_recs)
    {
        int num_read = ll_reload();

        if (num_read == 0)
        {
            return LL_NULL;
        }
        else
        {
            buf_end = buf_start + num_read;
        }
    }
	
	struct lastlog *llp = (struct lastlog *) &llbuf[cur_rec * LLSIZE];
//	printf("\t\treading buffer at position %lu\n", (cur_rec * LLSIZE));
	cur_rec++;

	return llp;
}

/*
 * ll_next:
 *
 */
struct lastlog *ll_next()
{
	struct lastlog *llp;

    //error was returned when ll_open was called
	if (ll_fd == -1)
		return LL_NULL;
	
	if(cur_rec == num_recs && ll_reload() == 0)
        return LL_NULL;
    
	llp = (struct lastlog *) &llbuf[cur_rec * LLSIZE];
	cur_rec++;

	return llp;
}

// int ll_reset(char *fname)
// {
// 	ll_close();
// 	return ll_open(fname);
// }

/*
 * ll_reload:
 *
 */
static int ll_reload()
{
	//where to read from is set first by ll_open, then by ll_seek
	int amt_read = read(ll_fd, llbuf, (NRECS*LLSIZE));
	
	if (amt_read < 0)
		amt_read = -1;
	

	num_recs = amt_read/LLSIZE;
	cur_rec = 0;
	//furthest_rec += num_recs;

	return num_recs;
}

/*
 * ll_close:
 *		close the open file
 */
int ll_close()
{
	int value = 0;
	
	//if there is no file open, do not close it
	if (ll_fd != -1)
		value = close(ll_fd);
		
	return value;
}

