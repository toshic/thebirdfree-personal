/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	profile_handler.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_battery_handler.h"
#include "avrcp_caps_handler.h"
#include "avrcp_character_handler.h"
#include "avrcp_common.h"
#include "avrcp_connect_handler.h"
#include "avrcp_continuation_handler.h"
#include "avrcp_element_attributes_handler.h"
#include "avrcp_group_navigation_handler.h"
#include "avrcp_play_status_handler.h"
#include "avrcp_player_app_settings_handler.h"
#include "avrcp_private.h"
#include "avrcp_l2cap_handler.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_notification_handler.h"
#include "avrcp_sdp_handler.h"
#include "avrcp_signal_handler.h"
#include "avrcp_signal_passthrough.h"
#include "avrcp_signal_unit_info.h"
#include "avrcp_signal_vendor.h"
#include "avrcp_init.h"
#include "avrcp_profile_handler.h"

#include <stdlib.h>



typedef enum
{
	avrcpUnexpectedClPrim,
	avrcpUnexpectedAvrcpPrim,
	avrcpUnexpectedMessage
} avrcpUnexpectedReasonCode;


/*****************************************************************************/
static void handleUnexpected(avrcpUnexpectedReasonCode code, avrcpState state, uint16 type)
{
	state = state;
	type = type;
	code = code;

    AVRCP_DEBUG((" AVRCP handleUnexpected - Code 0x%x State 0x%x MsgId 0x%x\n", code, state, type));
}


/*****************************************************************************/
void avrcpDataCleanUp(Task task, MessageId id, Message message)
{
    AvrcpCleanUpTask *cleanTask = (AvrcpCleanUpTask *) task;

    switch (id)
    {
    case MESSAGE_SOURCE_EMPTY:
        {
            /* Free the previously stored data ptr. */
            if (cleanTask->sent_data)
                free(cleanTask->sent_data);

            cleanTask->sent_data = 0;
        }
        break;

    default:
        handleUnexpected(avrcpUnexpectedAvrcpPrim, 0, id);
        break;
    }
}


