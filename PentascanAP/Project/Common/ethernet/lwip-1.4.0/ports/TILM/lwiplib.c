//*****************************************************************************
//
// lwiplib.c - lwIP TCP/IP Library Abstraction Layer.
//
// Copyright (c) 2008-2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 8264 of the Stellaris Firmware Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Ensure that the lwIP Compile Time Options are included first.
//
//*****************************************************************************
#include "lwiplib.h"

//*****************************************************************************
//
//! \addtogroup lwiplib_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// The lwIP Library abstration layer provides for a host callback function
// to be called periodically in the lwIP context.  This is the timer
// interval, in ms, for this periodic callback.  If the timer interval is
// defined to 0 (the default value), then no periodic host callback is
// performed.
//
//*****************************************************************************
#define LWIP_USE_INTERRUPT

#ifndef HOST_TMR_INTERVAL
#define HOST_TMR_INTERVAL       0
#else
extern void lwIPHostTimerHandler(void);
#endif

//*****************************************************************************
//
// The number of milliseconds between calls to the soft-MDIX function when
// using a RTOS.
//
//*****************************************************************************
#define SOFT_MDIX_INTERVAL      10

//*****************************************************************************
//
// Driverlib headers needed for this library module.
//
//*****************************************************************************
#include "hw_ints.h"
#include "hw_ethernet.h"
#include "hw_memmap.h"
#include "hw_nvic.h"
#include "hw_sysctl.h"
#include "hw_types.h"
#include "debug.h"
#include "ethernet.h"
#include "gpio.h"
#include "sysctl.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "FreeRTOS.h"
#include "Timers.h"
#include "netif/stellarisif.h"
/* http client header */
#include "httpc.h"
#include "Rtc.h"
#include <time.h>

//*****************************************************************************
//
// The lwIP network interface structure for the Stellaris Ethernet MAC.
//
//*****************************************************************************
static struct netif g_sNetIF;

//*****************************************************************************
//
// The local time when the soft MDI/MDIX switch was last switched.
//
//*****************************************************************************
static unsigned long g_ulMDIXTimer = 0;

//*****************************************************************************
//
// The default IP address aquisition mode.
//
//*****************************************************************************
static unsigned long g_ulIPMode = IPADDR_USE_DHCP;

//*****************************************************************************
//
// The IP address to be used.  This is used during the initialization of the
// stack and when the interface configuration is changed.
//
//*****************************************************************************
static unsigned long g_ulIPAddr;

//*****************************************************************************
//
// The netmask to be used.  This is used during the initialization of the stack
// and when the interface configuration is changed.
//
//*****************************************************************************
static unsigned long g_ulNetMask;

//*****************************************************************************
//
// The gateway address to be used.  This is used during the initialization of
// the stack and when the interface configuration is changed.
//
//*****************************************************************************
static unsigned long g_ulGWAddr;

//*****************************************************************************
//
// The stack to used for the interrupt task.
//
//*****************************************************************************
static unsigned long g_pulStack[128];

//*****************************************************************************
//
// The handle for the "queue" (semaphore) used to signal the interrupt task
// from the interrupt handler.
//
//*****************************************************************************
static xQueueHandle g_pInterrupt;


// periodic timer
static xTimerHandle xPeriodicTimer = NULL;


static volatile unsigned long g_eth_int_status;

