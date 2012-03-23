/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Hardware library includes. */
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "sysctl.h"
#include "lmi_timer.h"
#include "gpio.h"
/* Demo app includes. */
#include "partest.h"

#include "Rtc.h"
#include "lcd_terminal.h"

/*-----------------------------------------------------------*/

extern void init_serial (void);


/* Counts the total number of times that the high frequency timer has 'ticked'.
This value is used by the run time stats function to work out what percentage
of CPU time each task is taking. */
volatile unsigned long ulHighFrequencyTimerTicks = 0UL;
/*-----------------------------------------------------------*/

void vSetupHighFrequencyTimer( void )
{
	/* Timer zero is used to generate the interrupts, and timer 1 is used
	to measure the jitter. */
    SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER1 );
    TimerConfigure( TIMER1_BASE, TIMER_CFG_32_BIT_PER );

	/* Just used to measure time. */
    TimerLoadSet(TIMER1_BASE, TIMER_A, 0 );
    TimerEnable( TIMER1_BASE, TIMER_A );
}

unsigned long vGetHighFrequencyTimerTicks( void )
{
	return TimerLoadGet(TIMER1_BASE, TIMER_A);
}

/*
 * Configure the hardware for the demo.
 */
static void prvSetupHardware( void )
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
	RtcInit();
// use lcd	
//	lcd_terminal_init();
    init_serial();
}

/*
 * Hook functions that can get called by the kernel.
 */
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName );

extern void vMainTask( void *pvParameters );


/*-----------------------------------------------------------*/

/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/
int main( void )
{
    int i;
	prvSetupHardware();

    /* Main task */	
	xTaskCreate( vMainTask, ( signed portCHAR * ) "Main", 256, NULL, tskIDLE_PRIORITY + 1, NULL );

	/* Start the scheduler. */
	vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
    task. */
	return 0;
}
/*-----------------------------------------------------------*/

extern void UARTprint(char *message);

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	( void ) pxTask;
	( void ) pcTaskName;

    
    UARTprint("Stack Overflow : ");
    UARTprint(pcTaskName);
    
	for( ;; );
}

#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
    printf("ASSERT - %s:%ld\n",pcFilename,ulLine);
}
#endif

