/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_a2dp_connection.c
@brief    Handles a2dp connection.
*/


#include "headset_a2dp_connection.h"
#include "headset_a2dp_stream_control.h"
#include "headset_avrcp_event_handler.h"
#include "headset_configmanager.h"
#include "headset_debug.h"
#include "headset_hfp_slc.h"
#include "headset_init.h"
#include "headset_statemanager.h"

#include <bdaddr.h>
#include <ps.h>


#ifdef DEBUG_A2DP_CONNECTION
#define A2DP_CONNECTION_DEBUG(x) DEBUG(x)
#else
#define A2DP_CONNECTION_DEBUG(x) 
#endif


/****************************************************************************/
bool a2dpConnectRequest(bool isSource, bool manual_connect)
{
    bdaddr addr;
	uint8 seid = SBC_SINK_SEID;
	uint8 seids[2];
	uint16 size_seids = 1;

	if(isSource)
    	seid = SBC_SOURCE_SEID;
	
	if (stateManagerGetHfpState() == headsetPoweringOn)
	{
		A2DP_CONNECTION_DEBUG(("A2DP_CONNECTION: Already powered off\n"));
		return FALSE;
	}
    
    theHeadset.a2dpConnecting = TRUE;
	
	/* Send an avrcp_play once media has connected and entered
	   the correct state. This is so A2DP sources (standalone and AGs)
	   will start playing music straight away.
	*/
	theHeadset.sendPlayOnConnection = TRUE;
	theHeadset.manualA2dpConnect = manual_connect;

	seids[0] = seid;
	
	if (!A2dpGetMediaSink(theHeadset.a2dp))
	{
		A2DP_CONNECTION_DEBUG(("A2DP_CONNECTION: Connect\n"));
		/* Open media channel. Call the correct API depending on whether signalling connected or not. */
    	if (A2dpGetSignallingSink(theHeadset.a2dp))
			A2dpOpen(theHeadset.a2dp, size_seids, seids);
    	else
        	A2dpConnectOpen(&theHeadset.task, &addr, size_seids, seids, theHeadset.a2dp_data.sep_entries);
	}
	return TRUE;
}


/****************************************************************************/
bool a2dpConnectBdaddrRequest(bdaddr *pAddr, bool isSource)
{
	uint8 seid = SBC_SINK_SEID;
	uint8 seids[2];
	uint16 size_seids = 1;

	if(isSource)
	    seid = SBC_SOURCE_SEID;
	
	if ((stateManagerGetHfpState() == headsetPoweringOn) || 
		(stateManagerGetA2dpState() != headsetA2dpConnectable) ||
		a2dpIsConnecting())
	{
		A2DP_CONNECTION_DEBUG(("A2DP_CONNECTION: Invalid state\n"));
		return FALSE;
	}
	
    seids[0] = seid;
	
	/* Open media channel. Call the correct API depending on whether signalling connected or not. */
    if (!A2dpGetSignallingSink(theHeadset.a2dp))
    	A2dpConnectOpen(&theHeadset.task, pAddr, size_seids, seids, theHeadset.a2dp_data.sep_entries);
	else
		return FALSE;
				
    theHeadset.a2dpConnecting = TRUE;
	
	/* Send an avrcp_play once media has connected and entered
	   the correct state. This is so A2DP sources (standalone and AGs)
	   will start playing music straight away.
	*/
	theHeadset.sendPlayOnConnection = TRUE;
	theHeadset.manualA2dpConnect = TRUE;			
				
	return TRUE;
}


/****************************************************************************/
void a2dpDisconnectRequest(void)
{
	/* Don't resume A2DP streaming */
	streamControlCancelResumeA2dpStreaming();
	
	/* Disconnect AVRCP first. */
	avrcpDisconnectReq();
	
	/* Close all A2DP signalling and media channels */
	A2dpDisconnectAll(theHeadset.a2dp);
}


/****************************************************************************/
bool a2dpIsConnecting(void)
{
	return theHeadset.a2dpConnecting;	
}

/****************************************************************************/
void a2dpSwitchSource()
{
	Sink sink = A2dpGetSignallingSink(theHeadset.a2dp);
	bdaddr addr;
	
	if (stateManagerIsA2dpConnected() && sink)
    {      
		if (SinkGetBdAddr(sink, &addr))
		{
			/* Disconnect from current A2DP source */
			theHeadset.switch_a2dp_source = TRUE;
    		a2dpDisconnectRequest();
		}
    }  
}


