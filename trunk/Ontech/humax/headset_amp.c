/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
*/

/*!
@file    headset_amp.c        
@brief    Implementation of the audio amp control functionality.
*/

/****************************************************************************
    Header files
*/

#include "headset_amp.h"
#include "headset_debug.h"

#include <pio.h>


#ifdef DEBUG_AMP
    #define AMP_DEBUG(x) DEBUG(x)    
#else
    #define AMP_DEBUG(x) 
#endif   


/****************************************************************************
 	Enable or disable the audio amp.
*/
static void SET_AMP ( bool on )
{	
	uint32 pio = (uint32)1 << theHeadset.ampPio;
	
	PioSetDir(pio, pio); 
	
	if (on)
		PioSet(pio, pio);
	else
		PioSet(pio, 0);
}


/****************************************************************************
  FUNCTIONS
*/

/**************************************************************************/
void AmpOn ( void )
{
	if (theHeadset.useAmp)
	{
		AMP_DEBUG(("Amp: Cancel switch off\n"));
		MessageCancelAll(&theHeadset.task , APP_AMP_OFF);
					 
		if (!theHeadset.ampOn)
		{
			AMP_DEBUG(("Amp: Switch on\n"));
			SET_AMP(TRUE);
			theHeadset.ampOn = TRUE;
		}
	}
}


/**************************************************************************/
void AmpOff ( void )
{
	if (theHeadset.useAmp & theHeadset.ampOn)
	{
		AMP_DEBUG(("Amp: Switch off\n"));
		SET_AMP(FALSE);
		theHeadset.ampOn = FALSE;
	}
}


/**************************************************************************/
void AmpOffLater ( void )
{
	if (theHeadset.useAmp & theHeadset.ampAutoOff & theHeadset.ampOn)
	{
		AMP_DEBUG(("Amp: Switch off later\n"));
		MessageSendLater(&theHeadset.task , APP_AMP_OFF , 0 , D_SEC(theHeadset.ampOffDelay));
	}
}		


/**************************************************************************/
void AmpSetOffDelay ( uint16 delay )
{
	AMP_DEBUG(("Amp: Set off delay %d secs\n",delay));
	theHeadset.ampOffDelay = delay;
}


/**************************************************************************/
void AmpSetPio ( uint16 pio )
{
	AMP_DEBUG(("Amp: PIO used for amp: %d\n",theHeadset.useAmp));
	AMP_DEBUG(("Amp: Set PIO %d for amp\n",pio));
	theHeadset.ampPio = pio;
}
