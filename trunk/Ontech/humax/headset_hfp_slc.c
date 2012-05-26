/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_hfp_slc.c
@brief    Handle HFP SLC.
*/

#include "headset_a2dp_connection.h"
#include "headset_a2dp_stream_control.h"
#include "headset_configmanager.h"
#include "headset_debug.h"
#include "headset_hfp_call.h"
#include "headset_hfp_slc.h"
#include "headset_inquiry.h"
#include "headset_statemanager.h"
#include "headset_volume.h"

#include <bdaddr.h>
#include <hfp.h>
#include <panic.h>
#include <ps.h>
#include <stdlib.h>


#ifdef DEBUG_HFP_SLC
#define HFP_SLC_DEBUG(x) DEBUG(x)
#else
#define HFP_SLC_DEBUG(x) 
#endif


/****************************************************************************
  FUNCTIONS
*/


/****************************************************************************/
bool hfpSlcIsConnecting (void)
{
	return theHeadset.slcConnecting;	
}


/*****************************************************************************/
void hfpSlcConnectSuccess ( HFP * pProfile, Sink sink )
{
    bdaddr ag_addr;
	headsetHfpState hfpState;
    
	HFP_SLC_DEBUG(("HFP: Connected[%x]\n", (uint16)sink));
    
    theHeadset.slcConnecting = FALSE;
	theHeadset.LinkLossAttemptHfp = 0;
	theHeadset.hfp_list_index = 0xf;
	
	/* Enter connected state if not already connected */
	if (!stateManagerIsHfpConnected())
	{
	    stateManagerEnterHfpConnectedState();	
	}
	
	hfpState = stateManagerGetHfpState();
	
	/* Resume A2DP streaming if any existed before connection attempt */
	streamControlResumeA2dpStreaming(0);
	
	/* check to see if the call should be answered now we are connected */
    if ((theHeadset.features.AutoAnswerOnConnect) && (theHeadset.incoming_call_power_up))
    {
    	/* reset the answer call flag */
        theHeadset.incoming_call_power_up = FALSE;
            
        /*then attempt to answer the call*/
        MessageSend(&theHeadset.task ,EventAnswer ,0) ;
    }
 
	if (pProfile == theHeadset.hfp)
    {
        theHeadset.profile_connected = hfp_handsfree_profile;
		theHeadset.hfp_hsp = theHeadset.hfp;
    }
    else if (pProfile == theHeadset.hsp)
    {
        theHeadset.profile_connected = hfp_headset_profile;
		theHeadset.hfp_hsp = theHeadset.hsp;
    }    
    
    if (SinkGetBdAddr(sink, &ag_addr))
	{
		uint8 lAttributes[ATTRIBUTE_SIZE];

		/* Retrieve attributes for this device */
    	if (ConnectionSmGetAttributeNow(PSKEY_ATTRIBUTE_BASE, &ag_addr, ATTRIBUTE_SIZE, lAttributes))
		{
			if (lAttributes[attribute_hfp_hsp_profile])	
			{
				theHeadset.gHfpVolumeLevel = lAttributes[attribute_hfp_volume];
			}
			else
			{
				theHeadset.gHfpVolumeLevel = theHeadset.config->gVolLevels.defaultHfpVolLevel;
			}
			HFP_SLC_DEBUG(("HFP: Read HFP attributes [%d][%d][%d][%d][%d][%d]\n",lAttributes[0],
				   lAttributes[1],lAttributes[2],lAttributes[3],lAttributes[4],lAttributes[5])) ;
		}
		else
		{
			theHeadset.gHfpVolumeLevel = theHeadset.config->gVolLevels.defaultHfpVolLevel;
		}
    	/* always shuffle the pdl into mru order */        
    	ConnectionSmUpdateMruDevice(&ag_addr) ;
		/* Store last used AG */
		theHeadset.LastDevices->lastHfpConnected = ag_addr;
		theHeadset.LastDevices->lastHfpHsp = ATTRIBUTE_GET_HF_PROFILE(theHeadset.profile_connected);
	}
	
	/* Send retrieved volumes to AG */
	VolumeSendSettingsToAG(TRUE, TRUE);
    
    /* Ensure the underlying ACL is encrypted */       
    ConnectionSmEncrypt( &theHeadset.task , sink , TRUE );
	
	/* Disable NR/EC on AG if it's supported. This is called only if CVC is used on the headset. */
    if ((theHeadset.profile_connected == hfp_handsfree_profile) && theHeadset.features.audio_plugin)
        HfpDisableNrEc(theHeadset.hfp);
    
    /* Send a user event to the app for indication purposes */
    MessageSend ( &theHeadset.task , EventSLCConnected , 0 );
	
	/*attempt to pull the audio across if not already present, delay by 5 seconds
    to prevent a race condition occuring with certain phones  */
	if ((hfpState == headsetIncomingCallEstablish) || ((hfpState == headsetActiveCall) && !HfpGetAudioSink(theHeadset.hfp_hsp)))
	    MessageSendLater ( &theHeadset.task , APP_CHECK_FOR_AUDIO_TRANSFER , 0 , 5000 ) ;
	
	/* Send an event to connect to last used A2DP source if this connection was from power on, or inquiry */
	if (theHeadset.slcConnectFromPowerOn || theHeadset.inquiry_data)
	{	
		/* Only connect A2DP to a combined device if no call is ongoing */
		if (hfpState == headsetHfpConnected)
		{
			/* Delay connecting A2DP incase an AG tries to connect to headset first */
			APP_CONNECT_A2DP_T *message = (APP_CONNECT_A2DP_T*)PanicUnlessMalloc(sizeof(APP_CONNECT_A2DP_T));
			message->a2dp_ag_connect_signalling_only = TRUE;
			MessageSendLater(&theHeadset.task, APP_CONNECT_A2DP, message, 1500);
		}
		else
		{
			theHeadset.connect_a2dp_when_no_call = TRUE;			
		}
	}
	theHeadset.slcConnectFromPowerOn = FALSE;
	
	/* Stop any pending inquiry now */
    inquiryStop();
	
	PROFILE_MEMORY(("HFPConnect"))
}


