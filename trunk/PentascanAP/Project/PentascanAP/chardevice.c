/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Hardware library includes. */
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "sysctl.h"
#include "gpio.h"
#include "uart.h"

typedef struct {
    xQueueHandle TxQueue;
    unsigned long TxQueueLength;  
    xQueueHandle RxQueue;
    unsigned long RxQueueLength;  
    portTickType QueueWait;
    unsigned long PortBase; /* only for uart, otherwise set to zero */
}char_device;

static int prepare_device(char_device *dev)
{
    if(dev->TxQueueLength)
        dev->TxQueue = xQueueCreate( dev->TxQueueLength, sizeof(char) );
    if(dev->RxQueueLength)
        dev->RxQueue = xQueueCreate( dev->RxQueueLength, sizeof(char) );

    if(dev->TxQueueLength && !dev->TxQueue)
        return -1;
    if(dev->RxQueueLength && !dev->RxQueue)        
        return -1;

    return 0;
}

static int _getchar(char_device *dev)
{
    char ch;
    if(xQueueReceive(dev->RxQueue, &ch, dev->QueueWait))
        return ch;
    else
        return -1;
}

static void _putchar(char_device *dev,char ch)
{
    unsigned char ucData;
    xQueueSend(dev->TxQueue, &ch, dev->QueueWait);
    if(UARTSpaceAvail(dev->PortBase)){
        xQueueReceive(dev->TxQueue,&ucData,dev->QueueWait);
        UARTCharPut(dev->PortBase, ucData);
    }
}

static void charIntHandler(char_device *dev)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    unsigned long ulStatus;
    unsigned char ucData;

    //
    // Get the interrrupt status.
    //
    ulStatus = UARTIntStatus(dev->PortBase, true);

    //
    // Clear the asserted interrupts.
    //
    UARTIntClear(dev->PortBase, ulStatus);

    if(ulStatus & UART_INT_RX){
        ucData =UARTCharGet(dev->PortBase);
        xQueueSendFromISR(dev->RxQueue, &ucData, &xHigherPriorityTaskWoken);
    }

    if(ulStatus & UART_INT_TX){
        if(xQueueReceiveFromISR(dev->TxQueue, &ucData, &xHigherPriorityTaskWoken))
            UARTCharPut(dev->PortBase, ucData);
    }
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}


//////////////////////////////////////////////
//  UART2 : zigbee comm port
//////////////////////////////////////////////
char_device charZigbee;

static void zigbee_isr(void)
{
    charIntHandler(&charZigbee);
}

int zigbee_init(unsigned long baud)
{
    int result;

    charZigbee.TxQueueLength = 10;
    charZigbee.RxQueueLength = 20;
    charZigbee.QueueWait = 0;
    charZigbee.PortBase = UART2_BASE;

    result = prepare_device(&charZigbee);
    if(result)
        return result;
        
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    GPIOPinTypeUART(GPIO_PORTG_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART2_BASE, SysCtlClockGet(), baud,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    IntRegister(INT_UART2, zigbee_isr);
    
    UARTIntDisable(UART2_BASE, 0xFFFFFFFF);
    IntPrioritySet(INT_UART2,configKERNEL_INTERRUPT_PRIORITY);
    IntEnable(INT_UART2);
    UARTEnable(UART2_BASE);
    UARTFIFODisable(UART2_BASE);
    UARTIntEnable(UART2_BASE, UART_INT_TX | UART_INT_RX);
    
    return result;
}

int zigbee_getchar(void)
{
    return _getchar(&charZigbee);
}

void zigbee_putchar(char ch)
{
    _putchar(&charZigbee, ch);
}

//////////////////////////////////////////////
//  UART0 : debug port - map to stdin/stdout
//////////////////////////////////////////////
char_device charConsole;

static void console_isr(void)
{
    charIntHandler(&charConsole);
}

int console_init(unsigned long baud)
{
    int result;

    charConsole.TxQueueLength = 10;
    charConsole.RxQueueLength = 80;
    charConsole.QueueWait = 100 * portTICK_RATE_MS;
    charConsole.PortBase = UART0_BASE;

    result = prepare_device(&charConsole);
    if(result)
        return result;
        
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), baud,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    IntRegister(INT_UART0, console_isr);
    
    UARTIntDisable(UART0_BASE, 0xFFFFFFFF);
    IntPrioritySet(INT_UART0,configKERNEL_INTERRUPT_PRIORITY);
    IntEnable(INT_UART0);
    UARTEnable(UART0_BASE);
    UARTFIFODisable(UART0_BASE);
    UARTIntEnable(UART0_BASE, UART_INT_TX | UART_INT_RX);
    
    return result;
}

int console_getchar(void)
{
    return _getchar(&charConsole);
}

int console_putchar(char ch)
{
    _putchar(&charConsole, ch);
    return ch;
}

