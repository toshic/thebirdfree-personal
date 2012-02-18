/* Copyright (C) Cambridge Silicon Radio Limited 2005-2009 */
/* Part of BlueLab 4.1.2-Release 

FILE NAME
	spp_connect_handler.c        

DESCRIPTION
    Internal hander functions for Spp connections.

*/
#include "spp_connect_handler.h"
#include "spp_common.h"
#include "spp_sdp.h"

#include <panic.h>

/*lint -e525 -e830 */


/* Spp service search request */
static const uint8 SppServiceRequest [] =
{
    0x35, 0x05,                     /* type = DataElSeq, 5 bytes in DataElSeq */
    0x1a, 0x00, 0x00, 0x11, 0x01    /* 4 byte UUID */
};

/* Spp protocol search request */
static const uint8 protocolAttributeRequest [] =
{
    0x35, 0x03,         /* type = DataElSeq, 3 bytes in DataElSeq */
    0x09, 0x00, 0x04    /* 2 byte UINT attrID ProtocolDescriptorList */    
};

/*****************************************************************************/
static spp_connect_status convertRfcommConnectStatus(rfcomm_connect_status status)
{
    /* Convert from the rfcomm_connect_status returned by the connection lib. */
    switch (status)
    {
    case rfcomm_connect_success:
        return spp_connect_success;

    case rfcomm_connect_failed: 
        return spp_connect_failed;

    case rfcomm_server_channel_not_registered:
        return spp_connect_server_channel_not_registered;

    case rfcomm_connect_timeout:
        return spp_connect_timeout;

    case rfcomm_connect_rejected:
        return spp_connect_rejected;

    case rfcomm_normal_disconnect:
        return spp_connect_normal_disconnect;

    case rfcomm_abnormal_disconnect:
        return spp_connect_abnormal_disconnect;

    case rfcomm_connect_channel_already_open:
        return spp_connect_rfcomm_channel_already_open;

    default:
        /* If we get an unghandled rfcomm status panic in debug lib */
        SPP_DEBUG(("Unhandled rfc connect status 0x%x\n", status));

		/* Return generic connect fail in "release" lib variant */
        return spp_connect_failed;
    }
}


/*****************************************************************************/
static spp_disconnect_status convertRfcommDisconnectStatusSpp(rfcomm_disconnect_status status)
{
    /* Convert from the rfcomm_disconnect_status returned by the connection lib. */
	switch (status)
	{
	case rfcomm_disconnect_success:
	case rfcomm_disconnect_normal_disconnect:
		return spp_disconnect_success;

	case rfcomm_disconnect_abnormal_disconnect:
		return spp_disconnect_link_loss;

	case rfcomm_disconnect_connection_timeout:
		return spp_disconnect_timeout;

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
		return spp_disconnect_error;
	}
}

/****************************************************************************

DESCRIPTION
	This function is called to initialise the RFCOMM configuration parameters
	Values hard coded to avoid exposing rfcomm prim
*/
static void initConfigParams(rfcomm_config_params *config)
{
    config->break_signal = 0;
    config->modem_status = 0;
    config->timeout = D_SEC(DEFAULT_RFCOMM_CONNECTION_TIMEOUT);
    config->request = TRUE;
    config->port_params.port_speed = 0xFF;
    config->port_params.data_bits = 0x03;
    config->port_params.stop_bits = 0x00;
    config->port_params.parity = 0x01;
    config->port_params.parity_type = 0x00;
    config->port_params.flow_ctrl_mask = 0;
    config->port_params.xon = 0x11;
    config->port_params.xoff = 0x13;
    config->port_params.parameter_mask = 0x3F7F;
}

/*****************************************************************************/
void sppHandleConnectRequest(SPP *spp, const SPP_INTERNAL_CONNECT_REQ_T *req)
{
    uint16 length;
    uint8 *ptr;
	
    /* If search pattern not supplied by client use default. */
    if (req->length_service_search_pattern)
    {
        length = req->length_service_search_pattern;
        ptr = (uint8 *) req->service_search_pattern;
    }
    else
    {
        length = sizeof(SppServiceRequest);
        ptr = (uint8 *) SppServiceRequest;
    }
	
    /* Update the local state to indicate we're in the middle of searching. */
    sppSetState(spp, sppSearching);
	
	/* Update the frame size being used. */
	spp->max_frame_size = req->max_frame_size;
	
	
    if(req->rfcomm_channel_number)
	{
		MAKE_SPP_MESSAGE(SPP_INTERNAL_RFCOMM_CONNECT_REQ);
        message->addr = req->addr;
        message->rfc_channel = req->rfcomm_channel_number;
        MessageSend(&spp->task, SPP_INTERNAL_RFCOMM_CONNECT_REQ, message);
            
        sppSetState(spp, sppConnecting);
	}
	else
	{
		/* Issue the Protocol Search Request. */
    	ConnectionSdpServiceSearchAttributeRequest(&spp->task, &req->addr, 0x40, length, ptr, sizeof(protocolAttributeRequest), protocolAttributeRequest);
	}
}