/*****************************************************************************/
bool hfpSlcConnectFail( void )
{
    /* Update the app state */  
    HFP_SLC_DEBUG(("SLC : Connect Fail \n")); 
	
	/* Check if this was reconnection from link loss */
	if (theHeadset.LinkLossAttemptHfp && (++theHeadset.LinkLossAttemptHfp <= theHeadset.features.LinkLossRetries))
	{
		/* Still reconnection attempts after link loss to try */
		HFP_SLC_DEBUG(("SLC : Link loss connection attempt %d \n",theHeadset.LinkLossAttemptHfp)); 
		return FALSE;
	}
	else
	{
		/* No link loss reconnection */
		theHeadset.LinkLossAttemptHfp = 0;
		
		if (theHeadset.hfp_list_index != 0xf)
		{
			/* Attempt to connect to next HFP device in list */	
			MessageSendLater(&theHeadset.task, APP_CONTINUE_HFP_LIST_CONNECTION , 0, 0);
			return TRUE;
		}
	}
		
	/* Connect to last used A2DP source if flagged to do so. */
	if (theHeadset.slcConnectFromPowerOn)
	{
		a2dpEstablishConnection(TRUE, FALSE);
	}

    theHeadset.slcConnecting = FALSE; /*reset the connecting flag*/ 
	theHeadset.slcConnectFromPowerOn = FALSE;
	
	/* Resume A2DP streaming if any existed before connection attempt */
	streamControlResumeA2dpStreaming(0);

    /* No connection */
    theHeadset.profile_connected = hfp_no_profile;
    
    /* Send event to signify that all reconnection attempts failed */
	MessageSend(&theHeadset.task, EventHfpReconnectFailed, 0);
    
    /* Clear the queue */
    hfpCallClearQueuedEvent() ;
    
    PROFILE_MEMORY(("HFPConnectFail"))
			
	return TRUE;
}


/****************************************************************************/
bool hfpSlcLastConnectRequest( hfp_profile pProfile )
{
    bdaddr lLastAddr ;   
	uint16 profile = attribute_hf_hfp_profile;;
	
	if (hfpSlcIsConnecting () ||		
		(stateManagerGetHfpState() == headsetPoweringOn))
	{
		HFP_SLC_DEBUG(("SLC: Invalid state for connection\n"));
		return FALSE;
	}
	
    /* Retrieve the address of the last used AG from PS */
	if (!hfpSlcGetLastUsedAG(&lLastAddr, &profile) || !theHeadset.features.UseHFPprofile)
	{
		/* We have failed to connect as we don't have a valid address */
        HFP_SLC_DEBUG(("SLC: No Last Addr \n"));
		hfpSlcConnectFail();
		return FALSE;
	}
	
	theHeadset.slcConnecting = TRUE; 
	
	/* If HandsFree profile was requested then use the last profile stored (HFP or HSP),
 		otherwise use HSP as requested.
	*/
	if ((pProfile != hfp_headset_profile) && (profile == attribute_hf_hsp_profile))
	{
		pProfile = hfp_headset_profile;
	}
	
    HFP_SLC_DEBUG(("SLC: Connect Req [%x]\n" , pProfile));
    hfpSlcAttemptConnect ( pProfile , &lLastAddr );	
	
	return TRUE;
}


/****************************************************************************/
bool hfpSlcConnectBdaddrRequest( hfp_profile pProfile, bdaddr * pAddr )
{
	if (hfpSlcIsConnecting () ||		
		(stateManagerGetHfpState() == headsetPoweringOn))
	{
		HFP_SLC_DEBUG(("SLC: Invalid state for connection\n"));
		return FALSE;
	}
	
	theHeadset.slcConnecting = TRUE; 
	
    HFP_SLC_DEBUG(("SLC: Connect Bdaddr Req [%x]\n" , pProfile));
    hfpSlcAttemptConnect ( pProfile , pAddr );	
	
	return TRUE;
}


