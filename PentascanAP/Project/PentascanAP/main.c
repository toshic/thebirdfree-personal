/*
    FreeRTOS V7.1.0 - Copyright (C) 2011 Real Time Engineers Ltd.
	

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the standard demo application tasks.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Fast Interrupt Test" - A high frequency periodic interrupt is generated
 * using a free running timer to demonstrate the use of the
 * configKERNEL_INTERRUPT_PRIORITY configuration constant.  The interrupt
 * service routine measures the number of processor clocks that occur between
 * each interrupt - and in so doing measures the jitter in the interrupt timing.
 * The maximum measured jitter time is latched in the ulMaxJitter variable, and
 * displayed on the OLED display by the 'OLED' task as described below.  The
 * fast interrupt is configured and handled in the timertest.c source file.
 *
 * "OLED" task - the OLED task is a 'gatekeeper' task.  It is the only task that
 * is permitted to access the display directly.  Other tasks wishing to write a
 * message to the OLED send the message on a queue to the OLED task instead of
 * accessing the OLED themselves.  The OLED task just blocks on the queue waiting
 * for messages - waking and displaying the messages as they arrive.
 *
 * "Check" hook -  This only executes every five seconds from the tick hook.
 * Its main function is to check that all the standard demo tasks are still
 * operational.  Should any unexpected behaviour within a demo task be discovered
 * the tick hook will write an error to the OLED (via the OLED task).  If all the
 * demo tasks are executing with their expected behaviour then the check task
 * writes PASS to the OLED (again via the OLED task), as described above.
 *
 * "uIP" task -  This is the task that handles the uIP stack.  All TCP/IP
 * processing is performed in this task.
 */




/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/

/* Set the following option to 1 to include the WEB server in the build.  By
default the WEB server is excluded to keep the compiled code size under the 32K
limit imposed by the KickStart version of the IAR compiler.  The graphics
libraries take up a lot of ROM space, hence including the graphics libraries
and the TCP/IP stack together cannot be accommodated with the 32K size limit. */
#define mainINCLUDE_WEB_SERVER		1


/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Hardware library includes. */
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "sysctl.h"
#include "gpio.h"
#include "grlib.h"
#include "rit128x96x4.h"
#include "uart.h"
#include "lmi_flash.h"

/* Demo app includes. */
#include "partest.h"
#include "lcd_message.h"
#include "bitmap.h"

/* lwip library */
#include "lwiplib.h"

/* http client header */
#include "httpc.h"

/*-----------------------------------------------------------*/

/* The time between cycles of the 'check' functionality (defined within the
tick hook. */
#define mainCHECK_DELAY						( ( portTickType ) 1000 / portTICK_RATE_MS )

/* Size of the stack allocated to the uIP task. */
#define mainBASIC_WEB_STACK_SIZE            ( configMINIMAL_STACK_SIZE * 3 )

/* The OLED task uses the sprintf function so requires a little more stack too. */
#define mainOLED_TASK_STACK_SIZE			( configMINIMAL_STACK_SIZE + 50 )

/* Task priorities. */
#define mainQUEUE_POLL_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY )
#define mainGEN_QUEUE_TASK_PRIORITY			( tskIDLE_PRIORITY )

/* The maximum number of message that can be waiting for display at any one
time. */
#define mainOLED_QUEUE_SIZE					( 3 )

/* Dimensions the buffer into which the jitter time is written. */
#define mainMAX_MSG_LEN						25

/* The period of the system clock in nano seconds.  This is used to calculate
the jitter time in nano seconds. */
#define mainNS_PER_CLOCK					( ( unsigned portLONG ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )

/* Constants used when writing strings to the display. */
#define mainCHARACTER_HEIGHT				( 9 )
#define mainMAX_ROWS_128					( mainCHARACTER_HEIGHT * 14 )
#define mainMAX_ROWS_96						( mainCHARACTER_HEIGHT * 10 )
#define mainMAX_ROWS_64						( mainCHARACTER_HEIGHT * 7 )
#define mainFULL_SCALE						( 15 )
#define ulSSI_FREQUENCY						( 3500000UL )

/*-----------------------------------------------------------*/

/*
 * The task that handles the uIP stack.  All TCP/IP processing is performed in
 * this task.
 */
extern void vuIP_Task( void *pvParameters );

