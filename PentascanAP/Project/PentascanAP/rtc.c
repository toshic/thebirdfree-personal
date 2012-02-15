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

static volatile unsigned long timeval;

void RtcInit(void)
{
	SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER0 );
    TimerConfigure( TIMER0_BASE, TIMER_CFG_32_BIT_PER );
	
	/* Set the timer interrupt to be above the kernel - highest. */
	IntPrioritySet( INT_TIMER0A, 0 );

	/* Ensure interrupts do not start until the scheduler is running. */
	portDISABLE_INTERRUPTS();
	
	/* The rate at which the timer will interrupt. */
    TimerLoadSet( TIMER0_BASE, TIMER_A, configCPU_CLOCK_HZ );
    IntEnable( INT_TIMER0A );
    TimerIntEnable( TIMER0_BASE, TIMER_TIMA_TIMEOUT );

	/* Enable both timers. */	
    TimerEnable( TIMER0_BASE, TIMER_A );
}

unsigned long RtcGet(void)
{
    return timeval;
}

void RtcSet(unsigned long new_time)
{
    TimerDisable( TIMER0_BASE, TIMER_A );
    IntDisable( INT_TIMER0A );
    timeval = new_time;
    IntEnable( INT_TIMER0A );
    TimerEnable( TIMER0_BASE, TIMER_A );
}

void Timer0IntHandler( void )
{
    timeval++;
	TimerIntClear( TIMER0_BASE, TIMER_TIMA_TIMEOUT );
}


