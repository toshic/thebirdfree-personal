/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_notification_handler.c

DESCRIPTION
This file defines the the functions for hanlding Notifications feature, which includes
    - GetPlayStatus
    - RegisterNotification
    - EVENT_PLAY_STATUS_CHANGED
    - EVENT_TRACK_CHANGED
    - EVENT_TRACK_REACHED_END
    - EVENT_TRACK_REACHED_START
    - EVENT_PLAYBACK_POS_CHANGED
    - EVENT_BATTERY_STATUS_CHANGED
    - EVENT_SYSTEM_STATUS_CHANGED
    - EVENT_PLAYER_APPLICATION_SETTINGS_CHANGED    
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "avrcp_notification_handler.h"
#include "avrcp_continuation_handler.h"
#include "avrcp_metadata_transfer.h"


void sendRegisterNotificationResponse(  AVRCP               *avrcp, 
                                        avrcp_response_type resp, 
                                        uint16              size_mandatory, 
                                        uint8               *mandatory, 
                                        uint16              size_attributes, 
                                        Source              attributes)
{
    /* 1 byte error status code as data for rejected response */ 
    uint8  error_data[1];
    uint16 param_length = 1;
    avrcp_packet_type  metadata_packet_type =  avrcp_packet_type_single;

   /* Get the error status code */
    error_data[0] = avrcpGetErrorStatusCode(&resp, AVRCP0_CTYPE_NOTIFY);

    if ((resp == avctp_response_interim) || (resp == avctp_response_changed))
    {
        param_length = size_mandatory + size_attributes;
    }
    else
    {
        size_mandatory = 1; /*  1 octet length only for error code */
        mandatory = &error_data[0];
    }

    
    if (param_length > AVRCP_AVC_MAX_DATA_SIZE)
    {        
        /* There are more fragments to be sent, 
            store the data for the following fragments. */

        /* madatory[0] contains the event ID */
        avrcpStoreNextContinuationPacket(avrcp, attributes, 
                                param_length-AVRCP_AVC_MAX_DATA_SIZE, 
                                AVRCP_REGISTER_NOTIFICATION_PDU_ID, 
                                resp, 
                                GetNotificationTransaction(avrcp, mandatory[0]));

        param_length = AVRCP_AVC_MAX_DATA_SIZE;
        metadata_packet_type = avrcp_packet_type_start;
    }
    
    avrcpSendMetadataResponse(avrcp,  resp, AVRCP_REGISTER_NOTIFICATION_PDU_ID,
                              attributes, metadata_packet_type, param_length, 
                              size_mandatory, mandatory);

}


