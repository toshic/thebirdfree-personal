/* Standard includes. */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/* lwip library */
#include "lwiplib.h"
/* http client header */
#include "ff.h"
#include "Rtc.h"

#include "log.h"

const char *log_level_string[] =
{
    "LOG_EROR",
    "LOG_WARN",
    "LOG_INFO",
    "LOG_STAT"
};

typedef struct{
	unsigned long time_stamp;
	log_level level;
	char log_string[1];
}log_message;

static xSemaphoreHandle LogMutex;
static xQueueHandle LogQueue;
static log_level syslog_level;
static char *va_buffer;

void dbglevel(log_level level)
{
    syslog_level = level;
}
	
void syslog(log_level level,char *format,...)
{
	log_message *log;
	va_list args;

	while( xSemaphoreTake( LogMutex, portMAX_DELAY ) != pdPASS );

	va_start(args,format);
	vsprintf (va_buffer,format, args);
	va_end (args);

	log = mem_malloc(sizeof(log_message) + strlen(va_buffer));
	if(log == NULL){
	    fprintf(stderr,"syslog malloc Fail\n");
	    printf(va_buffer);
        xSemaphoreGive(LogMutex);
	    return;
	}
	
	log->level = level;
	log->time_stamp = RtcGetTime();
	strcpy(log->log_string,va_buffer);

	// send message queue to server task
	xQueueSend(LogQueue, (void*)&log, 0);

	xSemaphoreGive(LogMutex);
}

static void filelog(char *path,char *ts,char *string)
{
    FRESULT fresult;
    FIL *FileObject = mem_malloc(sizeof(FIL));
    unsigned int usBytesWritten;

    fresult = f_open(FileObject, path, FA_WRITE | FA_OPEN_ALWAYS);
    if(fresult != FR_OK)
        goto file_error;

    // seek to end of file
    fresult = f_lseek(FileObject, f_size(FileObject));
    if(fresult != FR_OK)
    {
        f_close(FileObject);
        goto file_error;
    }

    fresult = f_write(FileObject, ts, strlen(ts),
                     &usBytesWritten);
    if(fresult != FR_OK || strlen(ts) != usBytesWritten)
    {
        f_close(FileObject);
        goto file_error;
    }

    fresult = f_write(FileObject, string, strlen(string),
                     &usBytesWritten);
    if(fresult != FR_OK || strlen(string) != usBytesWritten)
    {
        f_close(FileObject);
        goto file_error;
    }

    fresult = f_write(FileObject, "\n", strlen("\n"),
                     &usBytesWritten);
    if(fresult != FR_OK || strlen("\n") != usBytesWritten)
    {
        f_close(FileObject);
        goto file_error;
    }

    f_close(FileObject);
    mem_free(FileObject);
    return;
    
file_error:
    printf("filelog fail\n");
    printf(string);
    printf("--------------------------\n");
    mem_free(FileObject);
    return;
}

void syslogd(void *pv)
{
    log_message *log;
    char *path;
    char *timestamp = mem_malloc(60);
    struct tm * timeinfo;
    
    LogQueue = xQueueCreate( 20, sizeof(log_message *) );
    if(LogQueue == NULL){
        printf("Console queue creation fail\n");
        vTaskDelete( NULL );
        return;
    }
    
    LogMutex = xSemaphoreCreateMutex();
    if(LogMutex == NULL){
        printf("syslog mutex creation fail\n");
        vTaskDelete( NULL );
        return;
    }
    
    va_buffer = mem_malloc(1024);
    if(va_buffer == NULL){
        printf("buffer malloc fail\n");
        vTaskDelete( NULL );
        return;
    }
    path = mem_malloc(strlen((char*)pv) + 1);
    if(path == NULL){
        printf("buffer malloc fail\n");
        vTaskDelete( NULL );
        return;
    }
    strcpy(path,(char*)pv);

    for(;;){
        xQueueReceive(LogQueue, &log, portMAX_DELAY);
        timeinfo = localtime(&(log->time_stamp));
        sprintf(timestamp,"<syslog - %04d/%02d/%02d %02d:%02d:%02d %s>\n",timeinfo->tm_year + 1900, timeinfo->tm_mon+ 1 , timeinfo->tm_mday,
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,log_level_string[log->level]);

        /* print if level is below log_level */
        if(syslog_level <= log->level){
            printf("%s%s\n",timestamp,log->log_string);
        }
        /* store file log */
        filelog(path,timestamp,log->log_string);
        /* free log struct */
        mem_free(log);
    }
}

void syslog_start(char *path)
{
    static int start;
    if(!start){
        /* simple way to prevent double excution */
        start = 1;
        xTaskCreate( syslogd, ( signed portCHAR * ) "syslogd", 128, (void*)path, tskIDLE_PRIORITY + 1, NULL );
    }
}
