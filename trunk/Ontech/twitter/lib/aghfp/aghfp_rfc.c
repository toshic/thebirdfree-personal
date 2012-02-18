/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1
*/


#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_common.h"
#include "aghfp_rfc.h"
#include "aghfp_init.h"
#include "aghfp_receive_data.h"
#include "aghfp_slc_handler.h"
#include "aghfp_call_manager.h"

#include <panic.h>


/*lint -e525 -e830 */

/* Convert from the rfcomm_connect_status returned by the connection lib. */
static aghfp_connect_status convertRfcommConnectStatus(rfcomm_connect_status status)
{
    switch (status)
    {
    case rfcomm_connect_success:
        return aghfp_connect_success;

    case rfcomm_connect_failed:
        return aghfp_connect_failed;

    case rfcomm_server_channel_not_registered:
        return aghfp_connect_server_channel_not_registered;

    case rfcomm_connect_timeout:
        return aghfp_connect_timeout;

    case rfcomm_connect_rejected:
        return aghfp_connect_rejected;

    case rfcomm_normal_disconnect:
        return aghfp_connect_normal_disconnect;

    case rfcomm_abnormal_disconnect:
        return aghfp_connect_abnormal_disconnect;

    case rfcomm_connect_key_missing:
        return aghfp_connect_key_missing;
        
    default:
        /* All rfcomm disconnects should be handled above if we get this panic in debug lib */
        AGHFP_DEBUG_PANIC(("Unhandled rfc connect status 0x%x\n", status));

		/* Return generic connect fail in "release" lib variant */
        return aghfp_connect_failed;
    }
}


/* Convert from the rfcomm_disconnect_status returned by the connection lib. */
static aghfp_disconnect_status convertRfcommDisconnectStatus(rfcomm_disconnect_status status)
{
	switch (status)
	{
		case rfcomm_disconnect_success:
		case rfcomm_disconnect_normal_disconnect:
			return aghfp_disconnect_success;

		case rfcomm_disconnect_abnormal_disconnect:
			return aghfp_disconnect_link_loss;

		case rfcomm_disconnect_connection_timeout:
			return aghfp_disconnect_timeout;

		default:
			return aghfp_disconnect_error;
	}
}


/****************************************************************************
	Request an RFCOMM channel to be allocated.
*/
void aghfpHandleRfcommAllocateChannel(AGHFP *aghfp)
{
	/* Make the call to the connection lib */
	ConnectionRfcommAllocateChannel(&aghfp->task);
}


/****************************************************************************
 Rfcomm channel has been allocated.
*/
void aghfpHandleRfcommRegisterCfm(AGHFP *aghfp, const CL_RFCOMM_REGISTER_CFM_T *cfm)
{
	/* Check the result code */
	if (cfm->status == success)
	{
		/* Rfcomm channel allocation succeeded send an init cfm */
		aghfpSendInternalInitCfm(&aghfp->task, aghfp_init_success, cfm->server_channel);
	}
	else
	{
		/* Send an init cfm with error code indicating channel alloc failed */
		aghfpSendInternalInitCfm(&aghfp->task, aghfp_init_rfc_chan_fail, 0);
	}
}


/****************************************************************************
 Issue a request to the connection lib to create an RFCOMM connection.
*/
void aghfpHandleRfcommConnectRequest(AGHFP *aghfp, const AGHFP_INTERNAL_RFCOMM_CONNECT_REQ_T *req)
{
	if(!(aghfp->rfcomm_lock))
	{
		aghfp->rfcomm_lock = TRUE;
    	/* Issue the connect request to the connection lib. */
   		ConnectionRfcommConnectRequest(&aghfp->task, &req->addr, aghfp->local_rfc_server_channel, req->rfc_channel, 0);
	}
}


/****************************************************************************
 Outcome of the RFCOMM connect request or response.
*/
void aghfpHandleRfcommConnectCfm(AGHFP *aghfp, const CL_RFCOMM_CONNECT_CFM_T *cfm)
{
    /* Check the status of the rfcomm connect cfm */
    if (cfm->status == rfcomm_connect_success)
    {
  		/* Store the sink */
  		aghfp->rfcomm_sink = cfm->sink;

        /* RFCOMM connection is up! Check which profile is supported by this task */
        if (supportedProfileIsHsp(aghfp->supported_profile))
        {
            /* HSP supported - SLC is up so tell the app */
   			aghfpSendSlcConnectCfmToApp(aghfp_connect_success, aghfp);
        }
        else if (supportedProfileIsHfp(aghfp->supported_profile))
        {
            /* HFP supported - RFCOMM is up, so just wait for HF to send us some AT commends */
        }
        else
        {
            /* This should never happen */
            AGHFP_DEBUG_PANIC(("Unhandled profile type 0x%x\n", aghfp->supported_profile));
        }

  		/* Check for data in the buffer */
  		aghfpHandleReceivedData(aghfp, StreamSourceFromSink(cfm->sink));
    }
    else
    {
        /* RFCOMM connect failed - Tell the app. */
        aghfpSendSlcConnectCfmToApp(convertRfcommConnectStatus(cfm->status), aghfp);
    }
	
	aghfp->rfcomm_lock = FALSE;
}


/****************************************************************************
	Response to an incoming RFCOMM connect request.
*/
void aghfpHandleRfcommConnectResponse(AGHFP *aghfp, bool response, const bdaddr *addr, uint8 server_channel, const rfcomm_config_params *config)
{
	/* Issue a reject without passing this up to the app */
	ConnectionRfcommConnectResponse(&aghfp->task, response, addr, server_channel, config);
}


/****************************************************************************
	Notification of an incoming rfcomm connection, pass this up to the app
	to decide whether to accept this or not.
*/
void aghfpHandleRfcommConnectInd(AGHFP *aghfp, const CL_RFCOMM_CONNECT_IND_T *ind)
{
	/* Ask the app whether to accept this connection or not */
	MAKE_AGHFP_MESSAGE(AGHFP_SLC_CONNECT_IND);
	message->bd_addr = ind->bd_addr;
	message->aghfp = aghfp;
	MessageSend(aghfp->client_task, AGHFP_SLC_CONNECT_IND, message);

	/* Update the local state to indicate we're in the middle of connecting. */
	aghfpSetState(aghfp, aghfp_slc_connecting);
}


/****************************************************************************
	Issue an RFCOMM disconnect to the connection lib.
*/
void aghfpHandleRfcommDisconnectRequest(AGHFP *aghfp)
{
	/* Request the connection lib disconnects the RFCOMM connection */
	ConnectionRfcommDisconnectRequest(&aghfp->task, aghfp->rfcomm_sink);
}


/****************************************************************************
	Indication that the RFCOMM connection has been disconnected.
*/
void aghfpHandleRfcommDisconnectInd(AGHFP *aghfp, const CL_RFCOMM_DISCONNECT_IND_T *ind)
{
	if ( aghfpCallManagerActive(aghfp) )
	{
		/* Inform Call Manager */
		aghfpManageCall(aghfp, CallEventSlcRemoved, aghfpConvertDisconnectStatusToCallFlag(convertRfcommDisconnectStatus(ind->status)));
	}
	else
	{
		/* Convert the rfc disconnect status into its hfp counterpart and send to app */
		aghfpSendSlcDisconnectIndToApp(aghfp, convertRfcommDisconnectStatus(ind->status));
	}
}

/*lint +e525 +e830 */