/*****************************************************************************/
bool avrcpSendNotification( AVRCP *avrcp,  
                            avrcp_response_type response, 
                            const uint8* ptr, 
                            uint16 packet_size)
{
    uint16 event_id;
    bool source_processed = TRUE;
    uint16 data_len = 0;
    uint16 data_start=0;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
    uint16 transaction = (avrcp->av_msg[0] & AVCTP_TRANSACTION_MASK) >>
                         AVCTP0_TRANSACTION_SHIFT;
#endif
    
    event_id = ptr[data_start];
    data_start=1;
    data_len = packet_size-data_start;

    switch (event_id)
    {
    case avrcp_event_playback_status_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND);
            message->avrcp = avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif
            
            /* EVENT_PLAYBACK_STATUS_CHANGED Length expected is 1 */
            if(data_len >= 1){
                message->response = response;
                message->play_status = ptr[data_start];
            }
            else
            {
                message->response = avrcp_rejected_invalid_content;
                message->play_status = 0xFF;
            }
            MessageSend(avrcp->clientTask,
                         AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND, message);
            
        }
        break;
    case avrcp_event_track_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_TRACK_CHANGED_IND);
            message->avrcp = avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif
            
            /*
             * If the Packet comes in continuation or end packet
             * fragmented, it may  not process
             * Identifier is 8 octets long
             */
            if(data_len >= 8)
            {
                message->response = response;
                message->track_index_high =
                               convertUint8ValuesToUint32(&ptr[data_start]);
                message->track_index_low = 
                               convertUint8ValuesToUint32(&ptr[data_start+4]);
            }
            else
            {
                message->response = avrcp_rejected_invalid_content;
                message->track_index_high = 0;
                message->track_index_low = 0;
            }
            MessageSend(avrcp->clientTask, 
                        AVRCP_EVENT_TRACK_CHANGED_IND, message);
        }
        break;
    case avrcp_event_track_reached_end:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_TRACK_REACHED_END_IND);
            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif
            message->response = response;
            MessageSend(avrcp->clientTask, 
                        AVRCP_EVENT_TRACK_REACHED_END_IND, message);
        }
        break;
    case avrcp_event_track_reached_start:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_TRACK_REACHED_START_IND);
            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif
            message->response = response;
            MessageSend(avrcp->clientTask, 
                        AVRCP_EVENT_TRACK_REACHED_START_IND, message);
        }
        break;
    case avrcp_event_playback_pos_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND);
            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif
            /*
             * If the Packet comes in continuation or end packet fragmented,
             * it may  not process the Identifier which is 8 octets long
             */
            if(data_len >= 4)
            {
                message->response = response;
                message->playback_pos =
                             convertUint8ValuesToUint32(&ptr[data_start]);
            }
            else
            {
                message->response = avrcp_rejected_invalid_content;
                message->playback_pos = 0;
            }
            MessageSend(avrcp->clientTask,
                        AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND, message);
        }
        break;
    case avrcp_event_batt_status_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_BATT_STATUS_CHANGED_IND);
            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif
            if(data_len >= 1){
                message->response = response;
                message->battery_status = ptr[data_start];
            }
            else
            {
                message->response = avrcp_rejected_invalid_content;
                message->battery_status = 0;
            }
            MessageSend(avrcp->clientTask,
                        AVRCP_EVENT_BATT_STATUS_CHANGED_IND, message);
        }
        break;
    case avrcp_event_system_status_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND);
            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif

            if(data_len >= 1){
                message->response = response;
                message->system_status = ptr[data_start];
            }
            else
            {
                message->system_status = 0;
                message->response = avrcp_rejected_invalid_content;
            }
            MessageSend(avrcp->clientTask,
                        AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND, message);
        }
        break;
    case avrcp_event_player_app_setting_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND);

            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
            message->no_packets = 0;
            message->ctp_packet_type = AVCTP0_PACKET_TYPE_SINGLE;
            message->data_offset = 0;
#endif
            message->metadata_packet_type = AVCTP0_PACKET_TYPE_SINGLE;

            message->response = response;

            if(data_len >= 3){
                message->number_of_attributes = ptr[data_start];
                source_processed = FALSE;
                message->attributes =avrcpSourceFromConstData(avrcp, 
                                    ptr+1, data_len-1);
                message->size_attributes = data_len - 1;
            }
            else
            {
                message->number_of_attributes = 0;
                message->size_attributes = 0;
                message->attributes = 0;
                message->response = avrcp_rejected_invalid_content;
            }

            MessageSend(avrcp->clientTask,
                        AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND, message);

        }
        break;

     case avrcp_event_volume_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_VOLUME_CHANGED_IND);

            message->avrcp = avrcp;

            if(data_len >= 1){
                message->response = response;
                message->volume = ptr[data_start];
            }
            else
            {
                message->volume = 0;
                message->response = avrcp_rejected_invalid_content;
            }
            MessageSend(avrcp->clientTask,
                        AVRCP_EVENT_VOLUME_CHANGED_IND, message);
        }   
        break;
    default:
        /* Unknown event. ignore */
        break;
    }

    /* Clear the pending flag only if the library was waiting for 
       the corresponding REGISTER_NOTIFICATION response. 
       otherwise ignore */
    if((event_id + avrcp_events_start_dummy) == avrcp->pending)
    {
        (void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
        avrcp->pending = avrcp_none;
    }

    return source_processed;

}


/*****************************************************************************/
void avrcpSendRegisterNotificationFailCfm(AVRCP *avrcp,
                                         avrcp_status_code status,
                                         avrcp_supported_events event_id)
{
    MAKE_AVRCP_MESSAGE(AVRCP_REGISTER_NOTIFICATION_CFM);

    message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
    message->transaction = 0;
#endif
    message->status = status;
    message->event_id = event_id;

    MessageSend(avrcp->clientTask, AVRCP_REGISTER_NOTIFICATION_CFM, message);
}


