/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_led_manager.h
@brief   Interface to LED manager functionality. 
*/

#ifndef HEADSET_LED_MANAGER_H
#define HEADSET_LED_MANAGER_H


#include "headset_private.h"
#include "headset_states.h"
#include "headset_events.h"


/****************************************************************************
  FUNCTIONS
*/

#ifdef ROM_LEDS


/****************************************************************************
NAME 
    LEDManagerInit

DESCRIPTION
    Initialises LED manager.

*/
void LEDManagerInit ( void ) ;


/****************************************************************************
NAME 
    LEDManagerAddLEDStatePattern

DESCRIPTION
    Adds a state LED mapping.

*/
void LEDManagerAddLEDStatePattern (  headsetHfpState pState , headsetA2dpState pA2dpState , LEDPattern_t* pPattern ) ;  


/****************************************************************************
NAME 
    LEDManagerAddLEDFilter

DESCRIPTION
    Adds an event LED mapping.
    
*/
void LEDManagerAddLEDFilter  ( LEDFilter_t* pLedFilter ) ;  


/****************************************************************************
NAME 
    LEDManagerAddLEDEventPattern

DESCRIPTION
    Adds an event LED mapping.
    
*/
void LEDManagerAddLEDEventPattern ( headsetEvents_t pEvent , LEDPattern_t* pPattern ) ;  


/****************************************************************************
NAME 
    LEDManagerIndicateEvent

DESCRIPTION
    Displays event notification.
    This function also enables / disables the event filter actions - if a normal event indication is not
    associated with the event, it checks to see if a filer is set up for the event.
    
*/
void LEDManagerIndicateEvent ( MessageId pEvent ) ;


/****************************************************************************
NAME	
	LEDManagerIndicateState

DESCRIPTION
	Displays state indication information.

*/
void LEDManagerIndicateState ( headsetHfpState pState , headsetA2dpState pA2dpState )  ;


/****************************************************************************
NAME	
	LedManagerDisableLEDS

DESCRIPTION
    Disable LED indications.

*/
void LedManagerDisableLEDS ( void ) ;


/****************************************************************************
NAME	
	LedManagerEnableLEDS

DESCRIPTION
    Enable LED indications.
    
*/
void LedManagerEnableLEDS  ( void ) ;


/****************************************************************************
NAME	
	LedManagerToggleLEDS

DESCRIPTION
    Toggle Enable / Disable LED indications.

*/
void LedManagerToggleLEDS  ( void )  ;


/****************************************************************************
NAME	
	LedManagerResetLEDIndications

DESCRIPTION
    Resets the LED Indications and reverts to state indications
	Sets the Flag to allow the Next Event to interrupt the current LED Indication
    Used if you have a permanent LED event indication that you now want to interrupt.
    
*/
void LedManagerResetLEDIndications ( void ) ;


/****************************************************************************
NAME	
	LEDManagerResetStateIndNumRepeatsComplete

DESCRIPTION
    Resets the LED Number of Repeats complete for the current state indication
       This allows the time of the led indication to be reset every time an event 
       occurs.
       
*/
void LEDManagerResetStateIndNumRepeatsComplete  ( void ) ;


/****************************************************************************
NAME 
    LMPrintPattern

DESCRIPTION
    Debug fn to output a LED pattern.
    
*/
#ifdef DEBUG_LM
void LMPrintPattern ( LEDPattern_t * pLED ) ;
#endif

#else /* ROM_LEDS */

	void ledsIndicateEvent( headsetEvents_t event )  ;
	void ledsIndicateState( headsetHfpState pState , headsetA2dpState pA2dpState )  ;

	#define LEDManagerIndicateEvent(y) ledsIndicateEvent(y) 

	#define LEDManagerIndicateState(y,z) ledsIndicateState(y,z) 
	
#endif /* ROM_LEDS */
	
#endif /* HEADSET_LED_MANAGER_H */