/****************************************************************************/
void hfpSlcAttemptConnect( hfp_profile pProfile , bdaddr * pAddr )
{
	
	/* Pause A2DP streaming if any */
	streamControlCeaseA2dpStreaming(TRUE);
	
    switch ( pProfile )
    {
        case (hfp_handsfree_profile) :
        {
            theHeadset.profile_connected = hfp_handsfree_profile;
           
            HFP_SLC_DEBUG(("SLC: Attempt HFP\n")) ;                    
            /* Issue a connect request for HFP */
            HfpSlcConnect(theHeadset.hfp, pAddr, 0);
        }    
        break;
        case (hfp_headset_profile) :
        {
            theHeadset.profile_connected = hfp_headset_profile;
            HFP_SLC_DEBUG(("SLC: Attempt HSP\n")) ;                    
            /* Issue a connect request for HFP */
            HfpSlcConnect(theHeadset.hsp, pAddr, 0);
        }   
        break;
        default:
            Panic();
            break;
    }
}


/****************************************************************************/
void hfpSlcDisconnect(void)
{
    /* Issue the disconnect request and let the HFP lib do the rest */
	if (theHeadset.hfp_hsp)
	    HfpSlcDisconnect(theHeadset.hfp_hsp);
}


/****************************************************************************/
bool hfpSlcGetLastUsedAG(bdaddr *addr, uint16 *profile)
{
	if (!BdaddrIsZero(&theHeadset.LastDevices->lastHfpConnected))
	{
		*addr = theHeadset.LastDevices->lastHfpConnected;
		*profile = theHeadset.LastDevices->lastHfpHsp;
	}
	else if (!BdaddrIsZero(&theHeadset.LastDevices->lastPaired))
	{
		*addr = theHeadset.LastDevices->lastPaired;
		*profile = attribute_hf_hfp_profile;
	}
	else
	{
		return FALSE;
	}
	
	return TRUE;
}


/****************************************************************************/
bool hfpSlcGetLastConnectedAG(bdaddr *addr)
{
	if (!BdaddrIsZero(&theHeadset.LastDevices->lastHfpConnected))
	{
		*addr = theHeadset.LastDevices->lastHfpConnected;
		return TRUE;
	}
	
	return FALSE;
}


/****************************************************************************/
bool hfpSlcGetListNextAG(uint16 *current_index, bdaddr *addr, uint16 *profile)
{
	uint8 lAttributes[ATTRIBUTE_SIZE];
	uint16 i;
	
	for (i=*current_index; i<MAX_PAIRED_DEVICES; i++)
	{
		if (ConnectionSmGetIndexedAttributeNow(PSKEY_ATTRIBUTE_BASE, i, ATTRIBUTE_SIZE, lAttributes, addr))
		{
			if (lAttributes[attribute_hfp_hsp_profile] || !lAttributes[attribute_a2dp_profile])
			{
				*current_index = i;
				*profile = lAttributes[attribute_hfp_hsp_profile];
				return TRUE;
			}
		}
		else
		{			
			break;
		}
	}
	return FALSE;
}


/****************************************************************************/
bool hfpSlcListConnection(void)
{
	bdaddr addr;
	uint16 index;
	uint16 profile = attribute_hf_hfp_profile;
	
	HFP_SLC_DEBUG(("SLC: List connection\n")) ;    
	
	if (hfpSlcIsConnecting () ||		
		((stateManagerGetHfpState() != headsetHfpConnectable) && (stateManagerGetHfpState() != headsetConnDiscoverable)) ||
		!theHeadset.features.UseHFPprofile)
	{
		HFP_SLC_DEBUG(("	Invalid State\n")) ; 
		return FALSE;
	}
	
	if (theHeadset.hfp_list_index == 0xf)
		theHeadset.hfp_list_index = 0;
	else
		theHeadset.hfp_list_index++;
	
	index = theHeadset.hfp_list_index;
	
	HFP_SLC_DEBUG(("	Index:%d\n",index)) ; 
	
	if (!hfpSlcGetListNextAG(&index, &addr, &profile))
	{
		theHeadset.hfp_list_index = 0xf;
		HFP_SLC_DEBUG(("	End of list\n")) ;
		hfpSlcConnectFail();
		return FALSE;
	}
	
	theHeadset.hfp_list_index = index;
	
    theHeadset.slcConnecting = TRUE;  
	
	if (profile == attribute_hf_hsp_profile)
		hfpSlcAttemptConnect(hfp_headset_profile, &addr);	
	else
		hfpSlcAttemptConnect(hfp_handsfree_profile, &addr);	
	
	return TRUE;
}



