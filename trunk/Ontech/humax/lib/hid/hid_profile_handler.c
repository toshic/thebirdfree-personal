/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2005-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	hid_profile_handler.c        
DESCRIPTION
	Handles all message send to the HID library.
*/

#include <assert.h>
#include <stream.h>

#include "hid.h"
#include "hid_init.h"
#include "hid_sdp.h"
#include "hid_profile_handler.h"
#include "hid_l2cap_handler.h"
#include "hid_control_handler.h"
#include "hid_common.h"
#include "hid_connect.h"

/* Macro to handle unexpected messages */
#define hidHandleUnexpected(hid, id) \
    HID_DEBUG(("hid profile handler %p - message type not yet handled 0x%x, state %d\n", (void *)hid, id, hidGetState(hid)));

/****************************************************************************
NAME 
	hidProfileHandler
DESCRIPTION
	All messages for this a HID instance are handled by this function
RETURNS
	void
*/
void hidProfileHandler(Task task, MessageId id, Message message)
{
	HID *hid = (HID *)task;
    hidState state = hidGetState(hid);
    
	/* Check the message id */
	switch (id)
    {
		case HID_INTERNAL_CONNECT_CONTROL_IND:
			switch (state)
			{
				case hidIdle:
					hidHandleInternalConnectControlInd(hid, (HID_INTERNAL_CONNECT_IND_T *)message);
					break;
				default:
					hidRejectInternalConnectInd(hid, (HID_INTERNAL_CONNECT_IND_T *)message);
					break;
			}
			break;

		case HID_INTERNAL_CONNECT_INTERRUPT_IND:
			switch (state)
			{
				case hidConnectingRemote:
					hidHandleInternalConnectInterruptInd(hid, (HID_INTERNAL_CONNECT_IND_T *)message);
					break;
				case hidIdle:
					hidRejectInternalConnectInd(hid, (HID_INTERNAL_CONNECT_IND_T *)message);
					hidDestroy(hid);
					break;
				default:
					hidRejectInternalConnectInd(hid, (HID_INTERNAL_CONNECT_IND_T *)message);
					break;
			}
			break;

		case HID_INTERNAL_CONNECT_RES:
            switch (state)
            {
                case hidConnectingRemote:
					hidHandleInternalConnectRes(hid, (HID_INTERNAL_CONNECT_RES_T *)message);
					break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
            
        case HID_INTERNAL_DISCONNECT_REQ:
            switch (state)
            {
                case hidConnectingLocal:
                case hidConnectingRemote:
                case hidConnected:
                    hidHandleInternalDisconnectReq(hid);
                    break;
                case hidDisconnectingRemote:
					break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
             }
             break;
        
        case HID_INTERNAL_CONNECT_TIMEOUT_IND:
            switch (state)
            {
                case hidConnectingLocal:
                case hidConnectingRemote:
                    hidHandleInternalConnectTimeoutInd(hid);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
        
        case HID_INTERNAL_DISCONNECT_TIMEOUT_IND:
            switch (state)
            {
                case hidDisconnectingRemote:
                    hidHandleInternalDisconnectTimeoutInd(hid);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;

        case HID_INTERNAL_CONTROL_REQ:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalControlRequest(hid, (HID_INTERNAL_CONTROL_REQ_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
            
        case HID_INTERNAL_REQUEST_TIMEOUT_IND:
            switch (state)
            {
                case hidConnected:
                    hidHandleInternalRequestTimeout(hid);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;

		case HID_INTERNAL_CHECK_CONTROL_REQ:
			switch (state)
			{
				case hidConnected:
					hidHandleControlPacket(hid, StreamSourceFromSink(hid->connection[HID_CON_CONTROL].con.sink));                          
					break;
				default:
					hidHandleUnexpected(hid, id);
					break;
			}
			break;

#ifdef HID_DEVICE
		case HID_INTERNAL_GET_IDLE_RES:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalGetIdleResponse(hid, (HID_INTERNAL_GET_IDLE_RES_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;

        case HID_INTERNAL_SET_IDLE_RES:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalSetIdleResponse(hid, (HID_INTERNAL_SET_IDLE_RES_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;

        case HID_INTERNAL_GET_PROTOCOL_RES:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalGetProtocolResponse(hid, (HID_INTERNAL_GET_PROTOCOL_RES_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;

        case HID_INTERNAL_SET_PROTOCOL_RES:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalSetProtocolResponse(hid, (HID_INTERNAL_SET_PROTOCOL_RES_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
            
        case HID_INTERNAL_GET_REPORT_RES:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalGetReportResponse(hid, (HID_INTERNAL_GET_REPORT_RES_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;

        case HID_INTERNAL_SET_REPORT_RES:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalSetReportResponse(hid, (HID_INTERNAL_SET_REPORT_RES_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
#endif /* HID_DEVICE */

#ifdef HID_HOST
        case HID_INTERNAL_GET_IDLE_REQ:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalGetIdleRequest(hid, (HID_INTERNAL_GET_IDLE_REQ_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
            
        case HID_INTERNAL_SET_IDLE_REQ:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalSetIdleRequest(hid, (HID_INTERNAL_SET_IDLE_REQ_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
            
        case HID_INTERNAL_GET_PROTOCOL_REQ:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalGetProtocolRequest(hid, (HID_INTERNAL_GET_PROTOCOL_REQ_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
            
        case HID_INTERNAL_SET_PROTOCOL_REQ:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalSetProtocolRequest(hid, (HID_INTERNAL_SET_PROTOCOL_REQ_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
            
        case HID_INTERNAL_GET_REPORT_REQ:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalGetReportRequest(hid, (HID_INTERNAL_GET_REPORT_REQ_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
            
        case HID_INTERNAL_SET_REPORT_REQ:
            switch (state)
            {
                case hidDisconnectingRemote:
                case hidConnected:
                    hidHandleInternalSetReportRequest(hid, (HID_INTERNAL_SET_REPORT_REQ_T *)message);
                    break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
#endif /* HID_HOST */

        case CL_L2CAP_CONNECT_CFM:
            switch (state)
            {
                case hidConnectingLocal:
                    hidHandleL2capConnectCfmLocal(hid, (CL_L2CAP_CONNECT_CFM_T *)message);
					break;
				case hidConnectingRemote:
                    hidHandleL2capConnectCfmRemote(hid, (CL_L2CAP_CONNECT_CFM_T *)message);
                    break;
                case hidDisconnectingLocal:
                case hidDisconnectingRemote:
                case hidDisconnectingFailed:
                    hidHandleL2capConnectCfmDisconnecting(hid, (CL_L2CAP_CONNECT_CFM_T *)message);
					break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
                
        case CL_L2CAP_DISCONNECT_IND:
            switch (state)
            {
				case hidConnectingLocal:
				case hidConnectingRemote:
                    hidHandleL2capDisconnectIndConnecting(hid, (CL_L2CAP_DISCONNECT_IND_T *)message);
					break;
                case hidConnected:
                    hidHandleL2capDisconnectIndConnected(hid, (CL_L2CAP_DISCONNECT_IND_T *)message);
					break;
				case hidDisconnectingLocal:
				case hidDisconnectingRemote:
				case hidDisconnectingFailed:
                    hidHandleL2capDisconnectIndDisconnecting(hid, (CL_L2CAP_DISCONNECT_IND_T *)message);
					break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
            }
            break;
            
		case CL_DM_ROLE_CFM:
			HID_PRINT(("CL_DM_ROLE_CFM role=0x%x\n", ((CL_DM_ROLE_CFM_T*)message)->role));
			break;
            
        case MESSAGE_MORE_DATA:
			switch (state)
			{
                case hidConnected:                
            		hidHandleControlPacket(hid, ((MessageMoreData *)message)->source);                          
                    break;
				case hidConnectingRemote:                    
                case hidDisconnectingLocal:
                case hidDisconnectingRemote:
				case hidDisconnectingFailed:
                case hidConnectingLocal:
					break;
                default:
                    hidHandleUnexpected(hid, id);
                    break;
        	}
			break; 

		case MESSAGE_MORE_SPACE:
		case MESSAGE_SOURCE_EMPTY:
		case MESSAGE_STREAM_DISCONNECT:
		case CL_SM_ENCRYPTION_CHANGE_IND:
			/* Ignore these messages, we are not interested */
			break;

        default:
         	/* Received an unknown message */
            hidHandleUnexpected(hid, id);
         	break;
    }
}

/****************************************************************************
NAME 
	hidLibProfileHandler
DESCRIPTION
	All messages for this profile lib are handled by this function
RETURNS
	void
*/
void hidLibProfileHandler(Task task, MessageId id, Message message)
{
	HID_LIB *hidLib = (HID_LIB *)task;
    
	/* Check the message id */
	switch (id)
	{
        case HID_INTERNAL_INIT_REQ:
            hidHandleInternalInitReq(hidLib, (HID_INTERNAL_INIT_REQ_T *)message);
            break;
            
    	case HID_INTERNAL_INIT_CFM:
            hidHandleInternalInitCfm(hidLib, (HID_INTERNAL_INIT_CFM_T *)message);
            break;
            
        case HID_INTERNAL_CONNECT_REQ:
            hidHandleInternalConnectReq(hidLib, (HID_INTERNAL_CONNECT_REQ_T *)message);
            break;
            
		case CL_SDP_REGISTER_CFM:
			hidHandleSdpRegisterCfm(hidLib, (CL_SDP_REGISTER_CFM_T *)message);
            break;
            
     	case CL_L2CAP_REGISTER_CFM:
			hidHandleL2capRegisterCfm(hidLib, (CL_L2CAP_REGISTER_CFM_T *)message);
			break;
    
        case CL_L2CAP_CONNECT_IND:
            hidHandleL2capConnectInd(hidLib, (CL_L2CAP_CONNECT_IND_T *)message);
            break;
            
        default:
			/* Received an unknown message */
            HID_PRINT(("hid lib profile handler - message type not yet handled 0x%x\n", id));
			break;
	}
}
