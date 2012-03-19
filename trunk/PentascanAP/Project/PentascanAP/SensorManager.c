/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "Timers.h"

/* Hardware library includes. */
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "sysctl.h"
#include "gpio.h"
#include "grlib.h"
#include "uart.h"
#include "lmi_flash.h"

/* Demo app includes. */
#include "partest.h"

/* lwip library */
#include "lwiplib.h"

/* http client header */
#include "httpc.h"
#include "Rtc.h"
#include "lcd_terminal.h"

// periodic timer
static xTimerHandle xPeriodicTimer = NULL;
static xSemaphoreHandle xSemaphoreTimer = NULL;

static void StartNetwork(void)
{
    /* Create the lwIP task if running on a processor that includes a MAC and
    PHY. */
    if( SysCtlPeripheralPresent( SYSCTL_PERIPH_ETH ) )
    {
        unsigned long ulUser0, ulUser1;
        unsigned char pucMACArray[8];
    
        //
        // Enable and Reset the Ethernet Controller.
        //
        SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
        SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);
    
        //
        // Enable Port F for Ethernet LEDs.
        //  LED0        Bit 3   Output
        //  LED1        Bit 2   Output
        //
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
        GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    
        //
        // Configure the hardware MAC address for Ethernet Controller
        // filtering of incoming packets.
        //
        // For the LM3S6965 Evaluation Kit, the MAC address will be stored in the
        // non-volatile USER0 and USER1 registers.  These registers can be read
        // using the FlashUserGet function, as illustrated below.
        //
        FlashUserGet(&ulUser0, &ulUser1);
    
        //
        // Convert the 24/24 split MAC address from NV ram into a 32/16 split
        // MAC address needed to program the hardware registers, then program
        // the MAC address into the Ethernet Controller registers.
        //
        pucMACArray[0] = ((ulUser0 >>  0) & 0xff);
        pucMACArray[1] = ((ulUser0 >>  8) & 0xff);
        pucMACArray[2] = ((ulUser0 >> 16) & 0xff);
        pucMACArray[3] = ((ulUser1 >>  0) & 0xff);
        pucMACArray[4] = ((ulUser1 >>  8) & 0xff);
        pucMACArray[5] = ((ulUser1 >> 16) & 0xff);
    
        //
        // Initialze the lwIP library, using DHCP.
        //
        lwIPInit(pucMACArray, 0, 0, 0, IPADDR_USE_DHCP);
    }
}

unsigned long checkFreeMem(void)
{
    unsigned long memsize = 0;
    char *ptr = NULL;

	vTaskSuspendAll();
    do{
        memsize+=1;
        ptr = malloc(memsize);
        if(ptr)
            free(ptr);
    }while(ptr);
	xTaskResumeAll();
    
    return memsize;
}


#if configUSE_TIMERS
static void TimerCallback( xTimerHandle pxExpiredTimer )
{
	xSemaphoreGive(xSemaphoreTimer);
}
static void http_test(void)
{
	time_t timer;
	static int count;
	unsigned long tick;

//	  printf("[%d]http ^9%d`\n",count++,http_get("192.168.100.20",80,"/ap.html"));
	tick = xTaskGetTickCount();
	printf(">>");
	printf("[%d]naver ^9%d ^f%ld`\n",count,http_get("www.naver.com",80,"/",NULL,NULL),xTaskGetTickCount() - tick);
	tick = xTaskGetTickCount();
//	  printf("[%d]google ^9%d ^f%ld`\n",count++,http_get("www.google.com",80,"/",NULL,NULL),xTaskGetTickCount() - tick);
	printf("freemem = %ld\n",checkFreeMem());
	printf("stack = ^b%d`\n",uxTaskGetStackHighWaterMark(NULL));
	timer=RtcGetTime();
	printf("^f%s`",asctime(localtime(&timer)) + 11);

}
#else
static void httpTimerCallback(void *pv)
{
    time_t timer;
    static int count;
    printf("[%d]naver ^9%d\n",count,http_get("www.naver.com",80,"/",NULL,NULL));
//    printf("[%d]google ^9%d\n",count++,http_get("www.google.com",80,"/",NULL,NULL));
    printf("freemem = %ld\n",checkFreeMem());
    timer=RtcGetTime();
    printf("^f%s`",asctime(localtime(&timer)) + 11);

    sys_timeout(2000, httpTimerCallback, NULL);
}
#endif

