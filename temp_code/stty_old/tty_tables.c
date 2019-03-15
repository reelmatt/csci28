/*
 * ==========================
 *   tty_tables.c
 * ==========================
 */

/* INCLUDES */
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include "sttyl.h"





// struct table_t table[] = {
// 	{ ICRNL	, "icrnl" , (tcflag_t *) ((char *)(&terminfo) + offsetof(struct termios, c_iflag))},
// 	{ OPOST	, "opost" , (tcflag_t *) ((char *)(&terminfo) + offsetof(struct termios, c_oflag))},
// 	{ HUPCL	, "opost" , (tcflag_t *) ((char *)(&terminfo) + offsetof(struct termios, c_cflag))},
// 	{ ISIG	, "opost" , (tcflag_t *) ((char *)(&terminfo) + offsetof(struct termios, c_lflag))},
// 	{ ICANON, "opost" , (tcflag_t *) ((char *)(&terminfo) + offsetof(struct termios, c_lflag))},
// 	{ ECHO	, "opost" , (tcflag_t *) ((char *)(&terminfo) + offsetof(struct termios, c_lflag))},
// 	{ ECHOE	, "opost" , (tcflag_t *) ((char *)(&terminfo) + offsetof(struct termios, c_lflag))},
// 	{ ECHOK	, "opost" , (tcflag_t *) ((char *)(&terminfo) + offsetof(struct termios, c_lflag))},
// 	{ 0		, NULL	  , 0 },
// }



/* TABLES */
struct flags input_flags[] = {
// 	{ IGNBRK	,	"Ignore BREAK condition on input" },
// 	{ BRKINT	,	"Signal interrupt on break" },
// 	{ IGNPAR	,	"Ignore chars with parity errors" },
// 	{ PARMRK	,	"Mark parity errors" },
// 	{ INPCK		,	"Enable input parity check" },
// 	{ ISTRIP	,	"Strip character" },
// 	{ INLCR		,	"Map NL to CR on input"},
// 	{ IGNCR		,	"Ignore CR"},
	{ ICRNL		,	"icrnl" },
// 	{ IXON		,	"Enable start/stop output control" },
// 	{ IXOFF		,	"Enable start/stop input control"},
	{ 0			,	NULL}
};

struct flags output_flags[] = {
	{ OPOST		,	"opost" },
//	{ ONLCR		,	"map NL to CR-NL"},
// 	{ OCRNL		,	"Map CR to NL on output" },
// 	{ ONOCR		,	"Don't output CR at column 0" },
// 	{ ONLRET	,	"Don't output CR" },
// 	{ OFILL		,	"Send fill characters for a delay" },
	{ 0			,	NULL }
};

struct flags control_flags[] = {
// 	{ CSIZE		,	"Character size mask" },
// 	{ CSTOPB	,	"Set two stop bits, rather than one" },
// 	{ CREAD		,	"Enable receiver" },
// 	{ PARENB	,	"Parity enable" },
// 	{ PARODD	,	"Odd parity, else even" },
	{ HUPCL		,	"hupcl" },
// 	{ CLOCAL	,	"Ignore modem status lines" },
//	{ CIGNORE	,	"Ignore control flags" },
	{ 0			,	NULL }
};

struct flags local_flags[] = {
	{ ISIG		,	"isig" },
	{ ICANON	,	"icanon" },
	{ ECHO		,	"echo" },
	{ ECHOE		,	"echoe" },
	{ ECHOK		,	"echok" },
// 	{ ECHONL	,	"Echo the NL character" },
// 	{ NOFLSH	,	"Disable flushing the input and output queues" },
// 	{ TOSTOP	,	"Send the SIGTTOU signal" },
	{ 0			,	NULL }
};

// struct cchars char_flags[] = {
// 	{ VEOF		,	"eof"} ,
// 	{ VEOL		,	"eol" },
// 	{ VERASE	,	"erase" },
// 	{ VINTR		,	"intr" },
// 	{ VKILL		,	"kill" },
// //	{ VMIN		,	"min" },
// 	{ VQUIT		,	"quit" },
// // 	{ VSTART	,	"start" },
// // 	{ VSTOP		,	"stop" },
// 	{ VSUSP		,	"susp" },
// //	{ VTIME		,	"time" },
// 	{ 0			,	NULL },
// };

struct table flag_tables[] = {
	{ "input_flags"		,	input_flags		, NULL },
	{ "control_flags"	,	control_flags	, NULL },
	{ "local_flags"		,	local_flags		, NULL },
	{ "output_flags"	,	output_flags	, NULL },
	{ NULL				,	NULL			, NULL }
};


/* GETTER FUNCTIONS */
struct table * get_table()
{
	return flag_tables;
}

// struct table_t * get_new_table()
// {
// 	return table;
// }

// struct cchars * get_chars()
// {
// 	return char_flags;
// }

struct flags * get_flags(char *table)
{
	int i;
	
	for(i = 0; flag_tables[i].name != NULL; i++)
	{
		if(strcmp(flag_tables[i].name, table) == 0)
			return flag_tables[i].table;
	}
	
	return NULL;
}


