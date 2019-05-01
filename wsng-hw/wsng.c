#include	<stdio.h>
#include	<stdlib.h>
#include	<strings.h>
#include	<string.h>
#include	<netdb.h>
#include	<errno.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include    <sys/wait.h>
#include	<signal.h>
#include	"socklib.h"

#include	<time.h>

/*
 * wsng.c - a web server
 *
 *    usage: ws [ -c configfilenmame ]
 * features: supports the GET command only
 *           runs in the current directory
 *           forks a new child to handle each request
 *           needs many additional features 
 *
 *  compile: cc ws.c socklib.c -o ws
 *  history: 2018-04-21 added SIGINT handling (mk had it)
 *  history: 2012-04-23 removed extern declaration for fdopen (it's in stdio.h)
 *  history: 2012-04-21 more minor cleanups, expanded some fcn comments
 *  history: 2010-04-24 cleaned code, merged some of MK's ideas
 *  history: 2008-05-01 removed extra fclose that was causing double free
 */


#define	PORTNUM	80
#define	SERVER_ROOT	"."
#define	CONFIG_FILE	"wsng.conf"
#define	VERSION		"1"
#define SERVER_NAME	"WSNG"

#define	MAX_RQ_LEN	4096
#define	LINELEN		1024
#define	PARAM_LEN	128
#define	VALUE_LEN	512
#define CONTENT_LEN 64

/* Content-Types */
struct content_type { char *extension; char *value; };
struct content_type table[] = {
    {"html", "text/html"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"css", "text/css"},
    {"gif", "image/gif"},
    {"png", "image/png"},
    {"txt", "text/plain"},
    {"js", "text/javascript"},
    {NULL, NULL},
};

char	myhost[MAXHOSTNAMELEN];
char    content_default[CONTENT_LEN];
int	myport;
char	*full_hostname();

#define	oops(m,x)	{ perror(m); exit(x); }

/*
 * prototypes
 */

int	startup(int, char *a[], char [], int *);
void	read_til_crnl(FILE *);
void	process_rq( char *, FILE *);
void	bad_request(FILE *);
void	cannot_do(FILE *fp);
void	do_404(char *item, FILE *fp);
void	do_403(char *item, FILE *fp);
void    do_head(FILE * fp);
void	do_cat(char *f, FILE *fpsock);
void	do_exec( char *prog, FILE *fp);
void	do_ls(char *dir, FILE *fp);
void process_dir(char *dir, FILE *fp);
void    output_listing(FILE * pp, FILE * fp, char *dir);
char *get_content_type(char *ext);
int	ends_in_cgi(char *f);
char 	*file_type(char *f);
void	header( FILE *fp, int code, char *msg, char *content_type );
int	isadir(char *f);
char	*modify_argument(char *arg, int len);
int	not_exist(char *f);
int no_access(char *f);
void	fatal(char *, char *);
void	handle_call(int);
int	read_request(FILE *, char *, int);
char	*readline(char *, int, FILE *);
void sigchld_handler(int s);
char *parse_query(char *line);
void set_content_type(char *ext, char *val);
void process_config_type(char [PARAM_LEN], char [VALUE_LEN], char [CONTENT_LEN]);

int	mysocket = -1;		/* for SIGINT handler */

int
main(int ac, char *av[])
{
	int 	sock, fd;

	/* set up */
	sock = startup(ac, av, myhost, &myport);
	mysocket = sock;

	/* sign on */
	printf("wsng%s started.  host=%s port=%d\n", VERSION, myhost, myport);

	/* main loop here */
	while(1)
	{
		fd    = accept( sock, NULL, NULL );	/* take a call	*/
		if ( fd == -1 )
		{
            if( errno == EINTR)		/* check if intr from sigchld */
                continue;
                
            perror("accept");
		}
		else
			handle_call(fd);		/* handle call	*/
	}
	return 0;
	/* never end */
}

