/* Scheduler includes. */
#include "FreeRTOS.h"
/* Library includes. */
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_types.h"
#include "interrupt.h"
#include "sysctl.h"
#include "lmi_timer.h"
#include "hw_timer.h"

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static volatile unsigned long timeval;
static volatile unsigned long timeoffset;

/* Counts the total number of times that the high frequency timer has 'ticked'.
This value is used by the run time stats function to work out what percentage
of CPU time each task is taking. */
volatile unsigned long ulHighFrequencyTimerTicks = 0UL;

static const char * const g_strweekday[] = {
    "Mon","Tue","Wed","Thu","Fri","Sat","Sun"
};

static const char * const g_strmonth[] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

struct
{
    char *zone;
    int   diff;
}
TZ[] =
{
   { "UTC",  0 }, { "GMT",  0 },            // Greenwich mean time
   { "AST", -4 }, { "ADT", -3 },            // Atlantic time
   { "EST", -5 }, { "EDT", -4 },            // Eastern time
   { "CST", -6 }, { "CDT", -5 },            // Central time
   { "MST", -7 }, { "MDT", -6 },            // Mountain time
   { "PST", -8 }, { "PDT", -7 },            // Pacific time
   { "KST", -9 }, { "KDT", -8 },            // Alaskan time
   { "YST", -9 }, { "YDT", -8 },            // Yukon time
   { "HST", -10}, { "HDT", -9 },            // Hawaii-Aleutian time
   { "SST", -11}, { "SDT", -10},            // Samoa time 
   { "",     0 }
};


void RtcInit(void)
{
	SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER0 );
    TimerConfigure( TIMER0_BASE, TIMER_CFG_32_BIT_PER );
	
	/* Ensure interrupts do not start until the scheduler is running. */
	portDISABLE_INTERRUPTS();
	
	/* The rate at which the timer will interrupt. */
    TimerLoadSet( TIMER0_BASE, TIMER_A, configCPU_CLOCK_HZ );
	IntPrioritySet( INT_TIMER0A, configKERNEL_INTERRUPT_PRIORITY );
    IntEnable( INT_TIMER0A );
    TimerIntEnable( TIMER0_BASE, TIMER_TIMA_TIMEOUT );

    /* setup timer 1 for cpu usage statistic */
    SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER1 );
    TimerConfigure( TIMER1_BASE, TIMER_CFG_32_BIT_PER );

	/* Just used to measure time. */
    TimerLoadSet(TIMER1_BASE, TIMER_A, configCPU_CLOCK_HZ / 10000 );    /* sys tick = 1msec, so 100usec will be set */
	IntPrioritySet( INT_TIMER1A, configMAX_SYSCALL_INTERRUPT_PRIORITY );
    IntEnable( INT_TIMER1A );
    TimerIntEnable( TIMER1_BASE, TIMER_TIMA_TIMEOUT );

    timeval = 0;
    timeoffset = 0;

    ulHighFrequencyTimerTicks = 0;

	/* Enable rtc timer. */	
    TimerEnable( TIMER0_BASE, TIMER_A );
    TimerEnable( TIMER1_BASE, TIMER_A );
}

unsigned long RtcGetTime(void)
{
    return timeval + timeoffset;
}

unsigned long RtcGetTimeFromTick(unsigned long tick)
{
    return timeval + tick;
}

unsigned long RtcGetTick(void)
{
    return timeval;
}

void RtcSetTime(unsigned long new_time)
{
    timeoffset = new_time - timeval;
}

unsigned long RtcConvertDateString(char *date_string)
{
    char *ptr;
    char digit[3];
    struct tm timeinfo;
    unsigned long time;

    int i;
    
    if(date_string == NULL)
        return 0;

    ptr = strchr(date_string,',');
    if(ptr)
        ptr++;
    else
        ptr = date_string;

    // skip space
    while(*ptr && *ptr == ' ')
        ptr++;

    // day
    ptr = strtok(ptr," \r\n");
    if(!ptr)
        return 0;
    timeinfo.tm_mday = atoi(ptr);

    // month
    ptr = strtok(NULL," \r\n");
    if(!ptr)
        return 0;
    for(i=0;i<12;i++)
        if(!strcmp(ptr,g_strmonth[i])){
            timeinfo.tm_mon = i;
            break;
        }
    if(i==12)
        return 0;
        
    // year    
    ptr = strtok(NULL," \r\n");
    if(!ptr)
        return 0;
    timeinfo.tm_year = atoi(ptr);
    // process y2k
    if(timeinfo.tm_year < 100)
        timeinfo.tm_year += 2000;
    timeinfo.tm_year -= 1900;

    // time
    ptr = strtok(NULL," \r\n");
    if(!ptr)
        return 0;
    if(strlen(ptr) != 8)
        return 0;

    if(ptr[2] == ':' && ptr[5] == ':'){
        digit[2] = 0;
        // hour
        digit[0] = ptr[0];
        digit[1] = ptr[1];
        timeinfo.tm_hour = atoi(digit);
        // min
        digit[0] = ptr[3];
        digit[1] = ptr[4];
        timeinfo.tm_min = atoi(digit);
        // sec
        digit[0] = ptr[6];
        digit[1] = ptr[7];
        timeinfo.tm_sec = atoi(digit);
    }else
        return 0;

    timeinfo.tm_isdst = 0;
    time = (unsigned long)mktime ( &timeinfo );

    // time zone
    ptr = strtok(NULL," \r\n");
    if(!ptr)
        return 0;
    for(i=0;i<sizeof(TZ);i++){
        if(!strcmp(ptr,TZ[i].zone)){
            time += (TZ[i].diff + 9) * 3600; // assume KST
            break;
        }
    }
    if(i == sizeof(TZ))
        return 0;

    return time;
}

void Timer0IntHandler( void )
{
    timeval++;
	TimerIntClear( TIMER0_BASE, TIMER_TIMA_TIMEOUT );
}
void Timer1IntHandler( void )
{
    ulHighFrequencyTimerTicks++;
	TimerIntClear( TIMER1_BASE, TIMER_TIMA_TIMEOUT );
}


