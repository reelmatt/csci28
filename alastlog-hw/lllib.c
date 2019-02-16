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
static int ll_fd = -1;				//file descriptor

static int ll_reload();


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
		
	return ll_fd;
}

/*
 *	ll_seek()
 *	Purpose: reposition location where next record is read from
 *	 Return: -1 on error, 0 on success
 *	  Input: rec, the index (based on UID) of the record requested
 *	 Method: If the rec equals cur_rec, no seeking needed; the next record
 *			 that will be read is correct. If the rec falls within the current
 *			 buffer, just update cur_rec to the appropriate index. If rec is
 *			 outside the buffer, calculate the offset and call lseek() to
 *			 adjust the file pointer read() will use to load a new buffer.
 *	   Note: When calling lseek() to prep for reading into the buffer, offset
 *			 is calculated to return to buffer at the record requested.
 */
int ll_seek(int rec)
{
	//error was returned when ll_open was called, no file to seek
	if (ll_fd == -1)
		return -1;

	//ll_read will get the correct record, no seeking required
	if (rec == cur_rec)
		return 0;

	//if rec is within the current buffer, update cur_rec index to read	
	if (rec > buf_start && rec < (buf_start + num_recs))
		cur_rec = rec - buf_start;
	else
	{
		off_t offset = rec * LLSIZE;
		
		if ( lseek(ll_fd, offset, SEEK_SET) == -1 )
				return -1;
		
		buf_start = rec;
		
        if (ll_reload() <= 0)
            return -1;    
	}	
	return 0;
}

/*
 *	ll_reload()
 *	Purpose: read the lastlog record located at cur_rec in the current buffer
 *	 Return: pointer to the lastlog record located in the buffer
 *	   Note: copied (with minor modifications), from utmplib.c file. Provided
 *			 in assignment files, also used in lecture 02.
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
/*	if(cur_rec == num_recs)
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
    }*/
    
    if(cur_rec == num_recs && ll_reload() == 0)
    	return LL_NULL;
	
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
	
	if (amt_read < 0)
		amt_read = -1;

	num_recs = amt_read/LLSIZE;
	cur_rec = 0;

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
	
	//if there is no file open, do not close it
	if (ll_fd != -1)
		value = close(ll_fd);
		
	return value;
}