/*
 *	sigchld_handler()
 *	Purpose: Handle exit statuses from child process to prevent zombies
 *	Note: taken from the 'zombierace' page provided with the assignment
 *		  materials for the assignment.
 */
void
sigchld_handler(int s)
{
    int old_errno = errno;
    
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = old_errno;
}

/*
 * handle_call(fd) - serve the request arriving on fd
 * summary: fork, then get request, then process request
 *    rets: child exits with 1 for error, 0 for ok
 *    note: closes fd in parent
 */
void handle_call(int fd)
{
	int	pid = fork();
	FILE	*fpin, *fpout;
	char	request[MAX_RQ_LEN];

	if ( pid == -1 ){
		perror("fork");
		return;
	}

	/* child: buffer socket and talk with client */
	if ( pid == 0 )
	{
		fpin  = fdopen(fd, "r");
		fpout = fdopen(fd, "w");
		if ( fpin == NULL || fpout == NULL )
			exit(1);

		if ( read_request(fpin, request, MAX_RQ_LEN) == -1 )
			exit(1);
		printf("got a call: request = %s", request);

		process_rq(request, fpout);
		fflush(fpout);		/* send data to client	*/
		exit(0);		/* child is done	*/
					/* exit closes files	*/
	}
	/* parent: close fd and return to take next call	*/
	close(fd);
}

/*
 * read the http request into rq not to exceed rqlen
 * return -1 for error, 0 for success
 */
int read_request(FILE *fp, char rq[], int rqlen)
{
	/* null means EOF or error. Either way there is no request */
	if ( readline(rq, rqlen, fp) == NULL )
		return -1;
	read_til_crnl(fp);
	return 0;
}

void read_til_crnl(FILE *fp)
{
        char    buf[MAX_RQ_LEN];
        while( readline(buf,MAX_RQ_LEN,fp) != NULL 
			&& strcmp(buf,"\r\n") != 0 )
                ;
}

/*
 * readline -- read in a line from fp, stop at \n 
 *    args: buf - place to store line
 *          len - size of buffer
 *          fp  - input stream
 *    rets: NULL at EOF else the buffer
 *    note: will not overflow buffer, but will read until \n or EOF
 *          thus will lose data if line exceeds len-2 chars
 *    note: like fgets but will always read until \n even if it loses data
 */
char *readline(char *buf, int len, FILE *fp)
{
        int     space = len - 2;
        char    *cp = buf;
        int     c;

        while( ( c = getc(fp) ) != '\n' && c != EOF ){
                if ( space-- > 0 )
                        *cp++ = c;
        }
        if ( c == '\n' )
                *cp++ = c;
        *cp = '\0';
        return ( c == EOF && cp == buf ? NULL : buf );
}
/*
 * initialization function
 * 	1. process command line args
 *		handles -c configfile
 *	2. open config file
 *		read rootdir, port
 *	3. chdir to rootdir
 *	4. open a socket on port
 *	5. gets the hostname
 *	6. return the socket
 *       later, it might set up logfiles, check config files,
 *         arrange to handle signals
 *
 *  returns: socket as the return value
 *	     the host by writing it into host[]
 *	     the port by writing it into *portnump
 */
int startup(int ac, char *av[], char host[], int *portnump)
{
	int	sock;
	int	portnum     = PORTNUM;
	char	*configfile = CONFIG_FILE ;
	int	pos;
	void	process_config_file(char *, int *);
	void	done(int);

	signal(SIGINT, done);
	for(pos=1;pos<ac;pos++){
		if ( strcmp(av[pos],"-c") == 0 ){
			if ( ++pos < ac )
				configfile = av[pos];
			else
				fatal("missing arg for -c",NULL);
		}
	}
	process_config_file(configfile, &portnum);
			
	sock = make_server_socket( portnum );
	if ( sock == -1 ) 
		oops("making socket",2);
	strcpy(myhost, full_hostname());
	*portnump = portnum;
	
	signal(SIGCHLD, sigchld_handler);	/* handler for zombies */
	
	return sock;
}


