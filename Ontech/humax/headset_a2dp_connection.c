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


/****************************************************************************
  FUNCTIONS
*/
bool a2dpGetLastUsedSource(bdaddr *addr, uint8 *seid)
{
	if (!BdaddrIsZero(&theHeadset.LastDevices->lastA2dpConnected))
	{
		*addr = theHeadset.LastDevices->lastA2dpConnected;
		*seid = theHeadset.last_used_seid;	
	}
	else if (!BdaddrIsZero(&theHeadset.LastDevices->lastPaired))
	{
		*addr = theHeadset.LastDevices->lastPaired;
		*seid = 0;
	}
	else
	{
		return FALSE;	
	}
		
	if (*seid == 0)
	{
		/* No remote device has connected to a SEP on this device, so no record of which SEP to use.  */
		/* Default to using SBC */
		*seid = SBC_SINK_SEID;
	}
	
	return TRUE;
}


/****************************************************************************/
bool a2dpEstablishConnection(bool a2dp_ag_connect_signalling_only, bool manual_connect)
{
	bdaddr a2dp_addr, ag_addr;	
	uint8 seid;
	
	if ( (stateManagerGetA2dpState() == headsetA2dpConnectable) && !a2dpIsConnecting() )
	{
		/* 	
			If this A2DP source is also an AG then only connect signalling channel. 
			If the A2DP source and AG are different devices then connect signalling and media channels.	   
		*/
		if (!a2dpGetLastUsedSource(&a2dp_addr, &seid))
			return FALSE;
	
		if (!hfpSlcGetLastConnectedAG(&ag_addr))
		{
			return (a2dpConnectRequest(TRUE, FALSE, manual_connect));
		}
	
		if (BdaddrIsSame(&a2dp_addr, &ag_addr) && a2dp_ag_connect_signalling_only)
		{
			if (!A2dpGetSignallingSink(theHeadset.a2dp))
			{
				return (a2dpConnectRequest(FALSE, FALSE, manual_connect));
			}
		}
		else
		{
			return (a2dpConnectRequest(TRUE, FALSE, manual_connect));
		}
	}
	
	return FALSE;
}


/****************************************************************************/
bool a2dpConnectRequest(bool connect_media, bool isSource, bool manual_connect)
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
    
	if (!a2dpGetLastUsedSource(&addr, &seid) || !theHeadset.features.UseA2DPprofile)
	{
		MessageSend(&theHeadset.task, EventA2dpReconnectFailed, 0);
		return FALSE;
	}
				
    theHeadset.a2dpConnecting = TRUE;
	
	/* Send an avrcp_play once media has connected and entered
	   the correct state. This is so A2DP sources (standalone and AGs)
	   will start playing music straight away.
	*/
	if (connect_media)
	{
		theHeadset.sendPlayOnConnection = TRUE;
		theHeadset.manualA2dpConnect = manual_connect;
		
		if (!A2dpGetMediaSink(theHeadset.a2dp))
		{
			A2DP_CONNECTION_DEBUG(("A2DP_CONNECTION: Connect\n"));
			/* Open media channel. Call the correct API depending on whether signalling connected or not. */
        	if (A2dpGetSignallingSink(theHeadset.a2dp))
    			A2dpOpen(theHeadset.a2dp, size_seids, seids);
        	else
            	A2dpConnectOpen(&theHeadset.task, &addr, size_seids, seids, theHeadset.a2dp_data.sep_entries);
		}
	}
	else
	{
		/* Connect signalling channel only */
		A2dpConnectSignallingChannel(&theHeadset.task, &addr, theHeadset.a2dp_data.sep_entries);
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
		a2dpIsConnecting() ||
		!theHeadset.features.UseA2DPprofile)
	{
		A2DP_CONNECTION_DEBUG(("A2DP_CONNECTION: Invalid state\n"));
		return FALSE;
	}
	
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
bool a2dpGetListNextA2dpSource(uint16 *current_index, bdaddr *addr, uint8 *seid)
{
    uint8 lAttributes[ATTRIBUTE_SIZE];
    uint16 i;

    for (i=*current_index; i<MAX_PAIRED_DEVICES; i++)
    {
        *current_index = i;
        if (ConnectionSmGetIndexedAttributeNow(PSKEY_ATTRIBUTE_BASE, i, ATTRIBUTE_SIZE, lAttributes, addr))
        {
            if (lAttributes[attribute_a2dp_profile] || !lAttributes[attribute_hfp_hsp_profile])
            {
                *seid = SBC_SINK_SEID;
                return TRUE;
            }
        }
    }
    return FALSE;
}


/****************************************************************************/
bool a2dpListConnection(void)
{
    bdaddr addr;
	uint8 seid = SBC_SINK_SEID;
	uint8 seids[2];
	uint16 size_seids = 1;
	uint16 index;
	
	A2DP_CONNECTION_DEBUG(("A2DP_CONNECTION: List Connection\n"));
	
	if ((stateManagerGetA2dpState() != headsetA2dpConnectable) ||
		a2dpIsConnecting() || 
		!theHeadset.features.UseA2DPprofile)
	{
		A2DP_CONNECTION_DEBUG(("	Invalid state\n"));
		return FALSE;
	}
	
	if (theHeadset.a2dp_list_index == 0xf)
		theHeadset.a2dp_list_index = 0;
	else
		theHeadset.a2dp_list_index++;
	
	index = theHeadset.a2dp_list_index;
	
	A2DP_CONNECTION_DEBUG(("	Index:%d\n",index)) ; 
    
	if (!a2dpGetListNextA2dpSource(&index, &addr, &seid))
	{
		A2DP_CONNECTION_DEBUG(("	End of list\n"));
		theHeadset.a2dp_list_index = 0xf;
		theHeadset.a2dpConnecting = FALSE;
		theHeadset.sendPlayOnConnection = FALSE;
		MessageSend(&theHeadset.task, EventA2dpReconnectFailed, 0);
		if (theHeadset.switch_a2dp_source)
		{
			/* If this was switch source, try to connect back to original device */
			index = 0;
			if (a2dpGetListNextA2dpSource(&index, &addr, &seid) && (index == 0));
			{
				A2DP_CONNECTION_DEBUG(("	Connect to original device\n"));
				theHeadset.switch_a2dp_source = FALSE;
				goto list_continue;
			}
		}		
		theHeadset.switch_a2dp_source = FALSE;
		return FALSE;
	}
	
	theHeadset.a2dp_list_index = index;
	
list_continue:
    theHeadset.a2dpConnecting = TRUE;
	theHeadset.sendPlayOnConnection = TRUE;
		
	if (!A2dpGetMediaSink(theHeadset.a2dp))
	{
		theHeadset.manualA2dpConnect = TRUE;
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


