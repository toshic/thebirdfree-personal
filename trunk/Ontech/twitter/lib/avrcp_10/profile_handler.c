/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	profile_handler.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_connect_handler.h"
#include "avrcp_l2cap_handler.h"
#include "avrcp_signal_handler.h"
#include "avrcp_sdp_handler.h"
#include "avrcp_common.h"
#include "init.h"
#include "profile_handler.h"


typedef enum
{
	avrcpUnexpectedClPrim,
	avrcpUnexpectedAvrcpPrim,
	avrcpUnexpectedMessage
} avrcpUnexpectedReasonCode;


/****************************************************************************
NAME	
    handleUnexpected	

DESCRIPTION
    This function is called as a result of a message arriving when this
	library was not expecting it.
*/
static void handleUnexpected(avrcpUnexpectedReasonCode code, avrcpState state, uint16 type)
{
	state = state;
	type = type;
	code = code;

    AVRCP_DEBUG(("handleUnexpected - Code 0x%x State 0x%x MsgId 0x%x\n", code, state, type));
}


/****************************************************************************
NAME	
	avrcpProfileHandler

DESCRIPTION
	All messages for the profile lib instance are handled by this function.
*/
void avrcpProfileHandler(Task task, MessageId id, Message message)
{
	AVRCP *avrcp = (AVRCP *) task;
	avrcpState profileState = avrcp->state;

	/* Check the message id */
	switch (id)
	{
	case CL_L2CAP_REGISTER_CFM:

		switch(profileState)
		{
		case avrcpInitialising:
			avrcpHandleL2capRegisterCfm(avrcp, (CL_L2CAP_REGISTER_CFM_T *) message);
			break;

		case avrcpReady:
		case avrcpConnecting:
		case avrcpConnected:
		default:
			/* Shouldn't get this message in any other state */
			handleUnexpected(avrcpUnexpectedClPrim, profileState, id);
			break;
		}
		break;

	case CL_L2CAP_CONNECT_CFM:
	
		switch(profileState)
		{
		case avrcpConnecting:
			avrcpHandleL2capConnectCfm(avrcp, (CL_L2CAP_CONNECT_CFM_T *) message);
			break;

		case avrcpInitialising:
		case avrcpReady:
		case avrcpConnected:
		default:
			/* Shouldn't get this message in any other state */
			handleUnexpected(avrcpUnexpectedClPrim, profileState, id);
			break;
		}
		break;	

	case CL_L2CAP_CONNECT_IND:
		
		switch(profileState)
		{
		case avrcpReady:
			avrcpHandleL2capConnectInd(avrcp, (CL_L2CAP_CONNECT_IND_T *) message);
			break;
		
		case avrcpConnecting:
		case avrcpConnected:
			avrcpHandleL2capConnectIndReject(avrcp, (CL_L2CAP_CONNECT_IND_T *) message);
			break;

		case avrcpInitialising:
		default:
			/* Shouldn't get this message in any other state */
			handleUnexpected(avrcpUnexpectedClPrim, profileState, id);
			break;
		}
		break;

	case CL_L2CAP_DISCONNECT_IND:
		
		switch(profileState)
		{
		case avrcpConnected:
			avrcpHandleL2capDisconnectInd(avrcp, (CL_L2CAP_DISCONNECT_IND_T *) message);
			break;
		
		case avrcpInitialising:
		case avrcpReady:
		case avrcpConnecting:
		default:
			/* Shouldn't get this message in any other state */
			handleUnexpected(avrcpUnexpectedClPrim, profileState, id);
			break;
		}
		break;

	case CL_SDP_REGISTER_CFM:
		
		switch(profileState)
		{
		case avrcpInitialising:
			avrcpHandleSdpRegisterCfm(avrcp, (CL_SDP_REGISTER_CFM_T *) message);
			break;

        case avrcpReady:
		case avrcpConnecting:
		case avrcpConnected:
		default:
			handleUnexpected(avrcpUnexpectedClPrim, profileState, id);
			break;
		}
		break;

    default:
        switch (id)
        {
            case AVRCP_INTERNAL_TASK_INIT_REQ:        
		    {
                uint16 app = (*((uint16 *)message));
		        avrcpInitTaskData(avrcp, (Task) app, avrcpReady, avrcp_device_none, 1);
		    }
            break;
            
            case AVRCP_INTERNAL_TASK_DELETE_REQ:
                avrcpHandleDeleteTask(avrcp);
                break;
                
            case AVRCP_INTERNAL_INIT_REQ:
                
                switch(profileState)
                {
                case avrcpInitialising:
                    avrcpHandleInternalInitReq(avrcp, (AVRCP_INTERNAL_INIT_REQ_T *) message);
                    break;
                    
                case avrcpReady:
                case avrcpConnecting:
                case avrcpConnected:
                default:
                    /* Shouldn't get this message in any other state */
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
                }
                break;
                
            case AVRCP_INTERNAL_CONNECT_REQ:
                    
                switch(profileState)
                {		
                case avrcpReady:
                    avrcpHandleInternalConnectReq(avrcp, (AVRCP_INTERNAL_CONNECT_REQ_T *) message);
                    break;
                case avrcpConnecting:
                case avrcpConnected:
                    avrcpSendCommonCfmMessageToApp(AVRCP_CONNECT_CFM, avrcp_bad_state, 0, avrcp);
                    break;
                case avrcpInitialising:
                default:
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
                }
                break;
                
            case AVRCP_INTERNAL_DISCONNECT_REQ:
                    
                switch(profileState)
                {		
                case avrcpConnected:
                    avrcpHandleInternalDisconnectReq(avrcp);
                    break;
                case avrcpReady:
                case avrcpConnecting:
                    avrcpSendCommonCfmMessageToApp(AVRCP_DISCONNECT_IND, avrcp_device_not_connected, 0, avrcp);
                    break;
                case avrcpInitialising:
                default:
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
                }
                break;
                
            case AVRCP_INTERNAL_CONNECT_RES:
                    
                switch(profileState)
                {
                case avrcpConnecting:
                    avrcpHandleInternalL2capConnectRes(avrcp, (AVRCP_INTERNAL_CONNECT_RES_T *) message);
                    break;
                case avrcpReady:
                    break;
                case avrcpInitialising:
                case avrcpConnected:
                default:
                    /* Shouldn't get this message in any other state */
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
                }
                break;
                
            case AVRCP_INTERNAL_PASSTHROUGH_REQ:

                switch(profileState)
                {
                case avrcpConnected:
                    avrcpHandleInternalPassThroughReq(avrcp, (AVRCP_INTERNAL_PASSTHROUGH_REQ_T *) message);
                    break;
                case avrcpReady:
                case avrcpConnecting:
                    avrcpSendCommonCfmMessageToApp(AVRCP_PASSTHROUGH_CFM, avrcp_device_not_connected, 0, avrcp);
                    break;
                case avrcpInitialising:
                default:
                    /* Shouldn't get this message in any other state */
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
                }
                break;
                
            case AVRCP_INTERNAL_PASSTHROUGH_RES:
			
                switch(profileState)
                {
                case avrcpConnected:
                    avrcpHandleInternalPassThroughRes(avrcp, (AVRCP_INTERNAL_PASSTHROUGH_RES_T *) message);
                    break;
                case avrcpReady:
                    break;
                case avrcpInitialising:
                case avrcpConnecting:
                default:
                    /* Shouldn't get this message in any other state */
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
                }
                break;

		    case AVRCP_INTERNAL_UNITINFO_REQ:
			
                switch(profileState)
                {
                case avrcpConnected:
                    avrcpHandleInternalUnitInfoReq(avrcp);
                    break;
                case avrcpReady:
                case avrcpConnecting:
                    avrcpSendUnitInfoCfmToClient(avrcp, avrcp_device_not_connected, 0, 0, (uint32) 0);
                    break;
                case avrcpInitialising:
                default:
                    /* Shouldn't get this message in any other state */
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
			    }
			    break;

		    case AVRCP_INTERNAL_UNITINFO_RES:
			
                switch(profileState)
                {
                case avrcpConnected:
                    avrcpHandleInternalUnitInfoRes(avrcp, (AVRCP_INTERNAL_UNITINFO_RES_T *) message);
                    break;
                case avrcpReady:
                    break;
                case avrcpInitialising:
                case avrcpConnecting:
                default:
                    /* Shouldn't get this message in any other state */
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
                }
                break;

		    case AVRCP_INTERNAL_SUBUNITINFO_REQ:
			
			    switch(profileState)
                {
                case avrcpConnected:
                    avrcpHandleInternalSubUnitInfoReq(avrcp, (AVRCP_INTERNAL_SUBUNITINFO_REQ_T *) message);
                    break;
                case avrcpReady:
                case avrcpConnecting:
                    avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_device_not_connected, 0, 0);
                    break;
                case avrcpInitialising:
                default:
                    /* Shouldn't get this message in any other state */
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
                }
                break;

		    case AVRCP_INTERNAL_SUBUNITINFO_RES:
			
                switch(profileState)
                {
                case avrcpConnected:
                    avrcpHandleInternalSubUnitInfoRes(avrcp, (AVRCP_INTERNAL_SUBUNITINFO_RES_T *) message);
                    break;
                case avrcpReady:
                    break;
                case avrcpInitialising:
                case avrcpConnecting:
                default:
                    /* Shouldn't get this message in any other state */
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
                }
                break;

		    case AVRCP_INTERNAL_VENDORDEPENDENT_REQ:
			
                switch(profileState)
                {
                case avrcpConnected:
                    avrcpHandleInternalVendorDependentReq(avrcp, (AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *) message);
                    break;
                case avrcpReady:	
                case avrcpConnecting:
                    avrcpSendCommonCfmMessageToApp(AVRCP_VENDORDEPENDENT_CFM, avrcp_device_not_connected, 0, avrcp);
                    break;
                case avrcpInitialising:
                default:
                    /* Shouldn't get this message in any other state */
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
                }
                break;

		    case AVRCP_INTERNAL_VENDORDEPENDENT_RES:
			
                switch(profileState)
                {
                case avrcpConnected:
                    avrcpHandleInternalVendorDependentRes(avrcp, (AVRCP_INTERNAL_VENDORDEPENDENT_RES_T *) message);
                    break;
                case avrcpReady:
                    break;
                case avrcpInitialising:
                case avrcpConnecting:
                default:
                    /* Shouldn't get this message in any other state */
                    handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                    break;
                }
                break;

		    case AVRCP_INTERNAL_WATCHDOG_TIMEOUT:
			    avrcpHandleInternalWatchdogTimeout(avrcp);
			    break;
      
            default:
                switch (id)
                {
			        case MESSAGE_MORE_DATA:				
                        switch(profileState)
                        {
                        case avrcpConnected:
                            avrcpHandleReceivedData(avrcp);
                            break;
                            
                        case avrcpInitialising:
                        case avrcpReady:
                        case avrcpConnecting:
                        default:
                            /* Shouldn't get this message in any other state */
                            handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
                            break;
                        }
                        break;

			        /* Ignored primitives */
			        case MESSAGE_MORE_SPACE:
			        case MESSAGE_SOURCE_EMPTY:
				        break;
				
			        default:		
				        /* Received an unknown message */
				        handleUnexpected(avrcpUnexpectedMessage, profileState, id);
				        break;
                }
        }
	}
}