/*
 * opens file or dies
 * reads file for lines with the format
 *   port ###
 *   server_root path
 * at the end, return the portnum by loading *portnump
 * and chdir to the rootdir
 */
void process_config_file(char *conf_file, int *portnump)
{
	FILE	*fp;
	char	rootdir[VALUE_LEN] = SERVER_ROOT;
	char	param[PARAM_LEN];
	char	value[VALUE_LEN];
	char	type[CONTENT_LEN];
	int	port;
	int	read_param(FILE *, char *, int, char *, int, char *, int );

	/* open the file */
	if ( (fp = fopen(conf_file,"r")) == NULL )
		fatal("Cannot open config file %s", conf_file);

	printf("reading the config file...\n");

	/* extract the settings */
	while( read_param(fp, param, PARAM_LEN, value, VALUE_LEN, type, CONTENT_LEN) != EOF )
	{
		if ( strcasecmp(param,"server_root") == 0 )
			strcpy(rootdir, value);
		if ( strcasecmp(param,"port") == 0 )
			port = atoi(value);
		if ( strcasecmp(param,"type") == 0)
			process_config_type(param, value, type);
// 		if ( strcasecmp(param,"type") == 0)
// 		    type[type] = value;
	}
	fclose(fp);

//     strcpy(content_default, "text/plain");
//     content_default = "text/plain";

	/* act on the settings */
	if (chdir(rootdir) == -1)
		oops("cannot change to rootdir", 2);
	*portnump = port;
	return;
}

void process_config_type(char param[PARAM_LEN], char val[VALUE_LEN], char type[CONTENT_LEN])
{
	printf("in process_config_type\n");
	printf("param is %s, val is %s, type is %s\n", param, val, type);
	
	if (strcmp(val, "DEFAULT") && type)
		strcpy(content_default, type);
	else
		set_content_type(val, type);	
}

/*
 * read_param:
 *   purpose -- read next parameter setting line from fp
 *   details -- a param-setting line looks like  name value
 *		for example:  port 4444
 *     extra -- skip over lines that start with # and those
 *		that do not contain two strings
 *   returns -- EOF at eof and 1 on good data
 *
 */
int
read_param (FILE *fp, 
			char *name, int nlen,	// place to store name
			char* value, int vlen,	// place to store value
			char* type, int clen)	// place to store content-type
{
	char	line[LINELEN];
	int	c;
	char	fmt[100] ;

	sprintf(fmt, "%%%ds%%%ds%%%ds", nlen, vlen, clen);

	/* read in next line and if the line is too long, read until \n */
	while( fgets(line, LINELEN, fp) != NULL )
	{
		if ( line[strlen(line)-1] != '\n' )
			while( (c = getc(fp)) != '\n' && c != EOF )
				;

		int num_args = sscanf(line, fmt, name, value, type );
		printf("num_args is %d\n", num_args);
		printf("they are: 1) %s, 2) %s, and 3) %s\n", name, value, type);
		
		if ( (num_args == 2 || num_args == 3) && *name != '#')
		{
// 			if(*name != '#')
				return 1;
		}
		
		// if ( sscanf(line, fmt, name, value ) == 2 && *name != '#' )
// 			return 1;
	}
	return EOF;
}
	


/* ------------------------------------------------------ *
   process_rq( char *rq, FILE *fpout)
   do what the request asks for and write reply to fp
   rq is HTTP command:  GET /foo/bar.html HTTP/1.0
   ------------------------------------------------------ */

