/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	spp_profile_handler.c        

DESCRIPTION
    Handles all messages passing through the SPP profile library
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "init.h"
#include "spp_connect_handler.h"
#include "spp_private.h"
#include "spp_sdp.h"


typedef enum
{
	sppUnexpectedClPrim,
	sppUnexpectedSppPrim
} sppUnexpectedReasonCode;


/*****************************************************************************/
static void handleUnexpected(sppUnexpectedReasonCode code, sppState state, uint16 type)
{
	code = code;
	state = state;
	type = type;
	
    SPP_DEBUG(("handleUnexpected - Code 0x%x State 0x%x MsgId 0x%x\n", code, state, type));
}


/*****************************************************************************/
void sppProfileHandler(Task task, MessageId id, Message message)
{
	SPP* spp = (SPP*) task;
	sppState profileState = spp->state;

	/* Check the message id */
    switch (id)
    {
    case SPP_INTERNAL_TASK_INIT_REQ:
        {
            uint16 app = (*((uint16 *)message));
            sppInitTaskData(spp, 0, 0, (Task) app, sppReady, 0, 0, 0, 0, 1);
        }
        break;

    case SPP_INTERNAL_TASK_DELETE_REQ:
        sppHandleFreeSppTask(spp);
        break;

    case SPP_INTERNAL_INIT_REQ:
        switch(profileState)
        {
        case sppInitialising:
            sppHandleInternalInitReq(spp, (SPP_INTERNAL_INIT_REQ_T *) message);
            break;

        case sppReady:
        case sppSearching:
        case sppConnecting:
        case sppConnected:
        default:
            handleUnexpected(sppUnexpectedSppPrim, profileState, id);
        }
        break;

    case CL_RFCOMM_REGISTER_CFM:
        switch(profileState)
        {
        case sppInitialising:
            sppHandleRfcommRegisterCfm(spp, (CL_RFCOMM_REGISTER_CFM_T *) message);
            break;
            
        case sppReady:
        case sppSearching:
        case sppConnecting:
        case sppConnected:
        default:
            handleUnexpected(sppUnexpectedClPrim, profileState, id);
            break;
        }
        break;

    case CL_SDP_REGISTER_CFM:
        switch(profileState)
        {
        case sppInitialising:
            sppHandleSdpRegisterCfm(spp, (CL_SDP_REGISTER_CFM_T *) message);
            break;
            
        case sppReady:
        case sppSearching:
        case sppConnecting:
            sppHandleSdpRegisterCfmReady(spp, (CL_SDP_REGISTER_CFM_T *) message);
            break;
            
        case sppConnected:
        default:
            handleUnexpected(sppUnexpectedClPrim, profileState, id);
            break;
        }
		break;

	case CL_SDP_UNREGISTER_CFM:
        switch(profileState)
        {
        case sppReady:
        case sppConnected:
            sppHandleSdpUnregisterCfm(spp, (CL_SDP_UNREGISTER_CFM_T *) message);
            break;
            
        case sppInitialising:		
        case sppSearching:
        case sppConnecting:
        default:
            handleUnexpected(sppUnexpectedClPrim, profileState, id);
            break;
        }
		break;

    case SPP_INTERNAL_CONNECT_REQ:
        switch(profileState)
        {
        case sppReady:
            sppHandleConnectRequest(spp, (SPP_INTERNAL_CONNECT_REQ_T *) message);
            break;
            
        case sppSearching:
        case sppConnecting:
        case sppConnected:	
            sppSendConnectCfmToApp(spp_connect_failed_busy, spp);
            break;
            
        case sppInitialising:
        default:
            handleUnexpected(sppUnexpectedSppPrim, profileState, id);
            break;
        }
		break;

	case SPP_INTERNAL_CONNECT_RES:
        switch(profileState)
        {
        case sppConnecting:
            sppHandleConnectResponse(spp, (SPP_INTERNAL_CONNECT_RES_T *) message);
            break;
            
        case sppReady:
        case sppSearching:
        case sppConnected:	
            sppSendConnectCfmToApp(spp_connect_failed_busy, spp);
            break;
            
        case sppInitialising:
        default:
            handleUnexpected(sppUnexpectedSppPrim, profileState, id);
            break;
        }
		break;

	case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
        switch(profileState)
        {
        case sppSearching:
            sppHandleSdpServiceSearchAttributeCfm(spp, (CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *) message);
            break;
            
        case sppReady:
        case sppConnecting:
        case sppConnected:
            break;
            
        case sppInitialising:		
        default:
            handleUnexpected(sppUnexpectedClPrim, profileState, id);
            break;
        }
		break;

	case SPP_INTERNAL_RFCOMM_CONNECT_REQ:
        switch(profileState)
        {
        case sppConnecting:
            sppHandleInternalRfcommConnectRequest(spp, (SPP_INTERNAL_RFCOMM_CONNECT_REQ_T *) message);
            break;
            
        case sppReady:
        case sppSearching:
        case sppInitialising:
        case sppConnected:	
        default:
            handleUnexpected(sppUnexpectedSppPrim, profileState, id);
            break;
        }
		break;

	case CL_RFCOMM_CONNECT_CFM:
        switch(profileState)
        {
        case sppConnecting: 
            sppHandleRfcommConnectCfm(spp, (CL_RFCOMM_CONNECT_CFM_T *) message);
            break;
            
        case sppConnected:	
        case sppReady:
        case sppSearching:
        case sppInitialising:
        default:
            handleUnexpected(sppUnexpectedClPrim, profileState, id);
            break;
        }
		break;

	case CL_RFCOMM_CONNECT_IND:
        switch(profileState)
        {
        case sppReady:
            sppHandleRfcommConnectInd(spp, (CL_RFCOMM_CONNECT_IND_T *) message);
            break;
            
        case sppSearching:
        case sppConnecting:
        case sppConnected:	
            sppHandleConnectIndReject(spp, (CL_RFCOMM_CONNECT_IND_T *) message);
            break;
            
        case sppInitialising:
        default:
            handleUnexpected(sppUnexpectedClPrim, profileState, id);
            break;
        }
		break;

	case CL_RFCOMM_DISCONNECT_IND:
        switch(profileState)
        {
        case sppReady:
        case sppSearching:
        case sppConnecting:
        case sppConnected:
            sppHandleRfcommDisconnectInd(spp, (CL_RFCOMM_DISCONNECT_IND_T *) message);
            break;
            
        case sppInitialising:
        default:
            handleUnexpected(sppUnexpectedClPrim, profileState, id);
            break;
        }
		break;

	case SPP_INTERNAL_DISCONNECT_REQ:
        switch(profileState)
        {
        case sppConnecting:
        case sppConnected:
            sppHandleInternalDisconnectReq(spp);
            break;
            
        case sppReady:
        case sppSearching:
            sppSendDisconnectIndToApp(spp, spp_disconnect_no_slc);
            break;
            
        case sppInitialising:
        default:
            handleUnexpected(sppUnexpectedSppPrim, profileState, id);
            break;
        }
		break;

	case CL_RFCOMM_CONTROL_IND:
        { 
            /* Forward the Modem Control Indicators */
            CL_RFCOMM_CONTROL_IND_T *src_msg;
            CL_RFCOMM_CONTROL_IND_T *ind = PanicUnlessNew(CL_RFCOMM_CONTROL_IND_T);
            src_msg = (CL_RFCOMM_CONTROL_IND_T *)message;
            *ind = *src_msg;    	    
            MessageSend(spp->clientTask, CL_RFCOMM_CONTROL_IND, ind);
        }
		break;
        
    case MESSAGE_MORE_DATA:
        {
            SPP_MESSAGE_MORE_DATA_T* msg = PanicUnlessNew(SPP_MESSAGE_MORE_DATA_T);
            msg->source = ((MessageMoreData*)message)->source;
            msg->spp = spp;
            MessageSend(spp->clientTask, SPP_MESSAGE_MORE_DATA, msg);
        }
        break;
        
    case MESSAGE_MORE_SPACE:
        {
            SPP_MESSAGE_MORE_SPACE_T* msg = PanicUnlessNew(SPP_MESSAGE_MORE_SPACE_T);
            msg->sink = ((MessageMoreSpace*)message)->sink;
            msg->spp = spp;
            MessageSend(spp->clientTask, SPP_MESSAGE_MORE_SPACE, msg);
        }
		break;
		
	case SPP_INTERNAL_SEND_CFM_TO_APP:
		{
			SPP_INTERNAL_SEND_CFM_TO_APP_T* msg = (SPP_INTERNAL_SEND_CFM_TO_APP_T*)message;
			sppSendConnectCfmToApp(msg->status, msg->spp);
		}
		break;
		
		/* Ignored messages */
	case MESSAGE_STREAM_DISCONNECT:
	case MESSAGE_SOURCE_EMPTY:
		break;
		
	default:
		/* Received an unknown message */
		SPP_DEBUG(("spp profile handler - msg type  not yet handled 0x%x\n", id));
		break;
	}
}
