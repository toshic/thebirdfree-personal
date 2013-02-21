/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_player_app_settings_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_metadata_transfer.h"
#include "avrcp_common.h"
#include "avrcp_continuation_handler.h"
#include "avrcp_notification_handler.h"
#include "avrcp_private.h"
#include "avrcp_signal_handler.h"

#include <source.h>
#include <stdlib.h>


void sendRegisterNotificationResponse(AVRCP *avrcp, avrcp_response_type response, uint16 size_mandatory, uint8 *mandatory, uint16 size_attributes, Source attributes)
{
	uint16 size_mandatory_data = 0;
	uint8 *mandatory_data = 0;
	uint16 param_length = 0;
	avrcp_response_type resp = response;

	standardiseCtypeNotify(&resp);

	if ((resp == avctp_response_interim) || (resp == avctp_response_changed))
	{
		mandatory_data = mandatory;
		size_mandatory_data = size_mandatory;
		param_length = size_mandatory_data + size_attributes;
	}
	else
	{
		if (mandatory)
			free (mandatory);
		mandatory_data = insertRejectCode(&resp, &size_mandatory_data, &param_length);
	}

	/* Check if Metadata packet has to be fragmented to fit into 512 AV/C frame size restriction. */
    if ((param_length + 13) > 515)
    {        
        sendMetadataResponse(avrcp, resp, AVRCP_REGISTER_NOTIFICATION_PDU_ID, attributes, avrcp_packet_type_start, 512, size_mandatory_data, mandatory_data);

		/* There are more fragments to be sent, store the data for the following fragments. */
		avrcpStoreNextContinuationPacket(avrcp, attributes, param_length, AVRCP_REGISTER_NOTIFICATION_PDU_ID, 2, resp, avrcp->notify_transaction_label[EVENT_PLAYER_SETTING_CHANGED-1]);
    }
    else
	{
        sendMetadataResponse(avrcp,  resp, AVRCP_REGISTER_NOTIFICATION_PDU_ID, attributes, avrcp_packet_type_single, param_length, size_mandatory_data, mandatory_data);
		if (attributes)
			SourceEmpty(attributes);
	}

    avrcpHandleReceivedData(avrcp);
}


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void avrcpSendNotification(AVRCP *avrcp, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	uint16 packet_type = ptr[0] & AVCTP0_PACKET_TYPE_MASK;
	uint16 transaction = (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F;
    uint16 event_id;
	bool source_processed = TRUE;
	uint16 data_start=0;
	uint16 data_len = 0;
    
	if (packet_type == AVCTP0_PACKET_TYPE_SINGLE)
	{
		data_start = 13;
		event_id = ptr[data_start];
		data_start++;
	}
	else if (packet_type == AVCTP0_PACKET_TYPE_START)
	{
		data_start = 14;
		event_id = ptr[data_start];
		data_start++;
	}
	else if ((packet_type == AVCTP0_PACKET_TYPE_CONTINUE) || (packet_type == AVCTP0_PACKET_TYPE_END))
	{
		data_start = 11;
		event_id = avrcp->last_metadata_notification;
	}
	else
	{
		AvrcpSourceProcessed(avrcp);
		return;
	}
	
	data_len = packet_size - data_start;

	if (packet_type == AVCTP0_PACKET_TYPE_END)
		avrcp->last_metadata_notification = 0;
	else
		avrcp->last_metadata_notification = event_id;
	
	switch (event_id)
	{
	case avrcp_event_playback_status_changed:
		{
			MAKE_AVRCP_MESSAGE(AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND);
			message->avrcp = avrcp;
			message->transaction = transaction;
			
			/* EVENT_PLAYBACK_STATUS_CHANGED Length expecetd is 1 */
			if(data_len >= 1){
				message->response = ptr[3];
				message->play_status = ptr[data_start];
			}
			else
			{
				message->response = avrcp_rejected_invalid_content;
				message->play_status = 0xFF;
			}
			MessageSend(avrcp->clientTask, AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND, message);
			
		}
		break;
	case avrcp_event_track_changed:
		{
			MAKE_AVRCP_MESSAGE(AVRCP_EVENT_TRACK_CHANGED_IND);
			message->avrcp = avrcp;
			message->transaction = transaction;
			
			/*
			 * If the Packet comes in continuation or end packet fragmented, it may 
			 * not process
			 * Identifier is 8 octets long
			 */
			if(data_len >= 8)
			{
				message->response = ptr[3];
				message->track_index_high = convertUint8ValuesToUint32(&ptr[data_start]);				
				message->track_index_low = convertUint8ValuesToUint32(&ptr[data_start+4]);				
			}
			else
			{
				message->response = avrcp_rejected_invalid_content;
				message->track_index_high = 0;
				message->track_index_low = 0;
			}
			MessageSend(avrcp->clientTask, AVRCP_EVENT_TRACK_CHANGED_IND, message);
		}
		break;
	case avrcp_event_track_reached_end:
		{
			MAKE_AVRCP_MESSAGE(AVRCP_EVENT_TRACK_REACHED_END_IND);
			message->avrcp = avrcp;
			message->transaction = transaction;
			message->response = ptr[3];
			MessageSend(avrcp->clientTask, AVRCP_EVENT_TRACK_REACHED_END_IND, message);
		}
		break;
	case avrcp_event_track_reached_start:
		{
			MAKE_AVRCP_MESSAGE(AVRCP_EVENT_TRACK_REACHED_START_IND);
			message->avrcp = avrcp;
			message->transaction = transaction;
			message->response = ptr[3];
			MessageSend(avrcp->clientTask, AVRCP_EVENT_TRACK_REACHED_START_IND, message);
		}
		break;
	case avrcp_event_playback_pos_changed:
		{
			MAKE_AVRCP_MESSAGE(AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND);
			message->avrcp = avrcp;
			message->transaction = transaction;
			/*
			 * If the Packet comes in continuation or end packet fragmented, it may 
			 * not process
			 * Identifier is 8 octets long
			 */
			if(data_len >= 4)
			{
				message->response = ptr[3];
				message->playback_pos = convertUint8ValuesToUint32(&ptr[data_start]);
			}
			else
			{
				message->response = avrcp_rejected_invalid_content;
				message->playback_pos = 0;
			}
			MessageSend(avrcp->clientTask, AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND, message);
		}
		break;
	case avrcp_event_batt_status_changed:
		{
			MAKE_AVRCP_MESSAGE(AVRCP_EVENT_BATT_STATUS_CHANGED_IND);
			message->avrcp = avrcp;
			message->transaction = transaction;
			if(data_len >= 1){
				message->response = ptr[3];
				message->battery_status = ptr[data_start];
			}
			else
			{
				message->response = avrcp_rejected_invalid_content;
				message->battery_status = 0;
			}
			MessageSend(avrcp->clientTask, AVRCP_EVENT_BATT_STATUS_CHANGED_IND, message);
		}
		break;
	case avrcp_event_system_status_changed:
		{
			MAKE_AVRCP_MESSAGE(AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND);
			message->avrcp = avrcp;
			message->transaction = transaction;
			if(data_len >= 1){
				message->response = ptr[3];
				message->system_status = ptr[data_start];
			}
			else
			{
				message->system_status = 0;
				message->response = avrcp_rejected_invalid_content;
			}
			MessageSend(avrcp->clientTask, AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND, message);
		}
		break;
	case avrcp_event_player_app_setting_changed:
		{
			uint16 total_packets = 1;
			MAKE_AVRCP_MESSAGE(AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND);

			/*
			 * If the Packet comes in continuation or end packet fragmented, it may 
			 * not process
			 * 
			 */
			if (packet_type == AVCTP0_PACKET_TYPE_START)
				total_packets = ptr[1];
			
			
			message->avrcp = avrcp;
			message->transaction = transaction;
			message->no_packets = total_packets;
			message->ctp_packet_type = packet_type;
			message->metadata_packet_type = AVCTP0_PACKET_TYPE_SINGLE;

			message->response = ptr[3];

			if(data_len >= 3){
				message->number_of_attributes = ptr[data_start];
				source_processed = FALSE;
				message->data_offset = data_start + 1;
				message->attributes = source;
				message->size_attributes = data_len - 1;
			}
			else
			{
				message->number_of_attributes = 0;
				message->data_offset = 0;
				message->size_attributes = 0;
				message->attributes = 0;
				message->response = avrcp_rejected_invalid_content;
			}

			MessageSend(avrcp->clientTask, AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND, message);

		}
		break;
	default:
		break;
	}

	avrcp->pending = avrcp_none;

	/* Drop the source here if everyone is finished with it. */
	if (source_processed)
		avrcpSourceProcessed(avrcp);
	
}

/*****************************************************************************/
void avrcpSendRegisterNotificationFailCfm(AVRCP *avrcp, avrcp_status_code status, avrcp_supported_events event_id)
{
    MAKE_AVRCP_MESSAGE(AVRCP_REGISTER_NOTIFICATION_CFM);

    message->avrcp = avrcp;
	message->transaction = 0;
    message->status = status;
	message->event_id = event_id;

	MessageSend(avrcp->clientTask, AVRCP_REGISTER_NOTIFICATION_CFM, message);
}
#endif

/*****************************************************************************/
void avrcpHandleRegisterNotificationCommand(AVRCP *avrcp, uint16 transaction, const uint8 *ptr)
{
	uint8 event = ptr[13];
	MAKE_AVRCP_MESSAGE(AVRCP_REGISTER_NOTIFICATION_IND);
    message->avrcp = avrcp;
	message->transaction = transaction;
	message->event_id = event;
	message->playback_interval = convertUint8ValuesToUint32(&ptr[14]);
    MessageSend(avrcp->clientTask, AVRCP_REGISTER_NOTIFICATION_IND, message);

	if ((event >= EVENT_PLAYBACK_STATUS_CHANGED) && (event <= EVENT_PLAYER_SETTING_CHANGED))
	{
		/* Store which event has been registered for. */
		avrcp->registered_events |= (1<<(event-1));
		/* Store the transaction label associated with the event. */
		avrcp->notify_transaction_label[event-1] = transaction;
	}
}


/*****************************************************************************/
void avrcpHandleInternalEventPlaybackStatusChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES_T *res)
{
	uint16 size_mandatory_data = 2;
	uint8 *mandatory_data = (uint8 *) malloc(size_mandatory_data);
	mandatory_data[0] = avrcp_event_playback_status_changed;
	mandatory_data[1] = res->play_status;

	sendRegisterNotificationResponse(avrcp, res->response, size_mandatory_data, mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventTrackChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES_T *res)
{
	uint16 size_mandatory_data = 9;
	uint8 *mandatory_data = (uint8 *) malloc(size_mandatory_data);
	mandatory_data[0] = avrcp_event_track_changed;

	convertUint32ToUint8Values(&mandatory_data[1], res->track_index_high);
	convertUint32ToUint8Values(&mandatory_data[5], res->track_index_low);

	sendRegisterNotificationResponse(avrcp, res->response, size_mandatory_data, mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventTrackReachedEndResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES_T *res)
{
	uint16 size_mandatory_data = 1;
	uint8 *mandatory_data = (uint8 *) malloc(size_mandatory_data);
	mandatory_data[0] = avrcp_event_track_reached_end;

	sendRegisterNotificationResponse(avrcp, res->response, size_mandatory_data, mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventTrackReachedStartResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES_T *res)
{
	uint16 size_mandatory_data = 1;
	uint8 *mandatory_data = (uint8 *) malloc(size_mandatory_data);
	mandatory_data[0] = avrcp_event_track_reached_start;

	sendRegisterNotificationResponse(avrcp, res->response, size_mandatory_data, mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventPlaybackPosChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES_T *res)
{
	uint16 size_mandatory_data = 5;
	uint8 *mandatory_data = (uint8 *) malloc(size_mandatory_data);
	mandatory_data[0] = avrcp_event_playback_pos_changed;
	convertUint32ToUint8Values(&mandatory_data[1], res->playback_pos);

	sendRegisterNotificationResponse(avrcp, res->response, size_mandatory_data, mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventBattStatusChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES_T *res)
{
	uint16 size_mandatory_data = 2;
	uint8 *mandatory_data = (uint8 *) malloc(size_mandatory_data);
	mandatory_data[0] = avrcp_event_batt_status_changed;
	mandatory_data[1] = res->battery_status;

	sendRegisterNotificationResponse(avrcp, res->response, size_mandatory_data, mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventSystemStatusChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES_T *res)
{
	uint16 size_mandatory_data = 2;
	uint8 *mandatory_data = (uint8 *) malloc(size_mandatory_data);
	mandatory_data[0] = avrcp_event_system_status_changed;
	mandatory_data[1] = res->system_status;

	sendRegisterNotificationResponse(avrcp, res->response, size_mandatory_data, mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventPlayerAppSettingChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES_T *res)
{
	uint16 size_mandatory_data = 2;
	uint8 *mandatory_data = (uint8 *) malloc(size_mandatory_data);
	mandatory_data[0] = avrcp_event_player_app_setting_changed;
	mandatory_data[1] = res->size_attributes / 2;

	sendRegisterNotificationResponse(avrcp, res->response, size_mandatory_data, mandatory_data, res->size_attributes, res->attributes);
}



