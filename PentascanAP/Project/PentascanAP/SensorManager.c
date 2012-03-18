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


static void httpTimerCallback( xTimerHandle pxExpiredTimer )
{
    time_t timer;
    static int count;
    unsigned long tick;

//    printf("[%d]http ^9%d`\n",count++,http_get("192.168.100.20",80,"/ap.html"));
    tick = xTaskGetTickCount();
    printf("[%d]naver ^9%d ^f%ld`\n",count,http_get("www.naver.com",80,"/",NULL,NULL),xTaskGetTickCount() - tick);
    tick = xTaskGetTickCount();
//    printf("[%d]google ^9%d ^f%ld`\n",count++,http_get("www.google.com",80,"/",NULL,NULL),xTaskGetTickCount() - tick);
    printf("freemem = %ld\n",checkFreeMem());
    printf("stack = ^b%d`\n",uxTaskGetStackHighWaterMark(NULL));
    timer=RtcGetTime();
    printf("^f%s`",asctime(localtime(&timer)) + 11);
}

void vMainTask( void *pvParameters )
{
    char c;
    char *temp = mem_malloc(200);

    StartNetwork();

	xPeriodicTimer = xTimerCreate(	( const signed char * ) "http timer",/* Text name to facilitate debugging.  The kernel does not use this itself. */
									( 2 * configTICK_RATE_HZ ),			/* The period for the timer. */
									pdTRUE,								/* Don't auto-reload - hence a one shot timer. */
									( void * ) 0,							/* The timer identifier.  In this case this is not used as the timer has its own callback. */
									httpTimerCallback );				/* The callback to be called when the timer expires. */


    printf("Pentascan AP\n");
	for( ;; )
	{
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
            xTimerStart(xPeriodicTimer,0);
            break;
        case 'x':
            xTimerStop(xPeriodicTimer,0);
            break;
        case 'm':
            show_alloctable();
        }
	}
}


int report_measure(){
    char *rpt;
    char node_string[100];
    int i;
#if 0
    rpt = mem_malloc(1024);
    sprintf(rpt,"/sensor/logging?id=%lu",time);
    for(i=0;i<10;i++){
        sprintf(node_string,"||%02x|%d.%02d,%d.%02d,%d,%d,%d",addr,temp/100,temp%100,humidity/100,humidity%100,co2,retry,sound);
        strcat(rpt,node_string);
    }
    http_get("pentascan.dyndns.org",2222,rpt);
#endif    
    return 0;
}