//*****************************************************************************
//
// This task handles reading packets from the Ethernet controller and supplying
// them to the TCP/IP thread.
//
//*****************************************************************************
static void
lwIPInterruptTask(void *pvArg)
{
    unsigned long phy_int_status;
    unsigned long phy_eth_int;
    //
    // Loop forever.
    //
    while(1)
    {
        //
        // Wait until the semaphore has been signalled.
        //
#ifdef LWIP_USE_INTERRUPT        
        while(xQueueReceive(g_pInterrupt, &pvArg, portMAX_DELAY) != pdPASS)
        {
        }
        phy_eth_int = (unsigned long)pvArg;
#else
        while(g_eth_int_status == 0)
        {
        }
        phy_eth_int = g_eth_int_status;
        g_eth_int_status = 0;
#endif
        if(phy_eth_int & ETH_INT_PHY){
            phy_int_status = EthernetPHYRead(ETH_BASE, PHY_MR17);
            printf("PHY_MR17 %04x\n",phy_int_status);
            
            if(phy_int_status & PHY_MR17_ANEGCOMP_INT){
                netif_set_link_up(&g_sNetIF);
                printf("<<< link up\n");
            }else{
                netif_set_link_down(&g_sNetIF);
                netif_set_down(&g_sNetIF);
                printf(">>> link down\n");
            }
        }
        //
        // Processes any packets waiting to be sent or received.
        //
        if(phy_eth_int & (ETH_INT_RX | ETH_INT_TX)){
            stellarisif_interrupt(&g_sNetIF);
        }
        //
        // Re-enable the Ethernet interrupts.
        //
        EthernetIntEnable(ETH_BASE, ETH_INT_PHY | ETH_INT_RX | ETH_INT_TX);
    }
}

//*****************************************************************************
//
// Handles the timeout for the soft-MDIX timer when using a RTOS.
//
//*****************************************************************************
static void
lwIPSoftMDIXTimer(void *pvArg)
{
    //
    // Service the MDIX timer.
    //
    if((EthernetPHYRead(ETH_BASE, PHY_MR1) & PHY_MR1_LINK) == 0)
    {
        g_ulMDIXTimer += SOFT_MDIX_INTERVAL;

        //
        // See if there has not been a link for 2 seconds.
        //
        if(g_ulMDIXTimer >= 2000)
        {
            //
            // There has not been a link for 2 seconds, so flip the MDI/MDIX
            // switch.  This is handled automatically by Fury rev A2, but is
            // harmless.
            //
            HWREG(ETH_BASE + MAC_O_MDIX) ^= MAC_MDIX_EN;

            //
            // Reset the MDIX timer.
            //
            g_ulMDIXTimer = 0;
        }
    }
    else
    {
        //
        // There is a link, so reset the MDIX timer.
        //
        g_ulMDIXTimer = 0;
    }

    //
    // Re-schedule the soft-MDIX timer callback function timeout.
    //
    sys_timeout(SOFT_MDIX_INTERVAL, lwIPSoftMDIXTimer, NULL);
}

//*****************************************************************************
//
// Handles the timeout for the host callback function timer when using a RTOS.
//
//*****************************************************************************
#if HOST_TMR_INTERVAL
static void
lwIPPrivateHostTimer(void *pvArg)
{
    //
    // Call the application-supplied host timer callback function.
    //
    lwIPHostTimerHandler();

    //
    // Re-schedule the host timer callback function timeout.
    //
    sys_timeout(HOST_TMR_INTERVAL, lwIPPrivateHostTimer, NULL);
}
#endif

static void lwIPStatus_CB(struct netif *netif)
{
    u8_t link_up = netif_is_link_up(netif);
    printf("lwIPStatus_CB %d\n",link_up);

    if(netif_is_link_up(netif)){
        xTimerStart(xPeriodicTimer,0);
    }else{
        xTimerStop(xPeriodicTimer,0);
    }
}

unsigned long checkFreeMem(void)
{
    unsigned long memsize = 0;
    char *ptr = NULL;

    do{
        memsize+=1024;
        ptr = pvPortMalloc(memsize);
        if(ptr)
            vPortFree(ptr);
    }while(ptr);
    
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
    printf("[%d]google ^9%d ^f%ld`\n",count++,http_get("www.google.com",80,"/",NULL,NULL),xTaskGetTickCount() - tick);
    printf("freemem = %ld\n",checkFreeMem());
    printf("stack = ^b%d`\n",uxTaskGetStackHighWaterMark(NULL));
    timer=RtcGetTime();
    printf("^f%s`",asctime(localtime(&timer)) + 11);
}

