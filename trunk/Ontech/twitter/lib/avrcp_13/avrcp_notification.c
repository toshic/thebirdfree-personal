/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_notification.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_common.h"
#include "avrcp_notification_handler.h"
#include "avrcp_metadata_command_req.h"
#include "avrcp_private.h"


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void AvrcpRegisterNotification(AVRCP *avrcp, avrcp_supported_events event_id, uint32 playback_interval)
{
	uint8 params[5];
	avrcp_status_code status;

	params[0] = event_id & 0xFF;
	convertUint32ToUint8Values(&params[1], playback_interval);

	status = avrcpMetadataStatusCommand(avrcp, AVRCP_REGISTER_NOTIFICATION_PDU_ID, event_id + avrcp_playback_status - 1, sizeof(params), params, 0, 0, 0);

	if (status != avrcp_success)
	{
		avrcpSendRegisterNotificationFailCfm(avrcp, status, event_id);
	}
}
#endif

void clearRegisterNotification(AVRCP *avrcp, uint8 event)
{
	/* Clear the necessary registered event. */
	avrcp->registered_events &= ~(1<<(event-1));
}


void AvrcpEventPlaybackStatusChangedResponse(AVRCP *avrcp, avrcp_response_type response, avrcp_play_status play_status)
{
	/* Only send a response if this event was registered by the CT. */
	if (avrcp->registered_events & (1<<(EVENT_PLAYBACK_STATUS_CHANGED-1)))
	{
		MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES);
		message->response = response;
		message->play_status = play_status;
		MessageSend(&avrcp->task, AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES, message);
		if (response == avctp_response_changed)
			clearRegisterNotification(avrcp, EVENT_PLAYBACK_STATUS_CHANGED);
	}
	else
		PRINT(("AvrcpEventPlaybackStatusChangedResponse: Event not registered\n"));
}


void AvrcpEventTrackChangedResponse(AVRCP *avrcp, avrcp_response_type response, uint32 track_index_high, uint32 track_index_low)
{
	/* Only send a response if this event was registered by the CT. */
	if (avrcp->registered_events & (1<<(EVENT_TRACK_CHANGED-1)))
	{
		MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES);
		message->response = response;
		message->track_index_high = track_index_high;
		message->track_index_low = track_index_low;
		MessageSend(&avrcp->task, AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES, message);
		if (response == avctp_response_changed)
			clearRegisterNotification(avrcp, EVENT_TRACK_CHANGED);
	}
	else
		PRINT(("AvrcpEventTrackChangedResponse: Event not registered\n"));
}


void AvrcpEventTrackReachedEndResponse(AVRCP *avrcp, avrcp_response_type response)
{
	/* Only send a response if this event was registered by the CT. */
	if (avrcp->registered_events & (1<<(EVENT_TRACK_END-1)))
	{
		MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES);
		message->response = response;
		MessageSend(&avrcp->task, AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES, message);
		if (response == avctp_response_changed)
			clearRegisterNotification(avrcp, EVENT_TRACK_END);
	}
	else
		PRINT(("AvrcpEventTrackReachedEndResponse: Event not registered\n"));
}


void AvrcpEventTrackReachedStartResponse(AVRCP *avrcp, avrcp_response_type response)
{
	/* Only send a response if this event was registered by the CT. */
	if (avrcp->registered_events & (1<<(EVENT_TRACK_START-1)))
	{
		MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES);
		message->response = response;
		MessageSend(&avrcp->task, AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES, message);
		if (response == avctp_response_changed)
			clearRegisterNotification(avrcp, EVENT_TRACK_START);
	}
	else
		PRINT(("AvrcpEventTrackReachedStartResponse: Event not registered\n"));
}


void AvrcpEventPlaybackPosChangedResponse(AVRCP *avrcp, avrcp_response_type response, uint32 playback_pos)
{
	/* Only send a response if this event was registered by the CT. */
	if (avrcp->registered_events & (1<<(EVENT_PLAYBACK_POS_CHANGED-1)))
	{
		MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES);
		message->response = response;
		message->playback_pos = playback_pos;
		MessageSend(&avrcp->task, AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES, message);
		if (response == avctp_response_changed)
			clearRegisterNotification(avrcp, EVENT_PLAYBACK_POS_CHANGED);
	}
	else
		PRINT(("AvrcpEventPlaybackPosChangedResponse: Event not registered\n"));
}


void AvrcpEventBattStatusChangedResponse(AVRCP *avrcp, avrcp_response_type response, avrcp_battery_status battery_status)
{
	/* Only send a response if this event was registered by the CT. */
	if (avrcp->registered_events & (1<<(EVENT_BATTERY_STATUS_CHANGED-1)))
	{
		MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES);
		message->response = response;
		message->battery_status = battery_status;
		MessageSend(&avrcp->task, AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES, message);
		if (response == avctp_response_changed)
			clearRegisterNotification(avrcp, EVENT_BATTERY_STATUS_CHANGED);
	}
	else
		PRINT(("AvrcpEventBattStatusChangedResponse: Event not registered\n"));
}


void AvrcpEventSystemStatusChangedResponse(AVRCP *avrcp, avrcp_response_type response, avrcp_system_status system_status)
{
	/* Only send a response if this event was registered by the CT. */
	if (avrcp->registered_events & (1<<(EVENT_SYSTEM_STATUS_CHANGED-1)))
	{
		MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES);
		message->response = response;
		message->system_status = system_status;
		MessageSend(&avrcp->task, AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES, message);
		if (response == avctp_response_changed)
			clearRegisterNotification(avrcp, EVENT_SYSTEM_STATUS_CHANGED);
	}
	else
		PRINT(("AvrcpEventSystemStatusChangedResponse: Event not registered\n"));
}


void AvrcpEventPlayerAppSettingChangedResponse(AVRCP *avrcp, avrcp_response_type response, uint16 size_attributes, Source attributes)
{
	/* Only send a response if this event was registered by the CT. */
	if (avrcp->registered_events & (1<<(EVENT_PLAYER_SETTING_CHANGED-1)))
	{
		MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES);
		message->response = response;
		message->size_attributes = size_attributes;
		message->attributes = attributes;
		MessageSend(&avrcp->task, AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES, message);
		if (response == avctp_response_changed)
			clearRegisterNotification(avrcp, EVENT_PLAYER_SETTING_CHANGED);
	}
	else
		PRINT(("AvrcpEventPlayerAppSettingChangedResponse: Event not registered\n"));
}
