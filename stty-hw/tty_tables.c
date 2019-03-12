#include <termios.h>
#include <stdio.h>
#include <string.h>
#include "sttyl.h"

//struct flaginfo {tcflag_t fl_value; char *fl_name; };
//struct cflaginfo {cc_t c_value; char *c_name; };

/*
 * ==========================
 *   TABLES -- to be moved
 * ==========================
 */

// typedef struct flaginfo f_table;
// typedef struct cflaginfo c_table;

f_info input_flags[] = {
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

f_info output_flags[] = {
	{ OPOST		,	"opost" },
//	{ ONLCR		,	"map NL to CR-NL"},
// 	{ OCRNL		,	"Map CR to NL on output" },
// 	{ ONOCR		,	"Don't output CR at column 0" },
// 	{ ONLRET	,	"Don't output CR" },
// 	{ OFILL		,	"Send fill characters for a delay" },
	{ 0			,	NULL }
};

f_info control_flags[] = {
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

f_info local_flags[] = {
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

struct cflaginfo char_flags[] = {
	{ VEOF		,	"eof"} ,
	{ VEOL		,	"eol" },
	{ VERASE	,	"erase" },
	{ VINTR		,	"intr" },
	{ VKILL		,	"kill" },
//	{ VMIN		,	"min" },
	{ VQUIT		,	"quit" },
// 	{ VSTART	,	"start" },
// 	{ VSTOP		,	"stop" },
	{ VSUSP		,	"susp" },
//	{ VTIME		,	"time" },
	{ 0			,	NULL },
};

// struct flags all_flags[] = {
// 	output_flags, 
// 	control_flags,
// 	local_flags,
// 	input_flags
// };

// struct table = { f_table [] };
// f_table all_flags[] = {input_flags, output_flags, control_flags, local_flags};
struct table_entry all_flags[] = {
	{ "input_flags"		,	input_flags	 , NULL},
	{ "control_flags"	,	control_flags , NULL },
	{ "local_flags"		,	local_flags , NULL	 },
	{ "output_flags"	,	output_flags , NULL},
	{ NULL				,	NULL , NULL }
};

struct table_entry * get_full_table()
{
	return all_flags;
// 	int i;
// 	
// 	for(i = 0; all_flags[i].table_name != NULL; i++)
// 	{
// 		if(strcmp(all_flags[i].table_name, table) == 0)
// 		{
// 			return &all_flags[i];
// 		}
// 	}
// 	
// 	return NULL;
}

f_info * get_table(char *table)
{
	int i;
	
	for(i = 0; all_flags[i].table_name != NULL; i++)
	{
		if(strcmp(all_flags[i].table_name, table) == 0)
		{
			return all_flags[i].table;
		}
	}
	
	return NULL;
}

c_info * get_chars()
{
	return char_flags;
}