/*
 * The display is written two by more than one task so is controlled by a
 * 'gatekeeper' task.  This is the only task that is actually permitted to
 * access the display directly.  Other tasks wanting to display a message send
 * the message to the gatekeeper.
 */
static void vOLEDTask( void *pvParameters );

static void vUartTask( void *pvParameters );

/*
 * Configure the hardware for the demo.
 */
static void prvSetupHardware( void );

/*
 * Configures the high frequency timers - those used to measure the timing
 * jitter while the real time kernel is executing.
 */
extern void vSetupHighFrequencyTimer( void );

/*
 * Hook functions that can get called by the kernel.
 */
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );
void vApplicationTickHook( void );


/*-----------------------------------------------------------*/

/* The queue used to send messages to the OLED task. */
xQueueHandle xOLEDQueue;

/* The welcome text. */
const portCHAR * const pcWelcomeMessage = ">> Pentascan AP Test";

/*-----------------------------------------------------------*/


/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/
int main( void )
{
	prvSetupHardware();
    init_serial();

#if 0
    /* url parse test */
    http_req("www.naver.com");
    http_req("www.naver.com/");
    http_req("www.naver.com/index.html");
    http_req("www.naver.com/pub/index.html");
    http_req("www.naver.com:8080");
    http_req("www.naver.com:8080/");
    http_req("www.naver.com:8080/index.html");
    http_req("www.naver.com:8080/pub/index.html");

    http_req("http://www.naver.com");
    http_req("http://www.naver.com/");
    http_req("http://www.naver.com/index.html");
    http_req("http://www.naver.com/pub/index.html");
    http_req("http://www.naver.com:8080");
    http_req("http://www.naver.com:8080/");
    http_req("http://www.naver.com:8080/index.html");
    http_req("http://www.naver.com:8080/pub/index.html");

    http_req("https://www.naver.com");
    http_req("https://www.naver.com/");
    http_req("https://www.naver.com/index.html");
    http_req("https://www.naver.com/pub/index.html");
    http_req("https://www.naver.com:8080");
    http_req("https://www.naver.com:8080/");
    http_req("https://www.naver.com:8080/index.html");
    http_req("https://www.naver.com:8080/pub/index.html");

    http_req("ftp://www.naver.com");
    http_req("ftp://www.naver.com/");
    http_req("ftp://www.naver.com/index.html");
    http_req("ftp://www.naver.com/pub/index.html");
    http_req("ftp://www.naver.com:8080");
    http_req("ftp://www.naver.com:8080/");
    http_req("ftp://www.naver.com:8080/index.html");
    http_req("ftp://www.naver.com:8080/pub/index.html");
#endif    

	/* Create the queue used by the OLED task.  Messages for display on the OLED
	are received via this queue. */
	xOLEDQueue = xQueueCreate( mainOLED_QUEUE_SIZE, sizeof( xOLEDMessage ) );

	/* Exclude some tasks if using the kickstart version to ensure we stay within
	the 32K code size limit. */
	#if mainINCLUDE_WEB_SERVER != 0
	{
		/* Create the uIP task if running on a processor that includes a MAC and
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

		    printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",pucMACArray[0],pucMACArray[1],pucMACArray[2],pucMACArray[3],pucMACArray[4],pucMACArray[5]);
		
		    //
		    // Initialze the lwIP library, using DHCP.
		    //
		    lwIPInit(pucMACArray, 0, 0, 0, IPADDR_USE_DHCP);
		}
	}
	#endif

	
	/* Start the tasks defined within this file/specific to this demo. */
	xTaskCreate( vOLEDTask, ( signed portCHAR * ) "OLED", mainOLED_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );

    /* uart loopback task */	
	xTaskCreate( vUartTask, ( signed portCHAR * ) "UART", configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY, NULL );

	/* Configure the high frequency interrupt used to measure the interrupt
	jitter time. */
	vSetupHighFrequencyTimer();

	/* Start the scheduler. */
	vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
    task. */
	return 0;
}
/*-----------------------------------------------------------*/

void prvSetupHardware( void )
{
    /* If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    a workaround to allow the PLL to operate reliably. */
    if( DEVICE_IS_REVA2 )
    {
        SysCtlLDOSet( SYSCTL_LDO_2_75V );
    }
	
	/* Set the clocking to run from the PLL at 50 MHz */
	SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );
	
	/* 	Enable Port F for Ethernet LEDs
		LED0        Bit 3   Output
		LED1        Bit 2   Output */
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF );
	GPIODirModeSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3), GPIO_DIR_MODE_HW );
	GPIOPadConfigSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3 ), GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD );	
	
	vParTestInitialise();
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    static xOLEDMessage xMessage = { "PASS" };
    static unsigned portLONG ulTicksSinceLastDisplay = 0;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* Called from every tick interrupt.  Have enough ticks passed to make it
	time to perform our health status check again? */
	ulTicksSinceLastDisplay++;
	if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
	{
		ulTicksSinceLastDisplay = 0;
		
		/* Send the message to the OLED gatekeeper for display. */
		xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR( xOLEDQueue, &xMessage, &xHigherPriorityTaskWoken );
	}
}
/*-----------------------------------------------------------*/