/*****************************************************************************/
void avrcpHandleRegisterNotificationCommand(AVRCP *avrcp, const uint8 *ptr)
{
    uint8 event = ptr[0];

    if (((event >= EVENT_PLAYBACK_STATUS_CHANGED) && 
        (event <= EVENT_PLAYER_SETTING_CHANGED)) ||
        (event == EVENT_VOLUME_CHANGED))
    {

        MAKE_AVRCP_MESSAGE(AVRCP_REGISTER_NOTIFICATION_IND);
        message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
        message->transaction = avrcp->rsp_transaction_label;
#endif
        message->event_id = event;
        message->playback_interval = convertUint8ValuesToUint32(&ptr[1]);

        /* Store which event has been registered for. */
        avrcp->registered_events |= (1<<event);
        /* Store the transaction label associated with the event. */
        avrcp->notify_transaction_label[event-1] = avrcp->rsp_transaction_label;

        MessageSend(avrcp->clientTask,AVRCP_REGISTER_NOTIFICATION_IND,message);
    }
    else
    {
        avrcpSendRejectMetadataResponse(avrcp, 
                                        avctp_response_not_implemented,
                                        AVRCP_REGISTER_NOTIFICATION_PDU_ID);
    }
}

/*****************************************************************************/
void avrcpHandleInternalEventPlaybackStatusChangedResponse(AVRCP *avrcp,
                     AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES_T *res)
{
    /* 2 Byte length data for Play back status changed event */
    uint8 mandatory_data[AVRCP_EVENT_STATUS_HDR_SIZE];
    mandatory_data[0] = avrcp_event_playback_status_changed;
    mandatory_data[1] = res->play_status;

    sendRegisterNotificationResponse(avrcp, res->response,
                                     AVRCP_EVENT_STATUS_HDR_SIZE,
                                     mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventTrackChangedResponse(AVRCP *avrcp,
                     AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES_T *res)
{
    uint8 mandatory_data[AVRCP_EVENT_TRACK_HDR_SIZE];

    mandatory_data[0] = avrcp_event_track_changed;
    convertUint32ToUint8Values(&mandatory_data[1], res->track_index_high);
    convertUint32ToUint8Values(&mandatory_data[5], res->track_index_low);

    sendRegisterNotificationResponse(avrcp, res->response, 
                                AVRCP_EVENT_TRACK_HDR_SIZE,
                                mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventTrackReachedEndResponse(AVRCP *avrcp,
                     AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES_T *res)
{
    uint8 mandatory_data[AVRCP_EVENT_DEFAULT_HDR_SIZE];
    mandatory_data[0] = avrcp_event_track_reached_end;

    sendRegisterNotificationResponse(avrcp, res->response, 
                                    AVRCP_EVENT_DEFAULT_HDR_SIZE, 
                                    mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventTrackReachedStartResponse(AVRCP *avrcp, 
                        AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES_T *res)
{
    uint8 mandatory_data[AVRCP_EVENT_DEFAULT_HDR_SIZE];
    mandatory_data[0] = avrcp_event_track_reached_start;

    sendRegisterNotificationResponse(avrcp, res->response,
                                     AVRCP_EVENT_DEFAULT_HDR_SIZE, 
                                     mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventPlaybackPosChangedResponse(AVRCP *avrcp,
                 AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES_T *res)
{
    /* 5 Byte length data for play back position changed response */
    uint8 mandatory_data[AVRCP_EVENT_POS_HDR_SIZE];
    mandatory_data[0] = avrcp_event_playback_pos_changed;
    convertUint32ToUint8Values(&mandatory_data[1], res->playback_pos);

    sendRegisterNotificationResponse(avrcp, res->response, 
                                    AVRCP_EVENT_POS_HDR_SIZE, 
                                    mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventBattStatusChangedResponse(AVRCP *avrcp,
              AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES_T *res)
{
    uint8 mandatory_data[AVRCP_EVENT_STATUS_HDR_SIZE];
    mandatory_data[0] = avrcp_event_batt_status_changed;
    mandatory_data[1] = res->battery_status;

    sendRegisterNotificationResponse(avrcp, res->response,
                                     AVRCP_EVENT_STATUS_HDR_SIZE, 
                                     mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventSystemStatusChangedResponse(AVRCP *avrcp,
               AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES_T *res)
{
    uint8 mandatory_data[AVRCP_EVENT_STATUS_HDR_SIZE];
    mandatory_data[0] = avrcp_event_system_status_changed;
    mandatory_data[1] = res->system_status;

    sendRegisterNotificationResponse(avrcp, res->response,
                                     AVRCP_EVENT_STATUS_HDR_SIZE, 
                                     mandatory_data, 0, 0);
}


/*****************************************************************************/
void avrcpHandleInternalEventPlayerAppSettingChangedResponse(AVRCP *avrcp,
             AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES_T *res)
{
    uint8 mandatory_data[AVRCP_EVENT_STATUS_HDR_SIZE];
    mandatory_data[0] = avrcp_event_player_app_setting_changed;
    mandatory_data[1] = res->size_attributes / 2;

    sendRegisterNotificationResponse(avrcp, res->response,
                                     AVRCP_EVENT_STATUS_HDR_SIZE,
                                     mandatory_data, res->size_attributes,
                                     res->attributes);
}


/*****************************************************************************/
void avrcpSendGetPlayStatusCfm(AVRCP *avrcp, 
                               avrcp_status_code status, 
                               uint32 song_length, 
                               uint32 song_elapsed, 
                               avrcp_play_status play_status, 
                               uint8 transaction) 
{
    MAKE_AVRCP_MESSAGE(AVRCP_GET_PLAY_STATUS_CFM);

    message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
    message->transaction = transaction;
#endif
    message->status = status;
    message->song_length = song_length;
    message->song_elapsed = song_elapsed;
    message->play_status = play_status;

    MessageSend(avrcp->clientTask, AVRCP_GET_PLAY_STATUS_CFM, message);
}


/*****************************************************************************/
void avrcpHandleGetPlayStatusResponse(AVRCP             *avrcp, 
                                      avrcp_status_code status,
                                      uint16            transaction, 
                                      const  uint8      *ptr, 
                                      uint16            packet_size)
{
    uint32 song_length = 0, song_elapsed = 0;
    uint8 play_status =0;

    if(status == avrcp_rejected)
    {
        /* next data is error status code */
        status = ptr[0] + avrcp_rejected_invalid_pdu;
    }

    /* packet should be greater than 9 bytes */
    if(packet_size < AVRCP_GET_PLAY_STATUS_SIZE)
    {
        AVRCP_INFO(("avrcpHandleGetPlayStatusResponse: Invalid Length\n",
                    packet_length)); 
    }
    else
    {
        /* Song length in 0-3 bytes */
        song_length = convertUint8ValuesToUint32(&ptr[0]);

        /* song elapsed in 4-7 bytes */
        song_elapsed = convertUint8ValuesToUint32(&ptr[4]);

        /* copy play status in the 8th byte*/
        play_status = ptr[8];
    } 

    avrcpSendGetPlayStatusCfm(avrcp, status,
                              song_length, song_elapsed, play_status, 
                              transaction);
}


/*****************************************************************************/
void avrcpHandleInternalGetPlayStatusResponse(AVRCP *avrcp,
                     AVRCP_INTERNAL_GET_PLAY_STATUS_RES_T *res)
{
    uint16 size_mandatory_data = 1;
    uint8 mandatory_data[9];
    uint16 param_length = 1;

    mandatory_data[0] = avrcpGetErrorStatusCode(&res->response, 
                                                AVRCP0_CTYPE_STATUS);

    if (res->response == avctp_response_stable)
    {
        size_mandatory_data = 9;
        param_length = size_mandatory_data;
        /* Insert the mandatory data */
        convertUint32ToUint8Values(&mandatory_data[0], res->song_length);
        convertUint32ToUint8Values(&mandatory_data[4], res->song_elapsed);
        mandatory_data[8] = res->play_status;
    }

    avrcpSendMetadataResponse(avrcp,  res->response, 
                              AVRCP_GET_PLAY_STATUS_PDU_ID, 0, 
                              avrcp_packet_type_single, param_length, 
                              size_mandatory_data, mandatory_data);

}