//*****************************************************************************
//
// Completes the initialization of lwIP.  This is directly called when not
// using a RTOS and provided as a callback to the TCP/IP thread when using a
// RTOS.
//
//*****************************************************************************
static void
lwIPPrivateInit(void *pvArg)
{
    struct ip_addr ip_addr;
    struct ip_addr net_mask;
    struct ip_addr gw_addr;

    //
    // If using a RTOS, create a queue (to be used as a semaphore) to signal
    // the Ethernet interrupt task from the Ethernet interrupt handler.
    //
#ifdef LWIP_USE_INTERRUPT        
    g_pInterrupt = xQueueCreate(1,sizeof(void *));
#else
    g_eth_int_status = 0;
#endif    

	xPeriodicTimer = xTimerCreate(	( const signed char * ) "http timer",/* Text name to facilitate debugging.  The kernel does not use this itself. */
									( 5 * configTICK_RATE_HZ ),			/* The period for the timer. */
									pdTRUE,								/* Don't auto-reload - hence a one shot timer. */
									( void * ) 0,							/* The timer identifier.  In this case this is not used as the timer has its own callback. */
									httpTimerCallback );				/* The callback to be called when the timer expires. */

    //
    // If using a RTOS, create the Ethernet interrupt task.
    //
    xTaskCreate(lwIPInterruptTask, (signed portCHAR *)"eth_int",
                256, 0, tskIDLE_PRIORITY + 3, 0);

    //
    // Setup the network address values.
    //
    if(g_ulIPMode == IPADDR_USE_STATIC)
    {
        ip_addr.addr = htonl(g_ulIPAddr);
        net_mask.addr = htonl(g_ulNetMask);
        gw_addr.addr = htonl(g_ulGWAddr);
    }
    else
    {
        ip_addr.addr = 0;
        net_mask.addr = 0;
        gw_addr.addr = 0;
    }

    //
    // Create, configure and add the Ethernet controller interface with
    // default settings.  ip_input should be used to send packets directly to
    // the stack when not using a RTOS and tcpip_input should be used to send
    // packets to the TCP/IP thread's queue when using a RTOS.
    //
    netif_add(&g_sNetIF, &ip_addr, &net_mask, &gw_addr, NULL, stellarisif_init,
              tcpip_input);
    netif_set_default(&g_sNetIF);

    netif_set_status_callback(&g_sNetIF,lwIPStatus_CB);

    //
    // Start DHCP, if enabled.
    //
#if LWIP_DHCP
    if(g_ulIPMode == IPADDR_USE_DHCP)
    {
        dhcp_start(&g_sNetIF);
    }
#endif

    //
    // Start AutoIP, if enabled and DHCP is not.
    //
#if LWIP_AUTOIP
    if(g_ulIPMode == IPADDR_USE_AUTOIP)
    {
        autoip_start(&g_sNetIF);
    }
#endif

    //
    // Bring the interface up.
    //
    if(g_ulIPMode == IPADDR_USE_STATIC)
        netif_set_up(&g_sNetIF);

    //
    // Setup a timeout for the host timer callback function if using a RTOS.
    //
#if HOST_TMR_INTERVAL
    sys_timeout(HOST_TMR_INTERVAL, lwIPPrivateHostTimer, NULL);
#endif

    //
    // If not running on a Fury-class device, then MDIX is handled in software.
    // In this case, when using a RTOS, setup a timeout for the soft-MDIX
    // handler.
    //
    if(!CLASS_IS_FURY)
    {
        sys_timeout(SOFT_MDIX_INTERVAL, lwIPSoftMDIXTimer, NULL);
    }
}