void process_rq(char *rq, FILE *fp)
{
	char	cmd[MAX_RQ_LEN], arg[MAX_RQ_LEN];
	char	*item, *modify_argument();

	if ( sscanf(rq, "%s%s", cmd, arg) != 2 ){
		bad_request(fp);
		return;
	}

	item = modify_argument(arg, MAX_RQ_LEN);
	item = parse_query(item);
	
	if ( strcmp(cmd, "HEAD") == 0 )
	    setenv("REQUEST_METHOD", "HEAD", 1);

	if ( strcmp(cmd,"GET") != 0 )
		cannot_do(fp);
	else if ( not_exist( item ) )
		do_404(item, fp );
	else if ( no_access( item) )
	    do_403(item, fp);
	else if ( isadir( item ) )
        process_dir( item, fp );
	else if ( ends_in_cgi( item ) )
		do_exec( item, fp );
	else
		do_cat( item, fp );
}

/*
 *	parse_query()
 *	Purpose: Parse the line and if a query, set QUERY_STRING
 *	  Input: line, the argument to parse
 *	 Return: If there is a query, store it in the QUERY_STRING env
 *			 variable. Return the rest of the argument, minus the query.
 */
char *
parse_query(char *line)
{
    char *query = strrchr(line, '?');

	if (query != NULL)
	{
		// set environment variables
		setenv("QUERY_STRING", (query + 1), 1);
		setenv("REQUEST_METHOD", "GET", 1);
		
		*query = '\0';	// terminate at the '?'
	}
	
    return line;
}


/*
 * modify_argument
 *  purpose: many roles
 *		security - remove all ".." components in paths
 *		cleaning - if arg is "/" convert to "."
 *  returns: pointer to modified string
 *     args: array containing arg and length of that array
 */

char *
modify_argument(char *arg, int len)
{
	char	*nexttoken;
	char	*copy = malloc(len);

	if ( copy == NULL )
		oops("memory error", 1);

	/* remove all ".." components from path */
	/* by tokeninzing on "/" and rebuilding */
	/* the string without the ".." items	*/

	*copy = '\0';

	nexttoken = strtok(arg, "/");
	while( nexttoken != NULL )
	{
		if ( strcmp(nexttoken,"..") != 0 )
		{
			if ( *copy )
				strcat(copy, "/");
			strcat(copy, nexttoken);
		}
		nexttoken = strtok(NULL, "/");
	}
	strcpy(arg, copy);
	free(copy);

	/* the array is now cleaned up */
	/* handle a special case       */

	if ( strcmp(arg,"") == 0 )
		strcpy(arg, ".");
	return arg;
}
/* ------------------------------------------------------ *
   the reply header thing: all functions need one
   if content_type is NULL then don't send content type
   ------------------------------------------------------ */

void
header( FILE *fp, int code, char *msg, char *content_type )
{
	//from web-time.c
    char * rfc822_time(time_t thetime);

	fprintf(fp, "HTTP/1.0 %d %s\r\n", code, msg);
    fprintf(fp, "Date: %s\r\n", rfc822_time(time(0L)));
	fprintf(fp, "Server: %s/%s\r\n", SERVER_NAME, VERSION);
	
	if ( content_type )
		fprintf(fp, "Content-Type: %s\r\n", content_type );
}

/* ------------------------------------------------------ *
   simple functions first:
   bad_request(fp)     bad request syntax
     cannot_do(fp)     unimplemented HTTP command
   do_404(item,fp)     no such object
   do_403(item,fp)	   wrong permissions (added by MT)
   ------------------------------------------------------ */

void
bad_request(FILE *fp)
{
	header(fp, 400, "Bad Request", "text/plain");
	fprintf(fp, "\r\nI cannot understand your request\r\n");
}

void
cannot_do(FILE *fp)
{
	header(fp, 501, "Not Implemented", "text/plain");
	fprintf(fp, "\r\n");

	fprintf(fp, "That command is not yet implemented\r\n");
}

void
do_404(char *item, FILE *fp)
{
	header(fp, 404, "Not Found", "text/plain");
	fprintf(fp, "\r\n");

	fprintf(fp, "The item you requested: %s\r\nis not found\r\n", 
			item);
}

