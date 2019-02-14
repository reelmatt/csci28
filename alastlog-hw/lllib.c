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
static int furthest_rec;			//the highest record available
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
	furthest_rec = 0;
	
	return ll_fd;
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

	//int furthest_rec = read_rec + num_recs;

	//rec == rec_I_want
	
    if (rec > 65000)
    {
        printf("this is the nobody record... leave for now\n");
        return 0;
    }
/*
	if (rec > num_recs)
	{
		printf("need to get more recs\n");
        printf("rec requested is %d, cur_rec is %d, num_recs is %d, furthest is %d\n",
               rec, cur_rec, num_recs, furthest_rec);

		off_t offset = (rec - cur_rec) * LLSIZE;
		
		//offset from CUR position
		if ( lseek(ll_fd, offset, SEEK_CUR) == -1 )
			return -1;
			
		ll_reload();
		
		
		//OFFSET == (rec_I_want - current_rec) * LLSIZE;
	}
	else if (rec < (furthest_rec - NRECS))
	{
		printf("need to rewind, rec not in buf and appears earlier\n");
		
		off_t offset = rec * LLSIZE;

		if ( lseek(ll_fd, offset, SEEK_SET) == -1 )
			return -1;
		
		cur_rec = 0;
		num_recs = 0;
		ll_reload();
		//OFFSET == rec * LLSIZE; <-- lseek moves pointer to beginning + OFFSET
	}
	else
	{*/

    if ( rec < num_recs )
    {
		printf("in buffer, but need to access it\n");
        printf("rec requested is %d, cur_rec is %d, num_recs is %d, furthest is %d\n",
               rec, cur_rec, num_recs, furthest_rec);
		
		off_t offset = (rec - cur_rec) * LLSIZE;
		printf("offset is %ld offset\n", offset);
		//offset from cur postion, could be negative
		if ( lseek(ll_fd, offset, SEEK_CUR) == -1 )
			return -1;
		
        
		//NO NEED TO CALL RELOAD, record already in buffer
		//OFFSET == (rec_I_want - current_rec) * LLSIZE
		cur_rec = rec;
     }
	
	
//	furthest_rec = rec + amt_read; //<-- amt_read from ll_seek()
	
	return 0;
}

/*
 *
 */
struct lastlog *ll_read()
{
    struct lastlog *llp;
	//error was returned when ll_open was called
	if (ll_fd == -1)
		return LL_NULL;

//if nothing in buffer, get something there	
    if(cur_rec == num_recs && ll_reload() == 0)
        return LL_NULL;

	llp = (struct lastlog *) &llbuf[cur_rec * LLSIZE];
//	cur_rec++;
    printf("in ll_read, read from position %lu\n", (cur_rec * LLSIZE));
//    printf("test host...\t%16.16s\n", llp->ll_host);

//    printf("exiting ll_read...\n");
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