//*****************************************************************************
//
//! Initializes the lwIP TCP/IP stack.
//!
//! \param pucMAC is a pointer to a six byte array containing the MAC
//! address to be used for the interface.
//! \param ulIPAddr is the IP address to be used (static).
//! \param ulNetMask is the network mask to be used (static).
//! \param ulGWAddr is the Gateway address to be used (static).
//! \param ulIPMode is the IP Address Mode.  \b IPADDR_USE_STATIC will force
//! static IP addressing to be used, \b IPADDR_USE_DHCP will force DHCP with
//! fallback to Link Local (Auto IP), while \b IPADDR_USE_AUTOIP will force
//! Link Local only.
//!
//! This function performs initialization of the lwIP TCP/IP stack for the
//! Stellaris Ethernet MAC, including DHCP and/or AutoIP, as configured.
//!
//! \return None.
//
//*****************************************************************************
void
lwIPInit(const unsigned char *pucMAC, unsigned long ulIPAddr,
         unsigned long ulNetMask, unsigned long ulGWAddr,
         unsigned long ulIPMode)
{
    //
    // Check the parameters.
    //
#if LWIP_DHCP && LWIP_AUTOIP
    ASSERT((ulIPMode == IPADDR_USE_STATIC) ||
           (ulIPMode == IPADDR_USE_DHCP) ||
           (ulIPMode == IPADDR_USE_AUTOIP));
#elif LWIP_DHCP
    ASSERT((ulIPMode == IPADDR_USE_STATIC) ||
           (ulIPMode == IPADDR_USE_DHCP));
#elif LWIP_AUTOIP
    ASSERT((ulIPMode == IPADDR_USE_STATIC) ||
           (ulIPMode == IPADDR_USE_AUTOIP));
#else
    ASSERT(ulIPMode == IPADDR_USE_STATIC);
#endif

    //
    // Enable the ethernet peripheral.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);

    //
    // Program the MAC address into the Ethernet controller.
    //
    EthernetMACAddrSet(ETH_BASE, (unsigned char *)pucMAC);

    //
    // Save the network configuration for later use by the private
    // initialization.
    //
    g_ulIPMode = ulIPMode;
    g_ulIPAddr = ulIPAddr;
    g_ulNetMask = ulNetMask;
    g_ulGWAddr = ulGWAddr;

    //
    // Initialize lwIP.  The remainder of initialization is done immediately if
    // not using a RTOS and it is deferred to the TCP/IP thread's context if
    // using a RTOS.
    //
    tcpip_init(lwIPPrivateInit, 0);
}


//*****************************************************************************
//
//! Handles Ethernet interrupts for the lwIP TCP/IP stack.
//!
//! This function handles Ethernet interrupts for the lwIP TCP/IP stack.  At
//! the lowest level, all receive packets are placed into a packet queue for
//! processing at a higher level.  Also, the transmit packet queue is checked
//! and packets are drained and transmitted through the Ethernet MAC as needed.
//! If the system is configured without an RTOS, additional processing is
//! performed at the interrupt level.  The packet queues are processed by the
//! lwIP TCP/IP code, and lwIP periodic timers are serviced (as needed).
//!
//! \return None.
//
//*****************************************************************************
void
lwIPEthernetIntHandler(void)
{
    unsigned long ulStatus;
    portBASE_TYPE xWake;

    //
    // Read and Clear the interrupt.
    //
    ulStatus = EthernetIntStatus(ETH_BASE, false);
    EthernetIntClear(ETH_BASE, ulStatus);

    //
    // The handling of the interrupt is different based on the use of a RTOS.
    //
    //
    // A RTOS is being used.  Signal the Ethernet interrupt task.
    //
#ifdef LWIP_USE_INTERRUPT        
    xQueueSendFromISR(g_pInterrupt, (void *)&ulStatus, &xWake);
#else
    g_eth_int_status = ulStatus;
#endif

    //
    // Disable the Ethernet interrupts.  Since the interrupts have not been
    // handled, they are not asserted.  Once they are handled by the Ethernet
    // interrupt task, it will re-enable the interrupts.
    //
    EthernetIntDisable(ETH_BASE, ETH_INT_PHY | ETH_INT_RX | ETH_INT_TX);

    //
    // Potentially task switch as a result of the above queue write.
    //
    portEND_SWITCHING_ISR(xWake);
}