void vUartTask( void *pvParameters )
{
    char c;
    printf("Enter Text:");


	for( ;; )
	{
	    c = getchar();
	    putchar(c);
	    if(c == 'g'){
            http_req("www.naver.com");
	    }else if(c == 'G'){
            http_req("202.131.30.11");
	    }
	}
}


void vOLEDTask( void *pvParameters )
{
xOLEDMessage xMessage;
unsigned portLONG ulY, ulMaxY;
static portCHAR cMessage[ mainMAX_MSG_LEN ];
extern volatile unsigned portLONG ulMaxJitter;
extern volatile unsigned portLONG ulMaxDifference;
unsigned portBASE_TYPE uxUnusedStackOnEntry;
const unsigned portCHAR *pucImage;

/* Functions to access the OLED.  The one used depends on the dev kit
being used. */
void ( *vOLEDInit )( unsigned portLONG ) = NULL;
void ( *vOLEDStringDraw )( const portCHAR *, unsigned portLONG, unsigned portLONG, unsigned portCHAR ) = NULL;
void ( *vOLEDImageDraw )( const unsigned portCHAR *, unsigned portLONG, unsigned portLONG, unsigned portLONG, unsigned portLONG ) = NULL;
void ( *vOLEDClear )( void ) = NULL;

	/* Just for demo purposes. */
	uxUnusedStackOnEntry = uxTaskGetStackHighWaterMark( NULL );

	vOLEDInit = RIT128x96x4Init;
	vOLEDStringDraw = RIT128x96x4StringDraw;
	vOLEDImageDraw = RIT128x96x4ImageDraw;
	vOLEDClear = RIT128x96x4Clear;
	ulMaxY = mainMAX_ROWS_96;
	pucImage = pucBasicBitmap;
	ulY = ulMaxY;
	
	/* Initialise the OLED and display a startup message. */
	vOLEDInit( ulSSI_FREQUENCY );	
	vOLEDStringDraw( "POWERED BY FreeRTOS", 0, 0, mainFULL_SCALE );
	vOLEDImageDraw( pucImage, 0, mainCHARACTER_HEIGHT + 1, bmpBITMAP_WIDTH, bmpBITMAP_HEIGHT );
	
	for( ;; )
	{
		/* Wait for a message to arrive that requires displaying. */
		xQueueReceive( xOLEDQueue, &xMessage, portMAX_DELAY );
	
		/* Write the message on the next available row. */
		ulY += mainCHARACTER_HEIGHT;
		if( ulY >= ulMaxY )
		{
			ulY = mainCHARACTER_HEIGHT;
			vOLEDClear();
			vOLEDStringDraw( pcWelcomeMessage, 0, 0, mainFULL_SCALE );			
		}

		/* Display the message along with the maximum jitter time from the
		high priority time test. */
		sprintf( cMessage, "%s [%uns]", xMessage.pcMessage, ulMaxJitter * mainNS_PER_CLOCK);
		ulMaxDifference = 0;
		vOLEDStringDraw( cMessage, 0, ulY, mainFULL_SCALE );
	}
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	( void ) pxTask;
	( void ) pcTaskName;

    printf("Stack Overflow : %s\n",pcTaskName);
    
	for( ;; );
}

#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
    printf("ASSERT - %s:%ld\n",pcFilename,ulLine);
}
#endif

