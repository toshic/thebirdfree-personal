/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_avrcp_event_handler.c
@brief    Implementation of AVRCP event handlers.
*/

#include "headset_a2dp_connection.h"
#include "headset_a2dp_stream_control.h"
#include "headset_avrcp_event_handler.h"
#include "headset_debug.h"
#include "headset_statemanager.h"

#include <panic.h>


#ifdef DEBUG_AVRCP_EVENT
#define AVRCP_EVENT_DEBUG(x) DEBUG(x)
#else
#define AVRCP_EVENT_DEBUG(x) 
#endif


/****************************************************************************
  LOCAL FUNCTIONS
*/


/**************************************************************************/
static void sendAVRCP(avc_operation_id op_id, uint8 state)
{
    theHeadset.avrcp_data.pending = TRUE;
    
    /* Send a key press */
    AvrcpPassthrough(theHeadset.avrcp, subunit_panel, 0, state, op_id, 0, 0);   
}


/**************************************************************************/
static void avrcpSendControlMessage(avrcp_controls control)
{
	if ( stateManagerIsAvrcpConnected() )
    {
		APP_AVRCP_CONTROLS_T *message = PanicUnlessNew(APP_AVRCP_CONTROLS_T);
		message->control = control;
		MessageSendConditionally(&theHeadset.task, APP_AVRCP_CONTROLS, message, &theHeadset.avrcp_data.pending);
	}
}


/**************************************************************************/
static void avrcpStopPress(void)
{
    /* see controls_handler description */
    avrcpSendControlMessage(AVRCP_CTRL_STOP_PRESS);
}


/**************************************************************************/
static void avrcpStopRelease(void)
{
    /* see button_handler message queue description */
    avrcpSendControlMessage(AVRCP_CTRL_STOP_RELEASE);
}


/**************************************************************************/
static void avrcpPausePress(void)
{
    /* see controls_handler description */
    avrcpSendControlMessage(AVRCP_CTRL_PAUSE_PRESS);
}


/**************************************************************************/
static void avrcpPauseRelease(void)
{
    /* see button_handler message queue description */
    avrcpSendControlMessage(AVRCP_CTRL_PAUSE_RELEASE);
}


/**************************************************************************/
static void avrcpPlayPress(void)
{
    /* see controls_handler description */
    avrcpSendControlMessage(AVRCP_CTRL_PLAY_PRESS);
}


/**************************************************************************/
static void avrcpPlayRelease(void)
{
    /* see button_handler message queue description */
    avrcpSendControlMessage(AVRCP_CTRL_PLAY_RELEASE);
}


/**************************************************************************/
static void avrcpForwardPress(void)
{
    /* see controls_handler description */
    avrcpSendControlMessage(AVRCP_CTRL_FORWARD_PRESS);
}


/**************************************************************************/
static void avrcpForwardRelease(void)
{
    /* see button_handler message queue description */
    avrcpSendControlMessage(AVRCP_CTRL_FORWARD_RELEASE);
}


/**************************************************************************/
static void avrcpBackwardPress(void)
{
    /* see controls_handler description */
    avrcpSendControlMessage(AVRCP_CTRL_BACKWARD_PRESS);
}


/**************************************************************************/
static void avrcpBackwardRelease(void)
{
    /* see button_handler message queue description */
    avrcpSendControlMessage(AVRCP_CTRL_BACKWARD_RELEASE);
}


