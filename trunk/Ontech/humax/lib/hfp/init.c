/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	init.c        

DESCRIPTION
	This file contains the initialisation code for the Hfp profile library.

NOTES

*/
/*lint -e655 */

/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_rfc.h"
#include "hfp_sdp.h"
#include "init.h"
#include "profile_handler.h"

#include <panic.h>

/* Send an init cfm message to the application */
static void sendInitCfmToApp(hfp_init_status status, HFP *hfp)
{
	MAKE_HFP_MESSAGE(HFP_INIT_CFM);
	message->status = status;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_INIT_CFM, message);

	/* If the initialisation failed, free the allocated task */
	if (status != hfp_init_success)
		free(hfp);
}


/****************************************************************************
NAME	
	hfpResetConnectionRelatedState

DESCRIPTION
	Reset all the connection related state in this function.

RETURNS
	Task
*/
void hfpResetConnectionRelatedState(HFP *hfp)
{
	/* Reset the AG's supported features to default vals as specified in the HFP spec */
	hfp->agSupportedFeatures = (AG_THREE_WAY_CALLING | AG_IN_BAND_RING);

	/* Init the SLC sink */
	hfp->sink = 0;

	/* Init the flag indicating which AT cmd we're waiting on for a response */
	hfp->at_cmd_resp_pending = hfpNoCmdPending;

	/* Init the indicator status - set the indices to zero as that is not a valid value */
	hfp->indicator_status.indexes.service = 0;
	hfp->indicator_status.indexes.call = 0;
	hfp->indicator_status.indexes.call_setup = 0;
	hfp->indicator_status.indexes.signal_strength = 0;
	hfp->indicator_status.indexes.roaming_status = 0;
	hfp->indicator_status.indexes.battery_charge = 0;
	hfp->indicator_status.indexes.call_hold_status = 0;
	hfp->indicator_status.call = 0;

	if (hfp->indicator_status.extra_indicators)
		free(hfp->indicator_status.extra_indicators);

	hfp->indicator_status.extra_indicators = 0;	
	hfp->indicator_status.extra_inds_enabled = 0;

	/* Reset the audio sink */
	hfp->audio_sink = 0;
	hfp->audio_connection_state = hfp_audio_disconnected;

	/* Reset the volume cmd pending flag */
	hfp->vol_setting = 0xff;
    
    /* CSR2CSR protocol initialised on a SLC by SLC basis */
    hfp->use_csr2csr = FALSE;
}


/****************************************************************************
NAME	
	HfpInit	

DESCRIPTION
	This function initialises an instance of the Hfp library. The 
	application registers its own task, theAppTask, with the Hfp library so
	that return messages can be routed to the correct task. 
	
	The config parameter is used to configure the profile this Hfp instance 
	will support (HSP or HFP). If the HFP is being supported the supported 
	features should also be supplied. These are passed in as a bit mask, as 
	defined in the HFP specification. If the profile instance is being 
	configured as HSP the supported features should be set to zero as they 
	are not used.

MESSAGE RETURNED
	HFP_INIT_CFM

RETURNS
	void
*/
void HfpInit(Task theAppTask, const hfp_init_params *config)
{
	HFP *hfp = PanicUnlessNew(HFP);

	/* Check the app has passed in a valid pointer. */
	if (!config)
	{
		HFP_DEBUG(("Config parameters not passed in\n"));
	}
        
        
	else
	{
		/* Set the state to initialising */
		hfpSetState(hfp, hfpInitialising);

		/* Set the handler function */
		hfp->task.handler = hfpProfileHandler;

		/* Init the start up state */
		if (supportedProfileIsHfp(config->supported_profile))
		{
			/* HFP supported - store the supported features masking out all irrelevant bits */
			hfp->hfpSupportedProfile = config->supported_profile;            
			hfp->hfpSupportedFeatures = (config->supported_features & 0x1f);
		}
		else if (supportedProfileIsHsp(config->supported_profile))
		{
			/* HSP supported */
			hfp->hfpSupportedProfile = config->supported_profile;
			hfp->hfpSupportedFeatures = 0;
		}
		else
		{
			/* If the app has not indicated support for any valid profile proceed no further */
			HFP_DEBUG(("Profile support not registered\n"));
		}

		if (config->size_service_record)
        {
            hfp->size_service_record = config->size_service_record;
			hfp->service_record = config->service_record;
        }
		else
        {
            hfp->size_service_record = 0;
			hfp->service_record = 0;
        }

        hfp->indicator_status.extra_indicators = 0;	

		/* Connection related state updated in separate function */
		hfpResetConnectionRelatedState(hfp);

		/* Init the local server channel */
		hfp->local_rfc_server_channel = 0;

		/* Init the service record handle */
		hfp->sdp_record_handle = 0;

		/* Store the app task so we know where to return responses */
		hfp->clientTask = theAppTask;
		
		/* Initialise the rfcomm lock */
		hfp->rfcomm_lock = FALSE;

    /* CSR2CSR protocol initialised on a SLC by SLC basis */
        hfp->use_csr2csr = FALSE;

		                       
		/* Send an internal init message to kick off initialisation */
		MessageSend(&hfp->task, HFP_INTERNAL_INIT_REQ, 0x00);
	}
}



/****************************************************************************
NAME	
	hfpHandleInternalInitReq

DESCRIPTION
	Send internal init req messages until we have completed the profile
	lib initialisation.

RETURNS
	void
*/
void hfpHandleInternalInitReq(HFP *hfp)
{
	/* Get an rfcomm channel */
	hfpHandleRfcommAllocateChannel(hfp);
}


/****************************************************************************
NAME	
	hfpSendInternalInitCfm

DESCRIPTION
	Send an internal init cfm message.

RETURNS
	void
*/

void hfpSendInternalInitCfm(Task task, hfp_init_status	s, uint8 c)
{
	MAKE_HFP_MESSAGE(HFP_INTERNAL_INIT_CFM);
	message->status = s;
	message->rfcomm_channel = c;
	MessageSend(task, HFP_INTERNAL_INIT_CFM, message);
}


/****************************************************************************
NAME	
	hfpHandleInternalInitCfm

DESCRIPTION
	This message is sent once various parts of the library initialisation 
	process have completed.

RETURNS
	void
*/
void hfpHandleInternalInitCfm(HFP *hfp, const HFP_INTERNAL_INIT_CFM_T *cfm)
{
	/* The init stage may ahve failed check the result code */
	if (cfm->status != hfp_init_success)
	{
		/* Something has gone wrong, tell the app */
		sendInitCfmToApp(cfm->status, hfp);
	}
	else
	{
		if (!cfm->rfcomm_channel)
		{
			/* Update the local state to initialised */
			hfpSetState(hfp, hfpReady);

			/* Register our intention to use Synchronous connections */
			ConnectionSyncRegister(&hfp->task);
		}
		else
		{
			/* Store the local server channel */
			hfp->local_rfc_server_channel = cfm->rfcomm_channel;

			/* Rfcomm channel allocated register a service record for the profile */
			hfpRegisterServiceRecord(hfp);
		}
	}
}

/****************************************************************************
NAME	
	hfpHandleSyncRegisterCfm

DESCRIPTION
	Handles confirmation registering the HFP to receive Synchronous connection
	notifications from the Connection library.  This completes the HFP initialisation
	process - send message to app.

RETURNS
	void
*/
void hfpHandleSyncRegisterCfm(HFP *hfp)
{
    /* We have finished the profile init so send an init cfm to the app */
    sendInitCfmToApp(hfp_init_success, hfp);
}