/*****************************************************************************/
void sppHandleInternalRfcommConnectRequest(SPP *spp, const SPP_INTERNAL_RFCOMM_CONNECT_REQ_T *message)
{
    SPP_INTERNAL_RFCOMM_CONNECT_REQ_T *req = (SPP_INTERNAL_RFCOMM_CONNECT_REQ_T *) message;
	rfcomm_config_params config;
	
	/* load default RFCOMM parameters */
	initConfigParams(&config);
	
	/* replace default frame size with user def */
	config.max_frame_size = spp->max_frame_size;
	
    /* Initiate an rfcomm connection. */
	ConnectionRfcommConnectRequest(&spp->task, &req->addr, spp->local_server_channel, req->rfc_channel, &config);
}


/*****************************************************************************/
void sppHandleConnectResponse(SPP *spp, const SPP_INTERNAL_CONNECT_RES_T *res)
{
	rfcomm_config_params config;
	
	/* load default RFCOMM parameters */
	initConfigParams(&config);
	
	/* replace default frame size with user def */
	spp->max_frame_size = res->max_frame_size;
	config.max_frame_size = spp->max_frame_size;
	
	/* Send response to the connection lib */
	ConnectionRfcommConnectResponse(&spp->task, res->response, &res->addr, spp->local_server_channel, &config);
    
    if (spp->lazy && !res->response)
    {
        /* If in lazy mode delete the task if we have just rejected the connection */
        MessageSend(&spp->task, SPP_INTERNAL_TASK_DELETE_REQ, 0);
    }
    else
    {
        /* If rejecting the connection reset the local state */
	    if (!res->response)
		    sppSetState(spp, sppReady);
    }
}


/*****************************************************************************/
void sppSendConnectCfmToApp(spp_connect_status status, SPP *spp)
{
    /* Send SPP_CONNECT_CFM message to the app. */	
	MAKE_SPP_MESSAGE(SPP_CONNECT_CFM);
	message->status = status;
   	message->sink = spp->sink;
	message->frame_size = spp->max_frame_size;
    message->spp = spp;
	MessageSend(spp->clientTask, SPP_CONNECT_CFM, message);
	
    if (status == spp_connect_success)
    {
        sppSetState(spp, sppConnected);
        
        /* Unregister the service record since service is no longer available. */
        if (!spp->lazy)        
            ConnectionUnregisterServiceRecord(&spp->task, spp->sdp_record_handle);
    }
    else
    {
        /* Connect attempt failed, free the task if in lazy mode */
        if (spp->lazy)
            MessageSend(&spp->task, SPP_INTERNAL_TASK_DELETE_REQ, 0);
        else
            sppSetState(spp, sppReady);
    }
}


/*****************************************************************************/
void sppHandleRfcommConnectCfm(SPP *spp, const CL_RFCOMM_CONNECT_CFM_T *cfm)
{
    if (cfm->status == rfcomm_connect_success)
	{
        spp->sink = cfm->sink;
		spp->max_frame_size = cfm->frame_size;
	}

    sppSendConnectCfmToApp(convertRfcommConnectStatus(cfm->status), spp);
}


/*****************************************************************************/
void sppHandleRfcommConnectInd(SPP *spp, const CL_RFCOMM_CONNECT_IND_T *ind)
{
	MAKE_SPP_MESSAGE(SPP_CONNECT_IND);
	message->addr = ind->bd_addr;
	message->spp = spp;
	message->frame_size = ind->frame_size;
	MessageSend(spp->clientTask, SPP_CONNECT_IND, message);

	/* Update the local state to indicate we're in the middle of connecting. */
	sppSetState(spp, sppConnecting);

    if (spp->lazy)
        spp->local_server_channel = ind->server_channel;
}


/*****************************************************************************/
void sppHandleConnectIndReject(SPP *spp, const CL_RFCOMM_CONNECT_IND_T *ind)
{
    ConnectionRfcommConnectResponse(&spp->task, 0, &ind->bd_addr, ind->server_channel, 0);

    /* If in lazy mode delete the task as we have just rejected the connection */
    if (spp->lazy)
        MessageSend(&spp->task, SPP_INTERNAL_TASK_DELETE_REQ, 0);
}


/*****************************************************************************/
void sppSendDisconnectIndToApp(SPP *spp, spp_disconnect_status status)
{
    MAKE_SPP_MESSAGE(SPP_DISCONNECT_IND);
    message->status = status;
    message->spp = spp;
    MessageSend(spp->clientTask, SPP_DISCONNECT_IND, message);
}


/*****************************************************************************/
void sppHandleRfcommDisconnectInd(SPP *spp, const CL_RFCOMM_DISCONNECT_IND_T *ind)
{
    /* Send SPP_DISCONNECT_IND message to the client. */	
    sppSendDisconnectIndToApp(spp, convertRfcommDisconnectStatusSpp(ind->status));

    if (spp->lazy)
    {
        /* Free the task */
        MessageSend(&spp->task, SPP_INTERNAL_TASK_DELETE_REQ, 0);
    }
    else
    {
		 spp->sink = 0;
		 sppSetState(spp, sppReady);

        /* Register service record. */
        if (!spp->sdp_record_handle && spp->length_sr)
		    sppRegisterServiceRecord(spp, spp->local_server_channel);
		
    }
}


/*****************************************************************************/
void sppHandleInternalDisconnectReq(SPP *spp)
{
	/* Request the connection lib disconnects the RFCOMM connection */
	ConnectionRfcommDisconnectRequest(&spp->task, spp->sink);
}


/*****************************************************************************/
void sppHandleFreeSppTask(SPP *spp)
{
    /* Discard all messages for the task and free it. */
    (void) MessageFlushTask(&spp->task);
    free(spp);
}


/*lint +e525 +e830 */
