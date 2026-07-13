#include "log.h"
#include "error_defs.h"
#include <stdio.h>

FILE *logfile = NULL;
const char *log_file_name = "logmsg.txt.ignore";

enum Error_Type log_message(enum Log_Level l, const char *message)
{
	if (logfile == NULL) {
		logfile = fopen(log_file_name, "a");
		if (logfile == NULL) {
			return E_OS;
		}
	}
	fprintf(logfile, "%s : %s\n",
		(l == LOG_ERROR) ? "ERROR" : "MESSAGE",
		message
	);
	return E_OK;
}