/*****************************************************************************/
void avrcpProfileHandler(Task task, MessageId id, Message message)
{
	AVRCP *avrcp = (AVRCP *) task;
	avrcpState profileState = avrcp->state;

	/* Check the message id */
	switch (id)
	{
	case CL_L2CAP_REGISTER_CFM:
		if (profileState == avrcpInitialising)
			avrcpHandleL2capRegisterCfm(avrcp, (CL_L2CAP_REGISTER_CFM_T *) message);
		else 
			goto handle_unexpected_cl_msg;
		break;

	case CL_L2CAP_CONNECT_CFM:
		if (profileState == avrcpConnecting)
			avrcpHandleL2capConnectCfm(avrcp, (CL_L2CAP_CONNECT_CFM_T *) message);
		else 
			goto handle_unexpected_cl_msg;
		break;

	case CL_L2CAP_CONNECT_IND:
		if (profileState == avrcpReady)
			avrcpHandleL2capConnectInd(avrcp, (CL_L2CAP_CONNECT_IND_T *) message);
		else if ((profileState == avrcpConnecting) || (profileState == avrcpConnected))
			avrcpHandleL2capConnectIndReject(avrcp, (CL_L2CAP_CONNECT_IND_T *) message);
		else
			goto handle_unexpected_cl_msg;
		break;

	case CL_L2CAP_DISCONNECT_IND:
		if (profileState == avrcpConnected)
			avrcpHandleL2capDisconnectInd(avrcp, (CL_L2CAP_DISCONNECT_IND_T *) message);
		else 
			goto handle_unexpected_cl_msg;
		break;

	case CL_SDP_REGISTER_CFM:
		if (profileState == avrcpInitialising)
			avrcpHandleSdpRegisterCfm(avrcp, (CL_SDP_REGISTER_CFM_T *) message);
		else 
			goto handle_unexpected_cl_msg;
		break;

	case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
		if (profileState == avrcpConnected)
			avrcpHandleServiceSearchAttributeCfm(avrcp, (CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *) message);
		else 
			goto handle_unexpected_cl_msg;
		break;
	
	default:
		switch (id)
		{
		case AVRCP_INTERNAL_TASK_INIT_REQ:        
			{
				uint16 app = (*((uint16 *)message));
				avrcpInitTaskData(avrcp, (Task) app, avrcpReady, avrcp_device_none, 0, 0, 0, 1);
			}
			break;

		case AVRCP_INTERNAL_TASK_DELETE_REQ:
			avrcpHandleDeleteTask(avrcp);
			break;

		case AVRCP_INTERNAL_INIT_REQ:
			if (profileState == avrcpInitialising)
				avrcpHandleInternalInitReq(avrcp, (AVRCP_INTERNAL_INIT_REQ_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_CONNECT_REQ:
			if (profileState == avrcpReady)
				avrcpHandleInternalConnectReq(avrcp, (AVRCP_INTERNAL_CONNECT_REQ_T *) message);
			else if ((profileState == avrcpConnecting) || (profileState == avrcpConnected))
				avrcpSendCommonCfmMessageToApp(AVRCP_CONNECT_CFM, avrcp_bad_state, 0, avrcp);
			else
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_DISCONNECT_REQ:
			if (profileState == avrcpConnected)
				avrcpHandleInternalDisconnectReq(avrcp);
			else if ((profileState == avrcpReady) || (profileState == avrcpConnecting))
				avrcpSendCommonCfmMessageToApp(AVRCP_DISCONNECT_IND, avrcp_device_not_connected, 0, avrcp);
			else
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_CONNECT_RES:
			if (profileState == avrcpConnecting)
				avrcpHandleInternalL2capConnectRes(avrcp, (AVRCP_INTERNAL_CONNECT_RES_T *) message);
			else if (profileState == avrcpReady)
			{
				/* do nothing */
			}
			else
				goto handle_unexpected_avrcp_msg;
			break;

#ifdef AVRCP_CT_SUPPORT
		case AVRCP_INTERNAL_PASSTHROUGH_REQ:
			if (profileState == avrcpConnected)
				avrcpHandleInternalPassThroughReq(avrcp, (AVRCP_INTERNAL_PASSTHROUGH_REQ_T *) message);
			else if ((profileState == avrcpReady) || (profileState == avrcpConnecting))
				avrcpSendCommonCfmMessageToApp(AVRCP_PASSTHROUGH_CFM, avrcp_device_not_connected, 0, avrcp);
			else
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_UNITINFO_REQ:
			if (profileState == avrcpConnected)
				avrcpHandleInternalUnitInfoReq(avrcp);
			else if ((profileState == avrcpReady) || (profileState == avrcpConnecting))
				avrcpSendUnitInfoCfmToClient(avrcp, avrcp_device_not_connected, 0, 0, (uint32) 0);
			else
				goto handle_unexpected_avrcp_msg;
			break;
			
		case AVRCP_INTERNAL_SUBUNITINFO_REQ:
			if (profileState == avrcpConnected)
				avrcpHandleInternalSubUnitInfoReq(avrcp, (AVRCP_INTERNAL_SUBUNITINFO_REQ_T *) message);
			else if ((profileState == avrcpReady) || (profileState == avrcpConnecting))
				avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_device_not_connected, 0, 0);
			else
				goto handle_unexpected_avrcp_msg;
			break;
		
		case AVRCP_INTERNAL_VENDORDEPENDENT_REQ:
			if (profileState == avrcpConnected)
				avrcpHandleInternalVendorDependentReq(avrcp, (AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *) message);
			else if ((profileState == avrcpReady) || (profileState == avrcpConnecting))
				avrcpSendCommonCfmMessageToApp(AVRCP_VENDORDEPENDENT_CFM, avrcp_device_not_connected, 0, avrcp);
			else
				goto handle_unexpected_avrcp_msg;
			break;

#endif
		case AVRCP_INTERNAL_PASSTHROUGH_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalPassThroughRes(avrcp, (AVRCP_INTERNAL_PASSTHROUGH_RES_T *) message);
			else if (profileState == avrcpReady)
			{
				/* do nothing */
			}
			else
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_UNITINFO_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalUnitInfoRes(avrcp, (AVRCP_INTERNAL_UNITINFO_RES_T *) message);
			else if (profileState == avrcpReady)
			{
				/* do nothing */
			}
			else
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_SUBUNITINFO_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalSubUnitInfoRes(avrcp, (AVRCP_INTERNAL_SUBUNITINFO_RES_T *) message);
			else if (profileState == avrcpReady)
			{
				/* do nothing */
			}
			else
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_VENDORDEPENDENT_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalVendorDependentRes(avrcp, (AVRCP_INTERNAL_VENDORDEPENDENT_RES_T *) message);
			else if (profileState == avrcpReady)
			{
				/* do nothing */
			}
			else
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_WATCHDOG_TIMEOUT:
#ifdef AVRCP_CT_SUPPORT
			avrcpHandleInternalWatchdogTimeout(avrcp);
#endif
			break;

		case AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT:
			avrcpHandleInternalSendResponseTimeout(avrcp, (AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT_T *) message);
			break;
			
		case AVRCP_INTERNAL_GET_CAPS_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalGetCapsResponse(avrcp, (AVRCP_INTERNAL_GET_CAPS_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalListAppAttributesResponse(avrcp, (AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_LIST_APP_VALUE_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalListAppValuesResponse(avrcp, (AVRCP_INTERNAL_LIST_APP_VALUE_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_GET_APP_VALUE_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalGetAppValueResponse(avrcp, (AVRCP_INTERNAL_GET_APP_VALUE_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_SET_APP_VALUE_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalSetAppValueResponse(avrcp, (AVRCP_INTERNAL_SET_APP_VALUE_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalGetAppAttributeTextResponse(avrcp, (AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalGetAppValueTextResponse(avrcp, (AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalGetElementAttributesResponse(avrcp, (AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_GET_PLAY_STATUS_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalGetPlayStatusResponse(avrcp, (AVRCP_INTERNAL_GET_PLAY_STATUS_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalEventPlaybackStatusChangedResponse(avrcp, (AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalEventTrackChangedResponse(avrcp, (AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalEventTrackReachedEndResponse(avrcp, (AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalEventTrackReachedStartResponse(avrcp, (AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalEventPlaybackPosChangedResponse(avrcp, (AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalEventBattStatusChangedResponse(avrcp, (AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalEventSystemStatusChangedResponse(avrcp, (AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalEventPlayerAppSettingChangedResponse(avrcp, (AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_ABORT_CONTINUING_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalAbortContinuingResponse(avrcp, (AVRCP_INTERNAL_ABORT_CONTINUING_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_NEXT_GROUP_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalNextGroupResponse(avrcp, (AVRCP_INTERNAL_NEXT_GROUP_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_PREVIOUS_GROUP_RES:
			if (profileState == avrcpConnected)
				avrcpHandleInternalPreviousGroupResponse(avrcp, (AVRCP_INTERNAL_PREVIOUS_GROUP_RES_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET:
			if (profileState == avrcpConnected)
				avrcpHandleNextContinuationPacket(avrcp, (AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET_T *) message);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_INFORM_BATTERY_STATUS_RES:
			if (profileState == avrcpConnected)
				prepareMetadataControlResponse(avrcp, ((AVRCP_INTERNAL_INFORM_BATTERY_STATUS_RES_T *) message)->response, AVRCP_INFORM_BATTERY_STATUS_PDU_ID);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_INFORM_CHAR_SET_RES:
			if (profileState == avrcpConnected)
				prepareMetadataControlResponse(avrcp, ((AVRCP_INTERNAL_INFORM_CHAR_SET_RES_T *) message)->response, AVRCP_INFORM_CHARACTER_SET_PDU_ID);			
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_GET_FEATURES:
			if (profileState == avrcpConnected)
				avrcpGetSupportedFeatures(avrcp, TRUE);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		case AVRCP_INTERNAL_GET_EXTENSIONS:
			if (profileState == avrcpConnected)
				avrcpGetProfileVersion(avrcp, TRUE);
			else 
				goto handle_unexpected_avrcp_msg;
			break;

		default:
			switch (id)
			{
			case MESSAGE_MORE_DATA:
            case AVRCP_INTERNAL_MESSAGE_MORE_DATA:
				if (profileState == avrcpConnected)
					avrcpHandleReceivedData(avrcp);
				else 
					goto handle_unexpected_avrcp_msg;
				break;

			/* Ignored primitives */
			case MESSAGE_MORE_SPACE:
			case MESSAGE_SOURCE_EMPTY:
				break;
			
			default:
				goto handle_unexpected_avrcp_msg; 
				break;
			}
		}
	}
	return;

handle_unexpected_cl_msg:
	handleUnexpected(avrcpUnexpectedClPrim, profileState, id);
	return;

handle_unexpected_avrcp_msg:
	handleUnexpected(avrcpUnexpectedAvrcpPrim, profileState, id);
	return;

}
