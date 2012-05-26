/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_rfc.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_rfc.h"
#include "init.h"
#include "hfp_receive_data.h"
#include "hfp_slc_handler.h"
#include "hfp_sdp.h"

#include <panic.h>


/*lint -e525 -e830 */


/* Convert from the rfcomm_connect_status returned by the connection lib. */
static hfp_connect_status convertRfcommConnectStatus(rfcomm_connect_status status)
{
    switch (status)
    {
    case rfcomm_connect_success:
        return hfp_connect_success;

    case rfcomm_connect_failed:
        return hfp_connect_failed;

    case rfcomm_server_channel_not_registered:
        return hfp_connect_server_channel_not_registered;

    case rfcomm_connect_timeout:
        return hfp_connect_timeout;

    case rfcomm_connect_rejected:
        return hfp_connect_rejected;

    case rfcomm_normal_disconnect:
        return hfp_connect_normal_disconnect;

    case rfcomm_abnormal_disconnect:
        return hfp_connect_abnormal_disconnect;

    default:
        /* All rfcomm disconnects should be handled above if we get this panic in debug lib */
        HFP_DEBUG(("Unhandled rfc connect status 0x%x\n", status));

		/* Return generic connect fail in "release" lib variant */
        return hfp_connect_failed;
    }
}


/* Convert from the rfcomm_disconnect_status returned by the connection lib. */
static hfp_disconnect_status convertRfcommDisconnectStatus(rfcomm_disconnect_status status)
{
	switch (status)
	{
	case rfcomm_disconnect_success:
	case rfcomm_disconnect_normal_disconnect:
		return hfp_disconnect_success;

	case rfcomm_disconnect_abnormal_disconnect:
		return hfp_disconnect_link_loss;

	case rfcomm_disconnect_connection_timeout:
		return hfp_disconnect_timeout;

	case rfcomm_disconnect_connection_rej_psm:
	case rfcomm_disconnect_connection_rej_security:
	case rfcomm_disconnect_connection_rej_resources:
    case rfcomm_disconnect_connection_rej_not_ready:
    case rfcomm_disconnect_connection_failed:
    case rfcomm_disconnect_connection_pending:
    case rfcomm_disconnect_config_unacceptable:
    case rfcomm_disconnect_config_rejected:
    case rfcomm_disconnect_config_unknown:
    case rfcomm_disconnect_config_rejected_locally:
    case rfcomm_disconnect_config_timeout:
    case rfcomm_disconnect_remote_refusal:
    case rfcomm_disconnect_race_condition_detected:
    case rfcomm_disconnect_insufficient_resources:
    case rfcomm_disconnect_cannot_change_flow_control_mechanism:
    case rfcomm_disconnect_dlc_already_exists:
    case rfcomm_disconnect_dlc_rej_security:
    case rfcomm_disconnect_generic_refusal:
    case rfcomm_disconnect_unexpected_primitive:
    case rfcomm_disconnect_invalid_server_channel:
    case rfcomm_disconnect_unknown_mux_id:
    case rfcomm_disconnect_local_entity_terminated_connection:
    case rfcomm_disconnect_unknown_primitive:
    case rfcomm_disconnect_max_payload_exceeded:
    case rfcomm_disconnect_inconsistent_parameters:
    case rfcomm_disconnect_insufficient_credits:
    case rfcomm_disconnect_credit_flow_control_protocol_violation:
    case rfcomm_disconnect_mux_already_open:
    case rfcomm_disconnect_res_ack_timeout:
	case rfcomm_disconnect_unknown:
	default:
		return hfp_disconnect_error;
	}
}


/****************************************************************************
NAME	
	hfpHandleRfcommAllocateChannel

DESCRIPTION
	Request an RFCOMM channel to be allocated.

RETURNS
	void
*/
void hfpHandleRfcommAllocateChannel(HFP *hfp)
{
	/* Make the call to the connection lib */
	ConnectionRfcommAllocateChannel(&hfp->task);
}


/****************************************************************************
NAME	
	hfpHandleRfcommRegisterCfm

DESCRIPTION
	Rfcomm channel has been allocated.

RETURNS
	void
*/
void hfpHandleRfcommRegisterCfm(HFP *hfp, const CL_RFCOMM_REGISTER_CFM_T *cfm)
{
	/* Check the result code */
	if (cfm->status == success)
		/* Rfcomm channel allocation succeeded send an init cfm */		
		hfpSendInternalInitCfm(&hfp->task, hfp_init_success, cfm->server_channel);
	else
		/* Send an init cfm with error code indicating channel alloc failed */
		hfpSendInternalInitCfm(&hfp->task, hfp_init_rfc_chan_fail, 0);
}


/****************************************************************************
NAME	
	hfpHandleRfcommConnectRequest

DESCRIPTION
	Issue a request to the connection lib to create an RFCOMM connection.

RETURNS
	void
*/
void hfpHandleRfcommConnectRequest(HFP *hfp, const HFP_INTERNAL_RFCOMM_CONNECT_REQ_T *req)
{
	if(!(hfp->rfcomm_lock))
	{
		hfp->rfcomm_lock = TRUE;
    	/* Issue the connect request to the connection lib. */
    	ConnectionRfcommConnectRequest(&hfp->task, &req->addr, hfp->local_rfc_server_channel, req->rfc_channel, 0);
	}
}


