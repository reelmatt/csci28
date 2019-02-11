#include <stdio.h>
#include <fcntl.h>
#include <lastlog.h>
#include <unistd.h>
#include "lllib.h"

#define NRECS 32
#define LLSIZE	(sizeof(struct lastlog))
#define LL_NULL ((struct lastlog *) NULL)

static char llbuf[NRECS * LLSIZE];	//buffer storage
static int num_recs;				//num in buffer
static int cur_rec;					//next rec to read
static int ll_fd = -1;				//file descriptor

static int ll_reload();


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
	
	return ll_fd;
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

int ll_reset(char *fname)
{
	ll_close();
	return ll_open(fname);
}

/*
 * ll_reload:
 *
 */
static int ll_reload()
{
	int amt_read = read(ll_fd, llbuf, (NRECS*LLSIZE));
	
	if (amt_read < 0)
		amt_read = -1;
	
	num_recs = amt_read/LLSIZE;
	cur_rec = 0;
//    printf("IN lllib.c, ll_reload got %d records to read\n", num_recs);
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

