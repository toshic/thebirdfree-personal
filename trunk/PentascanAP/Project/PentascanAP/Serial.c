/*----------------------------------------------------------------------------
 *      RL-ARM - FlashFS
 *----------------------------------------------------------------------------
 *      Name:    SERIAL.C
 *      Purpose: Serial Input Output for Luminary LM3S6965
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2011 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <stdio.h>

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

#define USE_UART_INTERRUPTx

xSemaphoreHandle xUartTxInterruptSemaphore = NULL;
xSemaphoreHandle xUartRxInterruptSemaphore = NULL;


/*----------------------------------------------------------------------------
 *       init_serial:  Initialize Serial Interface
 *---------------------------------------------------------------------------*/
void init_serial (void) {
  /* Initialize the serial interface */

    vSemaphoreCreateBinary( xUartTxInterruptSemaphore );
    xSemaphoreTake( xUartTxInterruptSemaphore, 0 );
    vSemaphoreCreateBinary( xUartRxInterruptSemaphore );
    xSemaphoreTake( xUartRxInterruptSemaphore, 0 );

    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Set GPIO A0 and A1 as UART pins.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));


    //
    // Set the UART to interrupt whenever the TX FIFO is almost empty or
    // when any character is received.
    //
    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    //
    // Enable the UART.
    //
    UARTIntDisable(UART0_BASE, 0xFFFFFFFF);
    IntEnable(INT_UART0);
    UARTEnable(UART0_BASE);
    IntPrioritySet(INT_UART0,3);
    printf("INT_UART0 %ld\n",IntPriorityGet(INT_UART0));

    printf("This is test message to check long message printf function\n");
}

/*----------------------------------------------------------------------------
 *       sendchar:  Write a character to Serial Port
 *---------------------------------------------------------------------------*/
int sendchar (int ch) {
#ifdef USE_UART_INTERRUPT
    if(!UARTSpaceAvail(UART0_BASE)){
        UARTIntEnable(UART0_BASE, UART_INT_TX);
        xSemaphoreTake( xUartTxInterruptSemaphore, portMAX_DELAY );
    }
#endif    
    UARTCharPut (UART0_BASE, ch);
    return (ch);
}

/*----------------------------------------------------------------------------
 *       getkey:  Read a character from Serial Port
 *---------------------------------------------------------------------------*/
int getkey (void) {
#ifdef USE_UART_INTERRUPT
    if(!UARTCharsAvail(UART0_BASE)){
        UARTIntEnable(UART0_BASE, UART_INT_RX);
        xSemaphoreTake( xUartRxInterruptSemaphore, portMAX_DELAY );
    }
#endif    
    return (UARTCharGet (UART0_BASE));
}

void
UARTIntHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    unsigned long ulStatus;

    //
    // Get the interrrupt status.
    //
    ulStatus = UARTIntStatus(UART0_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    UARTIntClear(UART0_BASE, ulStatus);

    if(ulStatus & UART_INT_TX){
        UARTIntDisable(UART0_BASE, UART_INT_TX);
        xSemaphoreGiveFromISR( xUartTxInterruptSemaphore, &xHigherPriorityTaskWoken );
    }
    
    if(ulStatus & UART_INT_RX){
        UARTIntDisable(UART0_BASE, UART_INT_RX);
        xSemaphoreGiveFromISR( xUartRxInterruptSemaphore, &xHigherPriorityTaskWoken );
    }
    
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
