/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_notification_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_NOTIFICATION_HANDLER_H_
#define AVRCP_NOTIFICATION_HANDLER_H_


#include "avrcp_private.h"


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME	
	avrcpSendNotification

DESCRIPTION
	Send indication of notification event up to the app.
*/
void avrcpSendNotification(AVRCP *avrcp, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpSendRegisterNotificationFailCfm

DESCRIPTION
	Send an AVRCP_REGISTER_NOTIFICATION_CFM failure message to the client.
*/
void avrcpSendRegisterNotificationFailCfm(AVRCP *avrcp, avrcp_status_code status, avrcp_supported_events event_id);
#endif

/****************************************************************************
NAME	
	avrcpHandleRegisterNotificationCommand

DESCRIPTION
	Handle Register Notification command received from the CT.
*/
void avrcpHandleRegisterNotificationCommand(AVRCP *avrcp, uint16 transaction, const uint8 *ptr);


/****************************************************************************
NAME	
	avrcpHandleInternalEventPlaybackStatusChangedResponse

DESCRIPTION
	Handle internal message AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES.
*/
void avrcpHandleInternalEventPlaybackStatusChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleInternalEventTrackChangedResponse

DESCRIPTION
	Handle internal message AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES.
*/
void avrcpHandleInternalEventTrackChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleInternalEventTrackReachedEndResponse

DESCRIPTION
	Handle internal message AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES.
*/
void avrcpHandleInternalEventTrackReachedEndResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleInternalEventTrackReachedStartResponse

DESCRIPTION
	Handle internal message AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES.
*/
void avrcpHandleInternalEventTrackReachedStartResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleInternalEventPlaybackPosChangedResponse

DESCRIPTION
	Handle internal message AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES.
*/
void avrcpHandleInternalEventPlaybackPosChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleInternalEventBattStatusChangedResponse

DESCRIPTION
	Handle internal message AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES.
*/
void avrcpHandleInternalEventBattStatusChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleInternalEventSystemStatusChangedResponse

DESCRIPTION
	Handle internal message AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES.
*/
void avrcpHandleInternalEventSystemStatusChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleInternalEventPlayerAppSettingChangedResponse

DESCRIPTION
	Handle internal message AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES.
*/
void avrcpHandleInternalEventPlayerAppSettingChangedResponse(AVRCP *avrcp, AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES_T *res);

void sendRegisterNotificationResponse(AVRCP *avrcp, avrcp_response_type response, uint16 size_mandatory, uint8 *mandatory, uint16 size_attributes, Source attributes);
void clearRegisterNotification(AVRCP *avrcp, uint8 event);

#endif /* AVRCP_NOTIFICATION_HANDLER_H_ */
