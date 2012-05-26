/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_hfp_call.h
@brief    Handles HFP call functionality.
*/
#ifndef HEADSET_HFP_CALL_H
#define HEADSET_HFP_CALL_H


#include "headset_private.h"


/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************
NAME    
    hfpCallInitiateVoiceDial
    
DESCRIPTION
    If HFP and connected - issues command
    If HFP and not connected - connects and issues if not in call
    If HSP sends button press

*/
bool hfpCallInitiateVoiceDial ( void );


/****************************************************************************
NAME    
    hfpCallCancelVoiceDial
    
DESCRIPTION
    Cancels a voice dial request.
   
*/
void hfpCallCancelVoiceDial ( void );


/****************************************************************************
NAME    
    hfpCallInitiateLNR
    
DESCRIPTION
    If HFP and connected - issues command
    If HFP and not connected - connects and issues if not in call
    If HSP sends button press

*/
bool hfpCallInitiateLNR ( void );


/****************************************************************************
NAME    
    hfpCallAnswer
    
DESCRIPTION
    Answer an incoming call from the headset.

*/
void hfpCallAnswer ( void );


/****************************************************************************
NAME    
    headsetCallReject
    
DESCRIPTION
    Reject an incoming/ outgoing call from the headset.

*/
void hfpCallReject ( void );


/****************************************************************************
NAME    
    hfpCallHangUp
    
DESCRIPTION
    Hang up the call from the headset.

*/
void hfpCallHangUp ( void );


/****************************************************************************
NAME    
    hfpCallTransferToggle
    
DESCRIPTION
    If the audio is at the headset end transfer it back to the AG and
    vice versa.

*/
void hfpCallTransferToggle ( void );


/****************************************************************************
NAME    
    hfpCallRecallQueuedEvent
    
DESCRIPTION
    Checks to see if an event was Queued and issues it.

*/
void hfpCallRecallQueuedEvent ( void );


/****************************************************************************
NAME    
    hfpCallClearQueuedEvent
    
DESCRIPTION
    Clears the QUEUE - used on failure to connect / power on / off etc.

*/
void hfpCallClearQueuedEvent ( void );


/****************************************************************************
NAME    
    headsetCheckForAudioTransfer
    
DESCRIPTION
    Checks on connection for an audio connction and performs a transfer if not present

*/
void headsetCheckForAudioTransfer ( void ) ;


#endif
