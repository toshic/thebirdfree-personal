/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
*/

/*!
@file    headset_a2dp_stream_control.h
@brief    Authorises remote device.
*/

#include "headset_private.h"

#ifndef _HEADSET_A2DP_STREAM_CONTROL_H_
#define _HEADSET_A2DP_STREAM_CONTROL_H_


/****************************************************************************
  FUNCTIONS
*/


/*************************************************************************
NAME    
     streamControlCeaseA2dpStreaming
    
DESCRIPTION
     Shutdown A2dp audio and send A2DP Suspend if required.

*/
void streamControlCeaseA2dpStreaming(bool send_suspend);


/*************************************************************************
NAME    
     streamControlConnectA2dpAudio
    
DESCRIPTION
     Connect A2dp audio.

*/
void streamControlConnectA2dpAudio(void);


/*************************************************************************
NAME    
     streamControlResumeA2dpStreaming
    
DESCRIPTION
     Tell app that it needs to restart A2DP streaming and audio.

*/
void streamControlResumeA2dpStreaming(uint32 user_delay);


/*************************************************************************
NAME    
     streamControlCancelResumeA2dpStreaming
    
DESCRIPTION
     Tell app not to resume A2DP streaming and audio.

*/
void streamControlCancelResumeA2dpStreaming(void);


/*************************************************************************
NAME    
     streamControlBeginA2dpStreaming
    
DESCRIPTION
     Actually begin A2DP streaming and audio now if possible.

*/
void streamControlBeginA2dpStreaming(void);


/*************************************************************************
NAME    
     streamControlStartA2dp
    
DESCRIPTION
     Send A2dp Start, but only if the headset Suspended the A2DP source.

*/
void streamControlStartA2dp(void);


/*************************************************************************
NAME    
     IsA2dpSourceAnAg
    
DESCRIPTION
     Are we connected to an AG and A2DP source which are the same device.

*/
bool IsA2dpSourceAnAg(void);


#endif /* _HEADSET_A2DP_STREAM_CONTROL_H_ */