/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************/
void avrcpEventHandleControls(APP_AVRCP_CONTROLS_T *msg)
{
	AVRCP_EVENT_DEBUG(("APP_AVRCP_CONTROLS : "));
    switch (msg->control)
    {
        case AVRCP_CTRL_PAUSE_PRESS:
	        AVRCP_EVENT_DEBUG(("Sending Pause Pressed\n"));
            sendAVRCP(opid_pause, 0);
            break;
        case AVRCP_CTRL_PAUSE_RELEASE:
            AVRCP_EVENT_DEBUG(("Sending Pause Released\n"));
            sendAVRCP(opid_pause, 1);
            break;
        case AVRCP_CTRL_PLAY_PRESS:
            AVRCP_EVENT_DEBUG(("Sending Play Pressed\n"));
            sendAVRCP(opid_play, 0);
            break;
        case AVRCP_CTRL_PLAY_RELEASE:
            AVRCP_EVENT_DEBUG(("Sending Play Released\n"));
            sendAVRCP(opid_play, 1);
            break;
        case AVRCP_CTRL_FORWARD_PRESS:
            AVRCP_EVENT_DEBUG(("Sending Forward Pressed\n"));
            sendAVRCP(opid_forward, 0);
            break;
        case AVRCP_CTRL_FORWARD_RELEASE:
            AVRCP_EVENT_DEBUG(("Sending Forward Released\n"));
            sendAVRCP(opid_forward, 1);
            break;
        case AVRCP_CTRL_BACKWARD_PRESS:
            AVRCP_EVENT_DEBUG(("Sending Backward Pressed\n"));
            sendAVRCP(opid_backward, 0);
            break;
        case AVRCP_CTRL_BACKWARD_RELEASE:
            AVRCP_EVENT_DEBUG(("Sending Backward Released\n"));
            sendAVRCP(opid_backward, 1);
            break;
        case AVRCP_CTRL_STOP_PRESS:
            AVRCP_EVENT_DEBUG(("Sending Stop Pressed\n"));
            sendAVRCP(opid_stop, 0);
            break;
        case AVRCP_CTRL_STOP_RELEASE:
            AVRCP_EVENT_DEBUG(("Sending Stop Released\n"));
            sendAVRCP(opid_stop, 1);
            break;
        case AVRCP_CTRL_FF_PRESS:
            AVRCP_EVENT_DEBUG(("Sending FF Pressed\n"));
            sendAVRCP(opid_fast_forward, 0);
            break;
        case AVRCP_CTRL_FF_RELEASE:
            AVRCP_EVENT_DEBUG(("Sending FF Released\n"));
            sendAVRCP(opid_fast_forward, 1);
            break;
        case AVRCP_CTRL_REW_PRESS:
            AVRCP_EVENT_DEBUG(("Sending REW Pressed\n"));
            sendAVRCP(opid_rewind, 0);
            break;
        case AVRCP_CTRL_REW_RELEASE:
            AVRCP_EVENT_DEBUG(("Sending REW Released\n"));
            sendAVRCP(opid_rewind, 1);
            break;
		default:
			break;
    }
}


/**************************************************************************/
void avrcpEventPlay(void)
{
	AVRCP_EVENT_DEBUG(("avrcpEventPlay\n"));
	
	if ( stateManagerIsAvrcpConnected () )
	{
		/* There is an AVRCP connection */
		
		/* If no media channel connected, then connect it now */
		if (!A2dpGetMediaSink(theHeadset.a2dp))
		{
			a2dpConnectRequest(TRUE, TRUE);
		}
		else
		{		
			/* Don't try to auto send AVRCP as user has intervened */
			theHeadset.sendPlayOnConnection = FALSE;
			
			/* AVRCP is connected so send AVRCP Play. */
			avrcpSendPlay();
		
			/* Change to streaming state if currently paused. */
			if ( stateManagerGetA2dpState () == headsetA2dpPaused )
				stateManagerEnterA2dpStreamingState();
			else if (!stateManagerIsA2dpStreaming())
			{
				/* There is a media connection but we aren't streaming, so send A2dpStart */
				streamControlStartA2dp();
			}
		}
	}	
	else
	{
		/* Toggle streaming state */
		if ( stateManagerGetA2dpState () == headsetA2dpConnected )
		{
			if ( A2dpGetMediaSink(theHeadset.a2dp) )
			{
				/* There is a media connection but we aren't streaming, so send A2dpStart */
				streamControlBeginA2dpStreaming();
			}
			else
			{
				/* No media connection so connect it now. */
				a2dpConnectRequest(TRUE, TRUE);	
			}
		}
		else if ( (stateManagerGetA2dpState () == headsetA2dpStreaming ) || (stateManagerGetA2dpState () == headsetA2dpPaused) )
		{
			streamControlCeaseA2dpStreaming(TRUE);
		}
	}
}


/**************************************************************************/
void avrcpEventPause(void)
{
	AVRCP_EVENT_DEBUG(("avrcpEventPause\n"));	
	if ( stateManagerIsAvrcpConnected () )
	{					
		/* Don't try to auto send AVRCP as user has intervened */
		theHeadset.sendPlayOnConnection = FALSE;
			
		/* AVRCP is connected so send AVRCP Pause. */
		avrcpSendPause();
		
		/* Change to paused state if currently streaming . */
		if ( stateManagerGetA2dpState () == headsetA2dpStreaming )
			stateManagerEnterA2dpPausedState();
	}	
	else
	{
		/* Toggle streaming state */
		if ( stateManagerGetA2dpState () == headsetA2dpConnected )
		{
			if ( A2dpGetMediaSink(theHeadset.a2dp) )
			{
				/* There is a media connection but we aren't streaming, so send A2dpStart */
				streamControlBeginA2dpStreaming();
			}
			else
			{
				/* No media connection so connect it now. */
				a2dpConnectRequest(TRUE, TRUE);	
			}
		}
		else if ( (stateManagerGetA2dpState () == headsetA2dpStreaming ) || (stateManagerGetA2dpState () == headsetA2dpPaused) )
		{
			streamControlCeaseA2dpStreaming(TRUE);
		}
	}
}