//*****************************************************************************
//
//! Returns the IP address for this interface.
//!
//! This function will read and return the currently assigned IP address for
//! the Stellaris Ethernet interface.
//!
//! \return Returns the assigned IP address for this interface.
//
//*****************************************************************************
unsigned long
lwIPLocalIPAddrGet(void)
{
    return((unsigned long)g_sNetIF.ip_addr.addr);
}

//*****************************************************************************
//
//! Returns the network mask for this interface.
//!
//! This function will read and return the currently assigned network mask for
//! the Stellaris Ethernet interface.
//!
//! \return the assigned network mask for this interface.
//
//*****************************************************************************
unsigned long
lwIPLocalNetMaskGet(void)
{
    return((unsigned long)g_sNetIF.netmask.addr);
}

//*****************************************************************************
//
//! Returns the gateway address for this interface.
//!
//! This function will read and return the currently assigned gateway address
//! for the Stellaris Ethernet interface.
//!
//! \return the assigned gateway address for this interface.
//
//*****************************************************************************
unsigned long
lwIPLocalGWAddrGet(void)
{
    return((unsigned long)g_sNetIF.gw.addr);
}

//*****************************************************************************
//
//! Returns the local MAC/HW address for this interface.
//!
//! \param pucMAC is a pointer to an array of bytes used to store the MAC
//! address.
//!
//! This function will read the currently assigned MAC address into the array
//! passed in \e pucMAC.
//!
//! \return None.
//
//*****************************************************************************
void
lwIPLocalMACGet(unsigned char *pucMAC)
{
    EthernetMACAddrGet(ETH_BASE, pucMAC);
}

//*****************************************************************************
//
// Completes the network configuration change.  This is directly called when
// not using a RTOS and provided as a callback to the TCP/IP thread when using
// a RTOS.
//
//*****************************************************************************
static void
lwIPPrivateNetworkConfigChange(void *pvArg)
{
    unsigned long ulIPMode;
    struct ip_addr ip_addr;
    struct ip_addr net_mask;
    struct ip_addr gw_addr;

    //
    // Get the new address mode.
    //
    ulIPMode = (unsigned long)pvArg;

    //
    // Setup the network address values.
    //
    if(ulIPMode == IPADDR_USE_STATIC)
    {
        ip_addr.addr = htonl(g_ulIPAddr);
        net_mask.addr = htonl(g_ulNetMask);
        gw_addr.addr = htonl(g_ulGWAddr);
    }
#if LWIP_DHCP || LWIP_AUTOIP
    else
    {
        ip_addr.addr = 0;
        net_mask.addr = 0;
        gw_addr.addr = 0;
    }
#endif

    //
    // Switch on the current IP Address Aquisition mode.
    //
    switch(g_ulIPMode)
    {
        //
        // Static IP
        //
        case IPADDR_USE_STATIC:
        {
            //
            // Set the new address parameters.  This will change the address
            // configuration in lwIP, and if necessary, will reset any links
            // that are active.  This is valid for all three modes.
            //
            netif_set_addr(&g_sNetIF, &ip_addr, &net_mask, &gw_addr);

            //
            // If we are going to DHCP mode, then start the DHCP server now.
            //
#if LWIP_DHCP
            if(ulIPMode == IPADDR_USE_DHCP)
            {
                dhcp_start(&g_sNetIF);
            }
#endif

            //
            // If we are going to AutoIP mode, then start the AutoIP process
            // now.
            //
#if LWIP_AUTOIP
            if(ulIPMode == IPADDR_USE_AUTOIP)
            {
                autoip_start(&g_sNetIF);
            }
#endif

            //
            // And we're done.
            //
            break;
        }

        //
        // DHCP (with AutoIP fallback).
        //
#if LWIP_DHCP
        case IPADDR_USE_DHCP:
        {
            //
            // If we are going to static IP addressing, then disable DHCP and
            // force the new static IP address.
            //
            if(ulIPMode == IPADDR_USE_STATIC)
            {
                dhcp_stop(&g_sNetIF);
                netif_set_addr(&g_sNetIF, &ip_addr, &net_mask, &gw_addr);
            }

            //
            // If we are going to AUTO IP addressing, then disable DHCP, set
            // the default addresses, and start AutoIP.
            //
#if LWIP_AUTOIP
            else if(ulIPMode == IPADDR_USE_AUTOIP)
            {
                dhcp_stop(&g_sNetIF);
                netif_set_addr(&g_sNetIF, &ip_addr, &net_mask, &gw_addr);
                autoip_start(&g_sNetIF);
            }
#endif
            break;
        }
#endif

        //
        // AUTOIP
        //
#if LWIP_AUTOIP
        case IPADDR_USE_AUTOIP:
        {
            //
            // If we are going to static IP addressing, then disable AutoIP and
            // force the new static IP address.
            //
            if(ulIPMode == IPADDR_USE_STATIC)
            {
                autoip_stop(&g_sNetIF);
                netif_set_addr(&g_sNetIF, &ip_addr, &net_mask, &gw_addr);
            }

            //
            // If we are going to DHCP addressing, then disable AutoIP, set the
            // default addresses, and start dhcp.
            //
#if LWIP_DHCP
            else if(ulIPMode == IPADDR_USE_DHCP)
            {
                autoip_stop(&g_sNetIF);
                netif_set_addr(&g_sNetIF, &ip_addr, &net_mask, &gw_addr);
                dhcp_start(&g_sNetIF);
            }
#endif
            break;
        }
#endif
    }

    //
    // Bring the interface up.
    //
    if(ulIPMode == IPADDR_USE_STATIC)
        netif_set_up(&g_sNetIF);

    //
    // Save the new mode.
    //
    g_ulIPMode = ulIPMode;
}

