/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_profile_handler.c        

DESCRIPTION
	File containing the profile handler function for the a2dp library.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_close_handler.h"
#include "a2dp_codec_handler.h"
#include "a2dp_delete.h"
#include "a2dp_disconnect_handler.h"
#include "a2dp_init.h"
#include "a2dp_l2cap_handler.h"
#include "a2dp_open_handler.h"
#include "a2dp_private.h"
#include "a2dp_profile_handler.h"
#include "a2dp_reconfigure_handler.h"
#include "a2dp_receive_packet_handler.h"
#include "a2dp_sdp.h"
#include "a2dp_signalling_handler.h"
#include "a2dp_start_handler.h"
#include "a2dp_suspend_handler.h"

#include <print.h>


/*****************************************************************************/
static void handleUnexpected(uint16 type)
{	
    A2DP_DEBUG(("A2DP handleUnexpected - MsgId 0x%x\n", type));
}


/****************************************************************************/
static void sendEncryptionChangeInd(A2DP *a2dp, const CL_SM_ENCRYPTION_CHANGE_IND_T *ind)
{
	MAKE_A2DP_MESSAGE(A2DP_ENCRYPTION_CHANGE_IND);
	message->encrypted = ind->encrypted;
	message->a2dp = a2dp;
	MessageSend(a2dp->clientTask, A2DP_ENCRYPTION_CHANGE_IND, message);
}