int report_measure(){
    char *rpt;
    char node_string[100];
    time_t timer;
    int i, ret;
    static int count, success;
    

	struct {
		unsigned char addr;
		int temp;
		int humidity;
		int co2;
		int retry;
		int sound;
	}sensor[10] = {
		{0xA0, 19 * 100, 41 * 100, 3600, 0, 0},
		{0xA1, 20 * 100, 42 * 100, 3600, 0, 0},
		{0xA2, 21 * 100, 43 * 100, 3600, 0, 0},
		{0xA3, 22 * 100, 44 * 100, 3600, 0, 0},
		{0xA4, 23 * 100, 45 * 100, 3600, 0, 0},
		{0xA5, 24 * 100, 46 * 100, 3600, 0, 0},
		{0xA6, 25 * 100, 47 * 100, 3600, 0, 0},
		{0xA7, 26 * 100, 48 * 100, 3600, 0, 0},
		{0xA8, 27 * 100, 49 * 100, 3600, 0, 0},
		{0xA9, 28 * 100, 50 * 100, 3600, 0, 0}
	};
			
	
	
    rpt = mem_malloc(1024);
    sprintf(rpt,"/sensor/logging?id=%lu",RtcGetTime());
    for(i=0;i<10;i++){
        sprintf(node_string,"||AABBCCDDEEFF00%02x|%d.%02d,%d.%02d,%d,%d,%d", sensor[i].addr,
				sensor[i].temp/100,sensor[i].temp%100,
				sensor[i].humidity/100,sensor[i].humidity%100,
				sensor[i].co2,sensor[i].retry,sensor[i].sound);
        strcat(rpt,node_string);
    }
    count++;
    if( (ret = http_get("pentascan.dyndns.org",2222,rpt,NULL,NULL)) == 200)
        success++;
    mem_free(rpt);

    printf("^a<%d/%d>`result ^f[%d]`\n",success,count,ret);
    timer=RtcGetTime();
    printf("^f%s`",asctime(localtime(&timer)) + 11);
    return 0;
}


void vMainTask( void *pvParameters )
{
    char c;
    char *temp = mem_malloc(200);

	// Semaphore cannot be used before a call to xSemaphoreCreateCounting().
	// The max value to which the semaphore can count should be 10, and the
	// initial value assigned to the count should be 0.
	xSemaphoreTimer = xSemaphoreCreateCounting( 10, 0 );

	if( xSemaphoreTimer == NULL )
	{
		printf("#### Fail to create semaphore\n");
		return;
	}

    StartNetwork();

#if configUSE_TIMERS
	xPeriodicTimer = xTimerCreate(	( const signed char * ) "http timer",/* Text name to facilitate debugging.  The kernel does not use this itself. */
									( 60 * configTICK_RATE_HZ ),			/* The period for the timer. */
									pdTRUE,								/* Don't auto-reload - hence a one shot timer. */
									( void * ) 0,							/* The timer identifier.  In this case this is not used as the timer has its own callback. */
									TimerCallback );				/* The callback to be called when the timer expires. */
#endif									


    printf("Pentascan AP\n");
    xTimerStart(xPeriodicTimer,0);
    
	for( ;; )
	{
		/* wait 100msec */
		if(xSemaphoreTake( xSemaphoreTimer, configTICK_RATE_HZ / 10 ) == pdTRUE){
			report_measure();
		}

		if(UARTCharsAvail(UART0_BASE)){
		    c = getchar();
		    putchar(c);

		    switch(c)
		    {
		    case 'g':
	            printf("google ^9%d`\n",http_get("www.google.com",80,"/",NULL,NULL));
	            break;
	        case 'n':
	            printf("naver ^9%d`\n",http_get("www.naver.com",80,"/",NULL,NULL));
	            break;
		    case 'G':
	            printf("google ip ^9%d`\n",http_get("173.194.72.105",80,"/",NULL,NULL));
	            break;
		    case 'a':
	            printf("http %d\n",http_req("http://192.168.100.20/ap.html",NULL,NULL));
	            break;
	        case 'p':
    	        report_measure();
    	        break;
	        case 'f':
	            printf("Freemem = %ld\n",checkFreeMem());
	            break;
	        case 't':
	        {
	            unsigned long timer;
	            timer=RtcGetTime();
	            printf("^f%s`",asctime(localtime(&timer)));
	            break;
	        }
	        case 's':
	            vTaskList(temp);
				UARTprint( "name\t\tstatus\tpri\tstack\ttcb");
	            UARTprint(temp);
	            break;
	        case 'r':
#if configUSE_TIMERS
	            xTimerStart(xPeriodicTimer,0);
#else
	            sys_timeout(2000, httpTimerCallback, NULL);
#endif
	            break;
	        case 'x':
#if configUSE_TIMERS
	            xTimerStop(xPeriodicTimer,0);
#else
	            sys_untimeout(httpTimerCallback, NULL);
#endif
	            break;
	        case 'm':
	            show_alloctable();
	        }
		}
	}
}

