
typedef enum {
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_INFO,
	LOG_LEVEL_STAT
}log_level;

typedef struct{
	unsigned long time_stamp;
	log_level level;
	char log_string[1];
}log_message;

static xSemaphoreHandle LogMutex;
	
void syslog(log_level level,char *format,...)
{
	log_message log;
	char buffer[100];
	va_list args;

	while( xSemaphoreTake( LogMutex, portMAX_DELAY ) != pdPASS );

	va_start(args,level);
	vsprintf (buffer,format, args);
	va_end (args);
	
	log = pvPortMalloc(sizeof(log_message) + strlen(buffer));
	log->level = level;
	log->time_stamp = RtcGetTime();
	strcpy(log->log_string,buffer);

	// send message queue to server task

	xSemaphoreGive(LogMutex);
}