/*****************************************************************************/
void a2dpProfileHandler(Task task, MessageId id, Message message)
{
    A2DP *a2dp = (A2DP *) task;
	uint16 app;

    switch (id)
    {
	case A2DP_INTERNAL_TASK_INIT_REQ:
		PRINT(("A2DP_INTERNAL_TASK_INIT_REQ\n"));
		app = (*((uint16 *)message));
		a2dpInitTask(a2dp, (Task) app);
		break;

	case A2DP_INTERNAL_CONNECT_SIGNALLING_REQ:
		PRINT(("A2DP_INTERNAL_CONNECT_SIGNALLING_REQ\n"));
		a2dpHandleConnectSignallingReq(a2dp, (A2DP_INTERNAL_CONNECT_SIGNALLING_REQ_T *) message);
		break;

	case A2DP_INTERNAL_CONNECT_SIGNALLING_RES:
		PRINT(("A2DP_INTERNAL_CONNECT_SIGNALLING_RES\n"));
		a2dpHandleConnectSignallingRes(a2dp, (A2DP_INTERNAL_CONNECT_SIGNALLING_RES_T *) message);
		break;

	case A2DP_INTERNAL_CONFIGURE_CODEC_RSP:
		PRINT(("A2DP_INTERNAL_CONFIGURE_CODEC_RSP\n"));
		a2dpHandleConfigureCodecRes(a2dp, (A2DP_INTERNAL_CONFIGURE_CODEC_RSP_T *) message);
		break;

    case A2DP_INTERNAL_OPEN_REQ:
        PRINT(("A2DP_INTERNAL_OPEN_REQ\n"));
        a2dpHandleOpenReq(a2dp, (A2DP_INTERNAL_OPEN_REQ_T *) message);
        break;

	case A2DP_INTERNAL_TASK_DELETE_REQ:
		PRINT(("A2DP_INTERNAL_TASK_DELETE_REQ\n"));
        a2dpHandleDeleteTask(a2dp);
        break;

	case A2DP_INTERNAL_WATCHDOG_IND:
		PRINT(("A2DP_INTERNAL_WATCHDOG_IND\n"));
		a2dpHandleWatchdogInd(a2dp);
		break;

	case A2DP_INTERNAL_SIGNAL_PACKET_IND:
		PRINT(("A2DP_INTERNAL_SIGNAL_PACKET_IND\n"));
		a2dpHandleNewSignalPacket(a2dp);
		break;

	case A2DP_INTERNAL_SIGNAL_CONNECTION_TIMEOUT_IND:
		PRINT(("A2DP_INTERNAL_SIGNAL_CONNECTION_TIMEOUT_IND\n"));
		a2dpHandleSignalConnectionTimeoutInd(a2dp);
		break;

	case A2DP_INTERNAL_RECONFIGURE_REQ:
		PRINT(("A2DP_INTERNAL_RECONFIGURE_REQ\n"));
		a2dpHandleReconfigureReq(a2dp, (A2DP_INTERNAL_RECONFIGURE_REQ_T *) message);
		break;

	case A2DP_INTERNAL_START_REQ:
		PRINT(("A2DP_INTERNAL_START_REQ\n"));
		a2dpHandleStartReq(a2dp);
		break;

	case A2DP_INTERNAL_SUSPEND_REQ:
		PRINT(("A2DP_INTERNAL_SUSPEND_REQ\n"));
		a2dpHandleSuspendReq(a2dp);
		break;

	case A2DP_INTERNAL_CLOSE_REQ:
		PRINT(("A2DP_INTERNAL_CLOSE_REQ\n"));
		a2dpHandleCloseReq(a2dp);
		break;

	case A2DP_INTERNAL_DISCONNECT_ALL_REQ:
		PRINT(("A2DP_INTERNAL_DISCONNECT_ALL_REQ\n"));
		a2dpHandleDisconnectAllReq(a2dp);
		break;

	case A2DP_INTERNAL_SEND_CODEC_PARAMS_REQ:
		PRINT(("A2DP_INTERNAL_SEND_CODEC_PARAMS_REQ\n"));
		a2dpSendCodecAudioParams(a2dp);
		if (((A2DP_INTERNAL_SEND_CODEC_PARAMS_REQ_T *) message)->send_reconfigure_message)
			a2dpSendReconfigureCfm(a2dp, a2dp_success);
		break;

	case A2DP_INTERNAL_GET_CAPS_TIMEOUT_IND:
		PRINT(("A2DP_INTERNAL_GET_CAPS_TIMEOUT_IND\n"));
		a2dpGetCapsTimeout(a2dp);
		break;


	default:
		switch(id)
		{
		case CL_SDP_REGISTER_CFM:
			PRINT(("CL_SDP_REGISTER_CFM\n"));				
			a2dpHandleSdpRegisterCfm(a2dp, (CL_SDP_REGISTER_CFM_T *) message);		
			break;

		case CL_L2CAP_REGISTER_CFM:
			PRINT(("CL_L2CAP_REGISTER_CFM\n"));
			a2dpHandleL2capRegisterCfm(a2dp, (CL_L2CAP_REGISTER_CFM_T *) message);
			break;

		case CL_L2CAP_CONNECT_IND:
			PRINT(("CL_L2CAP_CONNECT_IND\n"));
			a2dpHandleL2capConnectInd(a2dp, (CL_L2CAP_CONNECT_IND_T *) message);
			break;

		case CL_L2CAP_CONNECT_CFM:
			PRINT(("CL_L2CAP_CONNECT_CFM\n"));
			a2dpHandleL2capConnectCfm(a2dp, (CL_L2CAP_CONNECT_CFM_T *) message);
			break;

		case CL_L2CAP_DISCONNECT_IND:
			PRINT(("CL_L2CAP_DISCONNECT_IND\n"));
			a2dpHandleL2capDisconnectInd(a2dp, (CL_L2CAP_DISCONNECT_IND_T *) message);
			break;

		case CL_SM_ENCRYPTION_CHANGE_IND:
			PRINT(("CL_SM_ENCRYPTION_CHANGE_IND\n"));
			/* We have received an indication that the encryption status of the sink has changed */
			sendEncryptionChangeInd(a2dp, (CL_SM_ENCRYPTION_CHANGE_IND_T *) message);
			break;

		case CL_DM_ROLE_CFM:
			break;

		default:
			switch(id)
			{
			case MESSAGE_MORE_DATA:
				PRINT(("MESSAGE_MORE_DATA\n"));
				/* Data has arrived on the signalling channel */
				a2dpHandleSignalPacket(a2dp, ((MessageMoreData *) message)->source);
				break;

			case MESSAGE_MORE_SPACE:
			case MESSAGE_STREAM_DISCONNECT:
			case MESSAGE_SOURCE_EMPTY:
				break;

			default:
				handleUnexpected(id);
				break;
			}
		}
    }
}
