#include <termios.h>
#include <stdio.h>
#include "sttyl.h"

//struct flaginfo {tcflag_t fl_value; char *fl_name; };
//struct cflaginfo {cc_t c_value; char *c_name; };


/*
struct flaginfo input_flags[] = {
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


struct flaginfo * get_input_flags()
{
	return input_flags;
}




// 
// struct flaginfo get_input_flags()[]
// {
// 	return input_flags;
// }

struct flaginfo output_flags[] = {
	{ OPOST		,	"opost" },
//	{ ONLCR		,	"map NL to CR-NL"},
// 	{ OCRNL		,	"Map CR to NL on output" },
// 	{ ONOCR		,	"Don't output CR at column 0" },
// 	{ ONLRET	,	"Don't output CR" },
// 	{ OFILL		,	"Send fill characters for a delay" },
	{ 0			,	NULL }
};


struct flaginfo control_flags[] = {
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


struct flaginfo local_flags[] = {
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
	{ VEOF		,	"EOF"} ,
	{ VEOL		,	"EOL" },
	{ VERASE	,	"Erase char" },
	{ VINTR		,	"Interrupt" },
	{ VKILL		,	"Kill char" },
	{ VMIN		,	"Minimum number of chars for noncanon read" },
	{ VQUIT		,	"Quit" },
	{ VSTART	,	"Start character" },
	{ VSTOP		,	"Stop character" },
	{ VSUSP		,	"suspend char" },
	{ VTIME		,	"Timeout in deciseconds for noncanon" },
	{ 0			,	NULL },
};


*/