/****************************************************************************
NAME	
	hfpHandleRfcommConnectCfm

DESCRIPTION
	Outcome of the RFCOMM connect request or response.

RETURNS
	void
*/
void hfpHandleRfcommConnectCfm(HFP *hfp, const CL_RFCOMM_CONNECT_CFM_T *cfm)
{
    /* Check the status of the rfcomm connect cfm */
    if (cfm->status == rfcomm_connect_success)
    {
		/* Store the sink */
		hfp->sink = cfm->sink;

        /* RFCOMM connection is up! Check which profile is supported by this task */
        if (supportedProfileIsHsp(hfp->hfpSupportedProfile))
        {
	        hfp->agSupportedProfile = hfp_headset_profile;
	        
            /* HSP supported - SLC is up so tell the app */
            hfpSendSlcConnectCfmToApp(hfp_connect_success, hfp);
        }
        else if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
        {
            /* Initiate SLC establishment */
			MessageSend(&hfp->task, HFP_INTERNAL_AT_BRSF_REQ, 0);
        }
        else
        {
            /* This should never happen */
            HFP_DEBUG(("Unhandled profile type 0x%x\n", hfp->hfpSupportedProfile));
        }

		/* Check for data in the buffer */
		hfpHandleReceivedData(hfp, StreamSourceFromSink(cfm->sink));
    }
    else
    {
        /* RFCOMM connect failed - Tell the app. */
        hfpSendSlcConnectCfmToApp(convertRfcommConnectStatus(cfm->status), hfp);
    }
	
	/* Free the rfcomm lock */
	hfp->rfcomm_lock = FALSE;
}


/****************************************************************************
NAME	
	hfpHandleRfcommConnectResponse

DESCRIPTION
	Response to an incoming RFCOMM connect request.

RETURNS
	void
*/
void hfpHandleRfcommConnectResponse(HFP *hfp, bool response, const bdaddr *addr, uint8 server_channel, const rfcomm_config_params *config)
{
	/* Issue a reject without passing this up to the app */
	ConnectionRfcommConnectResponse(&hfp->task, response, addr, server_channel, config);
}


/****************************************************************************
NAME	
	hfpHandleRfcommConnectInd

DESCRIPTION
	Notification of an incoming rfcomm connection, pass this up to the app
	to decide whather to accept this or not.

RETURNS
	void
*/
void hfpHandleRfcommConnectInd(HFP *hfp, const CL_RFCOMM_CONNECT_IND_T *ind)
{
	/* Ask the app whather to accept this connection or not */
	MAKE_HFP_MESSAGE(HFP_SLC_CONNECT_IND);
	message->addr = ind->bd_addr;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_SLC_CONNECT_IND, message);

	/* Update the local state to indicate we're in the middle of connecting. */
	hfpSetState(hfp, hfpSlcConnecting);
}


/****************************************************************************
NAME	
	hfpHandleRfcommDisconnectRequest

DESCRIPTION
	Issue an RFCOMM disconnect to the connection lib.

RETURNS
	void
*/
void hfpHandleRfcommDisconnectRequest(HFP *hfp)
{
	/* Request the connection lib disconnects the RFCOMM connection */
	ConnectionRfcommDisconnectRequest(&hfp->task, hfp->sink);
}


/****************************************************************************
NAME	
	hfpHandleRfcommDisconnectInd

DESCRIPTION
	Indication that the RFCOMM connection has been disconnected.

RETURNS
	void
*/
void hfpHandleRfcommDisconnectInd(HFP *hfp, const CL_RFCOMM_DISCONNECT_IND_T *ind)
{
	if (hfp->state == hfpSlcConnecting)
	{   /* Rfcomm connection has been shutdown during the SLC connection process */
    	/* Cancel the AT response timeout message because we'll have no more AT cmds being sent */
    	(void) MessageCancelAll(&hfp->task, HFP_INTERNAL_WAIT_AT_TIMEOUT_IND);
    	
	    /* Report a failed connect attempt to app */
		hfpSendSlcConnectCfmToApp(hfp_connect_slc_failed, hfp);
	}
	else
	{   /* Convert the rfc disconnect status into its hfp counterpart and inform library */
    	MAKE_HFP_MESSAGE(HFP_INTERNAL_SLC_DISCONNECT_IND);
    	message->status = convertRfcommDisconnectStatus(ind->status);
    	MessageSend(&hfp->task, HFP_INTERNAL_SLC_DISCONNECT_IND, message);
    }
}


/****************************************************************************/
void hfpHandleRfcommControlInd(HFP *hfp, const CL_RFCOMM_CONTROL_IND_T *ind)
{
	ConnectionRfcommControlSignalRequest(&hfp->task, ind->sink, ind->break_signal, ind->modem_status);
}


/*lint +e525 +e830 */