//*****************************************************************************
//
//! Change the configuration of the lwIP network interface.
//!
//! \param ulIPAddr is the new IP address to be used (static).
//! \param ulNetMask is the new network mask to be used (static).
//! \param ulGWAddr is the new Gateway address to be used (static).
//! \param ulIPMode is the IP Address Mode.  \b IPADDR_USE_STATIC 0 will force
//! static IP addressing to be used, \b IPADDR_USE_DHCP will force DHCP with
//! fallback to Link Local (Auto IP), while \b IPADDR_USE_AUTOIP will force
//! Link Local only.
//!
//! This function will evaluate the new configuration data.  If necessary, the
//! interface will be brought down, reconfigured, and then brought back up
//! with the new configuration.
//!
//! \return None.
//
//*****************************************************************************
void
lwIPNetworkConfigChange(unsigned long ulIPAddr, unsigned long ulNetMask,
                        unsigned long ulGWAddr, unsigned long ulIPMode)
{
    //
    // Check the parameters.
    //
#if LWIP_DHCP && LWIP_AUTOIP
    ASSERT((ulIPMode == IPADDR_USE_STATIC) ||
           (ulIPMode == IPADDR_USE_DHCP) ||
           (ulIPMode == IPADDR_USE_AUTOIP));
#elif LWIP_DHCP
    ASSERT((ulIPMode == IPADDR_USE_STATIC) ||
           (ulIPMode == IPADDR_USE_DHCP));
#elif LWIP_AUTOIP
    ASSERT((ulIPMode == IPADDR_USE_STATIC) ||
           (ulIPMode == IPADDR_USE_AUTOIP));
#else
    ASSERT(ulIPMode == IPADDR_USE_STATIC);
#endif

    //
    // Save the network configuration for later use by the private network
    // configuration change.
    //
    g_ulIPAddr = ulIPAddr;
    g_ulNetMask = ulNetMask;
    g_ulGWAddr = ulGWAddr;

    //
    // Complete the network configuration change.  The remainder is done
    // immediately if not using a RTOS and it is deferred to the TCP/IP
    // thread's context if using a RTOS.
    //
    tcpip_callback(lwIPPrivateNetworkConfigChange, (void *)ulIPMode);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