void
do_403(char *item, FILE *fp)
{
	header(fp, 403, "Forbidden", "text/plain");
	fprintf(fp, "\r\n");

	fprintf(fp, "You do not have permission to access %s on this server\r\n",
	            item);
}

void
do_head(FILE * fp)
{
    header(fp, 200, "OK", "text/plain");
}

/* ------------------------------------------------------ *
   the directory listing section
   isadir() uses stat, not_exist() uses stat
   no_access() checks permissions of dir using stat
   process_dir() checks if an 'index.html' or 'index.cgi'
   		file exists. If yes, it outputs that, otherwise
   		calls do_ls().
   do_ls() opens a pipe to a listing of the directory.
   		For each entry, it formats the line with a link
   		to that file.
   ------------------------------------------------------ */

int
isadir(char *f)
{
	struct stat info;
	return ( stat(f, &info) != -1 && S_ISDIR(info.st_mode) );
}

int
not_exist(char *f)
{
	struct stat info;

	return( stat(f,&info) == -1 && errno == ENOENT );
}

/*
 *	no_access()
 *	Purpose: check permissions of page/file trying to be loaded
 *	  Input: f, the name of the file
 *	 Return: 1, if not allowed to access; 0 otherwise
 */
int
no_access(char *f)
{
	struct stat info;
	char path[LINELEN];

	// construct the path to the file
	strcpy(path, "./");
	snprintf(path, LINELEN, "%s/", f);

	// get the permissions info
	if( stat(path, &info) != -1 )
	{
		// there is no access allowed, return 1 to direct to a 403
		if(! (S_IRUSR & info.st_mode) || ! (S_IXUSR & info.st_mode) )
			return 1;
	}
	return 0;

}

/*
 *	process_dir()
 *	Purpose: check the current directory to see if an index file exists
 */
void
process_dir(char *dir, FILE *fp)
{
    struct stat info;
    char html[LINELEN];
    char cgi[LINELEN];
    
    // create a path to check HTML index
    strcpy(html, dir);
    strcat(html, "/index.html");
    
    // create a path to check CGI index
    strcpy(cgi, dir);
    strcat(cgi, "/index.cgi");

    if(stat(html, &info) == 0 )		// html exists
        do_cat(html, fp);
    else if (stat(cgi, &info) == 0)	// cgi exists
        do_exec(cgi, fp);
    else							// no index, output listing
        do_ls(dir, fp);
    
    return;
}

/*
 * lists the directory named by 'dir' 
 * sends the listing to the stream at fp
 *
 * Note: Modified for the assignment. 
 */
void
do_ls(char *dir, FILE *fp)
{
    int cmd_len = strlen(dir) + 7;
    char command[cmd_len];
    
    // construct command with directory name
    snprintf(command, cmd_len, "%s %s", "ls -l", dir);

    FILE *pp = popen(command, "r");
    if (pp == NULL)
    {
        perror(dir);
        return;
    }

	header(fp, 200, "OK", "text/html");
	fprintf(fp,"\r\n");
    
    output_listing(pp, fp, dir);
    
	if(pclose(pp) == -1)
        perror("oops");
}

/*
 *	output_listing()
 *	Purpose: Take output from an `ls` command and format with links
 *	  Input: pp, pointer to pipe FILE
 *			 fp, pointer to socket FILE
 *			 dir, parent directory to use for link paths
 *
 */
