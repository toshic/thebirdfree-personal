/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_pio.h
@brief    Part of the ledmanager Module responsible for managing the PIO outputs excluding LEDs
*/

#ifndef HEADSET_PIO_H
#define HEADSET_PIO_H

#ifdef ROM_LEDS

#define DIM_MSG_BASE (0x1000)

#include "headset_led_manager.h"


/****************************************************************************
  FUNCTIONS
*/


/****************************************************************************
NAME	
	PioSetLedPin

DESCRIPTION
    Fn to change set an LED attached to a PIO, a special LED pin , or a tricolour LED.
    
*/
void PioSetLedPin ( LedTaskData * pLedTask , uint16 pPIO , bool pOnOrOff ) ;


/****************************************************************************
NAME	
	PioSetPio

DESCRIPTION
    Fn to change a PIO.
    
*/
void PioSetPio ( uint16 pPIO , bool pOnOrOff  ) ;


/****************************************************************************
NAME	
	PioSetDimState  
	
DESCRIPTION
    Update funtion for a led that is currently dimming.
    
*/
void PioSetDimState ( LedTaskData * pLedTask , uint16 pPIO );


#endif /* ROM_LEDS */

#endif /* HEADSET_PIO_H */