/**************************************************************************/
void avrcpEventStop(void)
{
	AVRCP_EVENT_DEBUG(("avrcpEventStop\n"));
	if ( stateManagerIsAvrcpConnected () )
	{					
		/* Don't try to auto send AVRCP as user has intervened */
		theHeadset.sendPlayOnConnection = FALSE;
		
		/* AVRCP is connected so send AVRCP Stop. */
		avrcpSendStop();
		
		/* Change to paused state if currently streaming . */
		if ( stateManagerGetA2dpState () == headsetA2dpStreaming )
			stateManagerEnterA2dpPausedState();
	}	
	else
	{
		/* No AVRCP connection, but try and stop streaming. */
		if ( (stateManagerGetA2dpState () == headsetA2dpStreaming ) || (stateManagerGetA2dpState () == headsetA2dpPaused) )
		{
			streamControlCeaseA2dpStreaming(TRUE);
		}
	}
}


/**************************************************************************/
void avrcpEventSkipForward(void)
{
    if ( stateManagerIsAvrcpConnected() )
    {
        avrcpForwardPress();
        avrcpForwardRelease();
    }
}


/**************************************************************************/
void avrcpEventSkipBackward(void)
{
    if ( stateManagerIsAvrcpConnected() )
    {
        avrcpBackwardPress();
        avrcpBackwardRelease();
    }
}


/*************************************************************************/
void avrcpEventFastForwardPress(void)
{
    avrcpSendControlMessage(AVRCP_CTRL_FF_PRESS);
}


/*************************************************************************/
void avrcpEventFastForwardRelease(void)
{
    avrcpSendControlMessage(AVRCP_CTRL_FF_RELEASE);
}


/*************************************************************************/
void avrcpEventFastRewindPress(void)
{
    avrcpSendControlMessage(AVRCP_CTRL_REW_PRESS);
}


/*************************************************************************/
void avrcpEventFastRewindRelease(void)
{
    avrcpSendControlMessage(AVRCP_CTRL_REW_RELEASE);
}


/**************************************************************************/
void avrcpSendPause(void)
{
    if ( stateManagerIsAvrcpConnected() )
    {
		/* If the A2DP state is streaming or paused then this tracks what it thinks is
		   the playing status of the media. If the A2DP state is connected, then it needs to 
		   track the AVRCP commands independently of the A2DP state.
		*/
		if ( !stateManagerIsA2dpStreaming() )
			theHeadset.PlayingState = 1 - theHeadset.PlayingState;
		else
			theHeadset.PlayingState = 0;
		
        avrcpPausePress();
        avrcpPauseRelease();
    }
}


/**************************************************************************/
void avrcpSendPlay(void)
{
    if ( stateManagerIsAvrcpConnected() )
    {
		/* If the A2DP state is streaming or paused then this tracks what it thinks is
		   the playing status of the media. If the A2DP state is connected, then it needs to 
		   track the AVRCP commands independently of the A2DP state.
		*/
		if ( !stateManagerIsA2dpStreaming() )
			theHeadset.PlayingState = 1 - theHeadset.PlayingState;
		else
			theHeadset.PlayingState = 1;
					
        avrcpPlayPress();
        avrcpPlayRelease();
    }
}


/**************************************************************************/
void avrcpSendStop(void)
{
    if ( stateManagerIsAvrcpConnected() )
    {
		/* The media should now be stopped */
		theHeadset.PlayingState = 0;
		
        avrcpStopPress();
        avrcpStopRelease();
    }
}


/*************************************************************************/
void handleAVRCPConnectReq(APP_AVRCP_CONNECT_REQ_T *msg)
{
    if ((stateManagerGetAvrcpState() == avrcpReady) && (stateManagerGetHfpState() != headsetPoweringOn) && theHeadset.features.UseAVRCPprofile)
    {
        MessageCancelAll(&theHeadset.task, APP_AVRCP_CONNECT_REQ);
        /* Change to connecting state */
	    stateManagerSetAvrcpState(avrcpConnecting);
        /* Establish AVRCP connection */
	    AvrcpConnect(theHeadset.avrcp, &msg->addr);
    }
}


/**************************************************************************/
void avrcpConnectReq(bdaddr addr, bool delay_request)
{
	APP_AVRCP_CONNECT_REQ_T *message = (APP_AVRCP_CONNECT_REQ_T*)PanicUnlessMalloc(sizeof(APP_AVRCP_CONNECT_REQ_T));
	message->addr = addr;
	if (delay_request)
		MessageSendLater(&theHeadset.task, APP_AVRCP_CONNECT_REQ, message, 3000);
	else
    	MessageSend(&theHeadset.task, APP_AVRCP_CONNECT_REQ, message);
}


/**************************************************************************/
void avrcpDisconnectReq(void)
{
    if ( stateManagerIsAvrcpConnected() )
    {
        /* Disconnect AVRCP connection */
	    AvrcpDisconnect(theHeadset.avrcp);
    }
}