void
output_listing(FILE * pp, FILE * fp, char *dir)
{
    int num_files = -1;
    char line[LINELEN], copy[LINELEN];

    while(fgets(line, LINELEN, pp) != NULL)
    {
		// skip the first line of `ls` displaying `total #`
        if (++num_files == 0)
            continue;

        char *file = strrchr(line, ' ');	// get the last arg
        file[strlen(file) - 1] = '\0';		// terminate

        if(file != NULL)					// check there was an arg
        {
            file++;							// strip the space
			
            char link[LINELEN];				// construct the link
            snprintf(link, LINELEN, "<a href='%s/%s'>%s</a><br/>\n",
            		 dir, file, file);
            		 
            // construct the whole line
            strncpy( copy, line, (strlen(line) - strlen(file)) );
            strncat(copy, link, LINELEN);
        }

		// send to the socket
        fprintf(fp, "%s", copy);
        
        //reset
        memset(copy, 0, LINELEN);
    }
    return;
}

/* ------------------------------------------------------ *
   the cgi stuff.  function to check extension and
   one to run the program.
   ------------------------------------------------------ */

char *
file_type(char *f)
/* returns 'extension' of file */
{
	char	*cp;
	if ( (cp = strrchr(f, '.' )) != NULL )
		return cp+1;
	return "";
}

int
ends_in_cgi(char *f)
{
	return ( strcmp( file_type(f), "cgi" ) == 0 );
}

void
do_exec( char *prog, FILE *fp)
{
	int	fd = fileno(fp);

	header(fp, 200, "OK", NULL);
	fflush(fp);

	dup2(fd, 1);
	dup2(fd, 2);
	execl(prog,prog,NULL);
	perror(prog);
}
/* ------------------------------------------------------ *
   do_cat(filename,fp)
   sends back contents after a header
   ------------------------------------------------------ */

/*
 *	Modified from starter code. Moved Content-Type from if/else
 *	switch, to a table-driven design. See get_content_type().
 */
void
do_cat(char *f, FILE *fpsock)
{
	char	*extension = file_type(f);
	char	*content = get_content_type(extension);

	FILE	*fpfile;
	int	c;

	fpfile = fopen( f , "r");
	if ( fpfile != NULL )
	{
		header( fpsock, 200, "OK", content );
		fprintf(fpsock, "\r\n");
		while( (c = getc(fpfile) ) != EOF )
			putc(c, fpsock);
		fclose(fpfile);
	}
}

/*
 *	get_content_type()
 *	Purpose: Look up a given `ext` to see if there is a Content-Type match
 *	  Input: ext, the file extension to lookup
 *	 Method: Search the table of `struct content_type` to see if a match
 *			 exists for `ext`. If there is a match, return that type.
 *			 Otherwise, return the default.
 */
char *
get_content_type(char *ext)
{
    int i;
    
    for(i = 0; table[i].extension != NULL; i++)
    {
        if(strcmp(ext, table[i].extension) == 0)
            return table[i].value;
    }
    
    return content_default;
}

void set_content_type(char *ext, char *val)
{
	int i;
	
	for(i = 0; table[i].extension != NULL; i++)
	{
		if(strcmp(ext, table[i].extension) == 0)
		{
			strcpy(table[i].value, val);
			return;
		}
	}
}


char *
full_hostname()
/*
 * returns full `official' hostname for current machine
 * NOTE: this returns a ptr to a static buffer that is
 *       overwritten with each call. ( you know what to do.)
 */
{
	struct	hostent		*hp;
	char	hname[MAXHOSTNAMELEN];
	static  char fullname[MAXHOSTNAMELEN];

	if ( gethostname(hname,MAXHOSTNAMELEN) == -1 )	/* get rel name	*/
	{
		perror("gethostname");
		exit(1);
	}
	hp = gethostbyname( hname );		/* get info about host	*/
	if ( hp == NULL )			/*   or die		*/
		return NULL;
	strcpy( fullname, hp->h_name );		/* store foo.bar.com	*/
	return fullname;			/* and return it	*/
}


void fatal(char *fmt, char *str)
{
	fprintf(stderr, fmt, str);
	exit(1);
}

void done(int n)
{
	if ( mysocket != -1 ){
		fprintf(stderr, "closing socket\n");
		close(mysocket);
	}
	exit(0);
}
