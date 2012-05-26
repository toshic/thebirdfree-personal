/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_notification_handler.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_NOTIFICATION_HANDLER_H_
#define AVRCP_NOTIFICATION_HANDLER_H_

#include "avrcp_common.h"

/* Macros */
#define clearRegisterNotification(avrcp, event) \
            avrcp->registered_events &= ~(1 << (event))

#define isEventRegistered(avrcp,event) \
            avrcp->registered_events & (1<< (event))

#define GetNotificationTransaction(avrcp,event) \
            avrcp->notify_transaction_label[(event)-1]
        

/****************************************************************************
NAME    
    avrcpSendNotification

DESCRIPTION
    Send indication of notification event up to the app.
*/
bool avrcpSendNotification( AVRCP *avrcp,  
                            avrcp_response_type response, 
                            const uint8* ptr, 
                            uint16 packet_size);

/****************************************************************************
NAME    
    avrcpSendRegisterNotificationFailCfm

DESCRIPTION
    Send an AVRCP_REGISTER_NOTIFICATION_CFM failure message to the client.
*/
void avrcpSendRegisterNotificationFailCfm(AVRCP *avrcp, 
                                         avrcp_status_code status,
                                         avrcp_supported_events event_id);


/****************************************************************************
NAME    
    avrcpHandleRegisterNotificationCommand

DESCRIPTION
    Handle Register Notification command received from the CT.
*/
void avrcpHandleRegisterNotificationCommand(AVRCP *avrcp, const uint8 *ptr);


/****************************************************************************
NAME    
    avrcpHandleInternalEventPlaybackStatusChangedResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES.
*/
void avrcpHandleInternalEventPlaybackStatusChangedResponse(AVRCP *avrcp,
              AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalEventTrackChangedResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES.
*/
void avrcpHandleInternalEventTrackChangedResponse(AVRCP *avrcp,
               AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalEventTrackReachedEndResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES.
*/
void avrcpHandleInternalEventTrackReachedEndResponse(AVRCP *avrcp,
              AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalEventTrackReachedStartResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES.
*/
void avrcpHandleInternalEventTrackReachedStartResponse(AVRCP *avrcp,
              AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalEventPlaybackPosChangedResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES.
*/
void avrcpHandleInternalEventPlaybackPosChangedResponse(AVRCP *avrcp,
              AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalEventBattStatusChangedResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES.
*/
void avrcpHandleInternalEventBattStatusChangedResponse(AVRCP *avrcp,
              AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalEventSystemStatusChangedResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES.
*/
void avrcpHandleInternalEventSystemStatusChangedResponse(AVRCP *avrcp,
              AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalEventPlayerAppSettingChangedResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES.
*/
void avrcpHandleInternalEventPlayerAppSettingChangedResponse(AVRCP *avrcp,
             AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES_T *res);

/****************************************************************************/
void sendRegisterNotificationResponse(AVRCP *avrcp, 
                                avrcp_response_type response, 
                                uint16 size_mandatory, 
                                uint8 *mandatory, 
                                uint16 size_attributes, 
                                Source attributes);

/****************************************************************************
NAME    
    avrcpSendGetPlayStatusCfm

DESCRIPTION
    Send an AVRCP_GET_PLAY_STATUS_CFM message to the client.
*/
void avrcpSendGetPlayStatusCfm(AVRCP *avrcp, 
                            avrcp_status_code status, 
                            uint32 song_length, 
                            uint32 song_elapsed, 
                            avrcp_play_status play_status, 
                            uint8 transaction); 


/****************************************************************************
NAME    
    avrcpHandleGetPlayStatusResponse

DESCRIPTION
    Handle Get Play Status response received from the TG.
*/
void avrcpHandleGetPlayStatusResponse(AVRCP *avrcp, 
                                      avrcp_status_code status,
                                      uint16 transaction, 
                                      const uint8 *ptr, 
                                      uint16 packet_size);


/****************************************************************************
NAME    
    avrcpHandleInternalGetPlayStatusResponse

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_GET_PLAY_STATUS_RES message.
*/
void avrcpHandleInternalGetPlayStatusResponse(AVRCP *avrcp,
               AVRCP_INTERNAL_GET_PLAY_STATUS_RES_T *res);


#endif /* AVRCP_NOTIFICATION_HANDLER_H_ */
