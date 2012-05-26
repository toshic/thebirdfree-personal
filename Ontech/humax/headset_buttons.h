/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_buttons.h
@brief   Interface to the headset button functionality. 
*/
#ifndef HEADSET_BUTTONS_H
#define HEADSET_BUTTONS_H

#include "headset_buttonmanager.h"

/****************************************************************************
NAME 
 ButtonsInit

DESCRIPTION
 Initialises the button event 

RETURNS
 void
    
*/
void ButtonsInit ( void ) ;


/****************************************************************************
NAME 
	ButtonsCheckForChangeAfterInit 
 
DESCRIPTION
 	Called after the configuration has been read and will trigger buttons events
    if a pio has been pressed or held whilst the configuration was still being loaded
    , i.e. the power on button press    
*/
void ButtonsCheckForChangeAfterInit ( void );


#endif
