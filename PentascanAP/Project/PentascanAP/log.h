#include <stdio.h>

typedef enum {
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_INFO,
	LOG_LEVEL_STAT
}log_level;

void syslog_start(char *path);
void syslog(log_level level,char *format,...);

