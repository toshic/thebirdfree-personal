/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_avrcp_event_handler.h
@brief    Interface to AVRCP event handlers.
*/
#ifndef HEADSET_AVRCP_EVENT_HANDLER_H
#define HEADSET_AVRCP_EVENT_HANDLER_H


#include "headset_private.h"


/*************************************************************************
NAME    
     avrcpEventHandleControls
    
DESCRIPTION
     Handles the Avrcp control messages and sends the AVRCP command to the remote end.
*/
void avrcpEventHandleControls(APP_AVRCP_CONTROLS_T *msg);


/*************************************************************************
NAME    
     avrcpEventPlay
    
DESCRIPTION
     Handles the Avrcp Play event.
*/
void avrcpEventPlay(void);


/*************************************************************************
NAME    
     avrcpEventPause
    
DESCRIPTION
     Handles the Avrcp Pause event.
*/
void avrcpEventPause(void);


/*************************************************************************
NAME    
     avrcpEventStop
    
DESCRIPTION
     Handles the Avrcp Stop event.
*/
void avrcpEventStop(void);       


/*************************************************************************
NAME
    avrcpEventSkipForward

DESCRIPTUION
    Sends an AVRCP Skip Forward.
*/
void avrcpEventSkipForward(void);


/*************************************************************************
NAME
    avrcpEventSkipBackward

DESCRIPTUION
    Sends an AVRCP Skip Backward.
*/
void avrcpEventSkipBackward(void);


/*************************************************************************
NAME
    avrcpSendStop

DESCRIPTUION
    Sends an AVRCP Stop.
*/
void avrcpSendStop(void);


/*************************************************************************
NAME    
     avrcpEventFastForwardPress
    
DESCRIPTION
     Signal that Fast Forward has been pressed
*/
void avrcpEventFastForwardPress(void);


/*************************************************************************
NAME    
     avrcpEventFastForwardRelease
    
DESCRIPTION
     Signal that Fast Forward has been released
*/
void avrcpEventFastForwardRelease(void);


/*************************************************************************
NAME    
     avrcpEventFastRewindPress
    
DESCRIPTION
     Signal that Fast Rewind has been pressed
*/
void avrcpEventFastRewindPress(void);


/*************************************************************************
NAME    
     avrcpEventFastRewindRelease
    
DESCRIPTION
     Signal that Fast Rewind has been released
*/
void avrcpEventFastRewindRelease(void);


/*************************************************************************
NAME
    avrcpSendPlay

DESCRIPTUION
    Sends an AVRCP Play.
*/
void avrcpSendPlay(void);


/*************************************************************************
NAME
    avrcpSendPause

DESCRIPTUION
    Sends an AVRCP Pause.
*/
void avrcpSendPause(void);


/*************************************************************************
NAME    
     handleAVRCPConnectReq
    
DESCRIPTION
     This function is called to create an AVRCP connection.     
*/
void handleAVRCPConnectReq(APP_AVRCP_CONNECT_REQ_T *msg);


/*************************************************************************
NAME    
     avrcpConnectReq
    
DESCRIPTION
     This function sends an internal message to create an AVRCP connection.
	 If delay_request is set to TRUE then the AVRCP connection is delayed by a fixed time.
*/
void avrcpConnectReq(bdaddr addr, bool delay_request);


/*************************************************************************
NAME    
     avrcpDisconnectReq
    
DESCRIPTION
     This function is called to disconnect an AVRCP connection     
*/
void avrcpDisconnectReq(void);


#endif
