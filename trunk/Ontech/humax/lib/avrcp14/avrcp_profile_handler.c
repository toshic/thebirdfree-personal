/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    profile_handler.c        

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "avrcp_caps_handler.h"
#include "avrcp_connect_handler.h"
#include "avrcp_continuation_handler.h"
#include "avrcp_element_attributes_handler.h"
#include "avrcp_player_app_settings_handler.h"
#include "avrcp_l2cap_handler.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_notification_handler.h"
#include "avrcp_signal_passthrough.h"
#include "avrcp_signal_unit_info.h"
#include "avrcp_signal_vendor.h"
#include "avrcp_absolute_volume.h"
#include "avrcp_profile_handler.h"


static bool avrcpHandleInitializingMessages(AVRCP *avrcp, 
                                            MessageId id, 
                                            Message   message)
{
    switch(id)
    {
       case AVRCP_INTERNAL_INIT_REQ:
            avrcpHandleInternalInitReq(avrcp,
                                     (AVRCP_INTERNAL_INIT_REQ_T *) message);
            break;

       case CL_SDP_REGISTER_CFM:
            avrcpHandleSdpRegisterCfm(avrcp,
                                     (CL_SDP_REGISTER_CFM_T *) message);
            break;    

       case CL_L2CAP_REGISTER_CFM:
            avrcpHandleL2capRegisterCfm(avrcp, 
                                     (CL_L2CAP_REGISTER_CFM_T *) message);
            break;
   
        default:
            return FALSE;  
    }

    return TRUE;
}

static bool avrcpHandleReadyMessages(AVRCP *avrcp, 
                                     MessageId id, 
                                     Message   message)
{
    switch(id)
    {
        case AVRCP_INTERNAL_CONNECT_REQ:
            avrcpHandleInternalConnectReq(avrcp, 
                                   (AVRCP_INTERNAL_CONNECT_REQ_T *) message);
            break;

       case AVRCP_INTERNAL_DISCONNECT_REQ:
            avrcpSendCommonCfmMessageToApp( AVRCP_DISCONNECT_IND, 
                                    avrcp_device_not_connected, 0, avrcp);
            break;

        case CL_L2CAP_CONNECT_IND:
            avrcpHandleL2capConnectInd(avrcp,
                                       (CL_L2CAP_CONNECT_IND_T *) message);
            break;

        default:
            return FALSE;  
    }

    return TRUE;
}

static bool avrcpHandleConnectingMessages(AVRCP *avrcp, 
                                         MessageId id, 
                                         Message   message)
{
    switch(id)
    {
       case AVRCP_INTERNAL_CONNECT_RES:
             avrcpHandleInternalL2capConnectRes(avrcp, 
                             (AVRCP_INTERNAL_CONNECT_RES_T *) message);
             break;

       case  CL_L2CAP_CONNECT_CFM:
            avrcpHandleL2capConnectCfm(avrcp, 
                       (CL_L2CAP_CONNECT_CFM_T *) message);
            break;

      case CL_L2CAP_CONNECT_IND:
            avrcpHandleL2capConnectIndReject(avrcp,
                                       (CL_L2CAP_CONNECT_IND_T *) message);
            break;



        default:
            return FALSE;  
    }

    return TRUE;
}

static bool avrcpHandleConnectedMessages(AVRCP *avrcp, 
                                         MessageId id, 
                                         Message   message)
{
    switch(id)
    {
        case AVRCP_INTERNAL_DISCONNECT_REQ:
            avrcpHandleInternalDisconnectReq(avrcp);
            break;

        case AVRCP_INTERNAL_PASSTHROUGH_REQ:
            avrcpHandleInternalPassThroughReq(avrcp, 
                           (AVRCP_INTERNAL_PASSTHROUGH_REQ_T *) message);
            break;

        case AVRCP_INTERNAL_PASSTHROUGH_RES:
            avrcpHandleInternalPassThroughRes(avrcp, 
                           (AVRCP_INTERNAL_PASSTHROUGH_RES_T *) message);
            break;

        case AVRCP_INTERNAL_UNITINFO_REQ:
            avrcpHandleInternalUnitInfoReq(avrcp);
            break;

        case AVRCP_INTERNAL_UNITINFO_RES:
            avrcpHandleInternalUnitInfoRes( avrcp, 
                               (AVRCP_INTERNAL_UNITINFO_RES_T *) message);
            break;

        case AVRCP_INTERNAL_SUBUNITINFO_REQ:
            avrcpHandleInternalSubUnitInfoReq(avrcp, 
                                (AVRCP_INTERNAL_SUBUNITINFO_REQ_T *) message);
            break;

        case AVRCP_INTERNAL_SUBUNITINFO_RES:
            avrcpHandleInternalSubUnitInfoRes(avrcp, 
                               (AVRCP_INTERNAL_SUBUNITINFO_RES_T *) message);
            break;

        case AVRCP_INTERNAL_VENDORDEPENDENT_REQ:
            avrcpHandleInternalVendorDependentReq(avrcp, 
                               (AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *) message);
            break;

        case AVRCP_INTERNAL_VENDORDEPENDENT_RES:
            avrcpHandleInternalVendorDependentRes(avrcp, 
                               (AVRCP_INTERNAL_VENDORDEPENDENT_RES_T *) message);
            break;

       case AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT:
            avrcpHandleInternalSendResponseTimeout(avrcp,
                          (AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT_T *) message);
            break;

        case AVRCP_INTERNAL_GET_CAPS_RES:
            avrcpHandleInternalGetCapsResponse(avrcp, 
                               (AVRCP_INTERNAL_GET_CAPS_RES_T *) message);
            break;

        case AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES:
            avrcpHandleInternalListAppAttributesResponse(avrcp, 
                            (AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES_T *) message);
            break;

        case AVRCP_INTERNAL_LIST_APP_VALUE_RES:
            avrcpHandleInternalListAppValuesResponse(avrcp, 
                            (AVRCP_INTERNAL_LIST_APP_VALUE_RES_T *) message);
            break;

        case AVRCP_INTERNAL_GET_APP_VALUE_RES:
            avrcpHandleInternalGetAppValueResponse(avrcp, 
                            (AVRCP_INTERNAL_GET_APP_VALUE_RES_T *) message);
            break;

        case AVRCP_INTERNAL_SET_APP_VALUE_RES:
            avrcpHandleInternalSetAppValueResponse(avrcp, 
                             (AVRCP_INTERNAL_SET_APP_VALUE_RES_T *) message);
            break;

        case AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES:
            avrcpHandleInternalGetAppAttributeTextResponse(avrcp, 
                     (AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES_T *) message);
            break;

        case AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES:
            avrcpHandleInternalGetAppValueTextResponse(avrcp, 
                     (AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES_T *) message);
            break;

        case AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES:
            avrcpHandleInternalGetElementAttributesResponse(avrcp, 
                       (AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES_T *) message);
            break;

        case AVRCP_INTERNAL_GET_PLAY_STATUS_RES:
            avrcpHandleInternalGetPlayStatusResponse(avrcp, 
                      (AVRCP_INTERNAL_GET_PLAY_STATUS_RES_T *) message);
            break;

        case AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES:
            avrcpHandleInternalEventPlaybackStatusChangedResponse(avrcp, 
                 (AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES_T *) message);
            break;

        case AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES:
            avrcpHandleInternalEventTrackChangedResponse(avrcp, 
                       (AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES_T *) message);
            break;

        case AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES:
            avrcpHandleInternalEventTrackReachedEndResponse(avrcp, 
                      (AVRCP_INTERNAL_EVENT_TRACK_REACHED_END_RES_T *) message);
            break;

        case AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES:
            avrcpHandleInternalEventTrackReachedStartResponse(avrcp, 
                   (AVRCP_INTERNAL_EVENT_TRACK_REACHED_START_RES_T *) message);
            break;

        case AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES:
            avrcpHandleInternalEventPlaybackPosChangedResponse(avrcp, 
                     (AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES_T *) message);
            break;

        case AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES:
            avrcpHandleInternalEventBattStatusChangedResponse(avrcp, 
                     (AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES_T *) message);
            break;

        case AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES:
            avrcpHandleInternalEventSystemStatusChangedResponse(avrcp, 
                  (AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES_T *) message);
            break;

       case AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES:
           avrcpHandleInternalEventPlayerAppSettingChangedResponse(avrcp, 
              (AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES_T *) message);
            break;

       case AVRCP_INTERNAL_REJECT_METADATA_RES:
           avrcpHandleInternalRejectMetadataResponse(avrcp, 
                    ((AVRCP_INTERNAL_REJECT_METADATA_RES_T*)message)->response,
                    ((AVRCP_INTERNAL_REJECT_METADATA_RES_T*)message)->pdu_id);
           break;   

       case AVRCP_INTERNAL_ABORT_CONTINUING_RES:
           avrcpHandleInternalAbortContinuingResponse(avrcp, 
                        (AVRCP_INTERNAL_ABORT_CONTINUING_RES_T *) message);
           break;

       case AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET:
           avrcpHandleNextContinuationPacket(avrcp, 
                      (AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET_T *) message);
           break;

        case AVRCP_INTERNAL_GROUP_RES:
           avrcpHandleInternalGroupResponse(avrcp, 
                        (AVRCP_INTERNAL_GROUP_RES_T *) message);
           break;   

        case AVRCP_INTERNAL_INFORM_BATTERY_STATUS_RES:
            prepareMetadataControlResponse(avrcp, 
              ((AVRCP_INTERNAL_INFORM_BATTERY_STATUS_RES_T *) message)->response,
                      AVRCP_INFORM_BATTERY_STATUS_PDU_ID);
            break;

        case AVRCP_INTERNAL_INFORM_CHAR_SET_RES:
            prepareMetadataControlResponse(avrcp,
                 ((AVRCP_INTERNAL_INFORM_CHAR_SET_RES_T *) message)->response, 
                           AVRCP_INFORM_CHARACTER_SET_PDU_ID);            
            break;

        case AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES:
            avrcpHandleInternalAbsoluteVolumeRsp(avrcp,
                              (AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES_T*) message);
            break; 
   
        case AVRCP_INTERNAL_EVENT_VOLUME_CHANGED_RES:
            avrcpHandleInternalAbsoluteVolumeEvent(avrcp,
                     (AVRCP_INTERNAL_EVENT_VOLUME_CHANGED_RES_T*) message);
            break; 

        case AVRCP_INTERNAL_GET_FEATURES:
            avrcpGetSupportedFeatures(avrcp, TRUE);
            break;

        case AVRCP_INTERNAL_GET_EXTENSIONS:
            avrcpGetProfileVersion(avrcp, TRUE);
            break;

        case CL_L2CAP_DISCONNECT_IND:
            avrcpHandleL2capDisconnectInd(avrcp, 
                          (CL_L2CAP_DISCONNECT_IND_T *) message);
              break;

        default:
            return FALSE;  
    }

    return TRUE;
}


static void avrcpHandleUnhandledMessages(AVRCP *avrcp, 
                                         MessageId id, 
                                         Message   message)
{
    switch(id)
    {
        case AVRCP_INTERNAL_TASK_INIT_REQ:        
            {
                uint16 app = (*((uint16 *)message));
                avrcpInitTaskData(avrcp, (Task) app, 
                                  avrcpReady, avrcp_device_none, 0, 0, 0, 1);
            }
            break;

        case AVRCP_INTERNAL_TASK_DELETE_REQ:
            avrcpHandleDeleteTask(avrcp);
            break;

        case AVRCP_INTERNAL_CONNECT_REQ:
            avrcpSendCommonCfmMessageToApp( AVRCP_CONNECT_CFM, 
                                            avrcp_bad_state, 0, avrcp);

            break;

        case AVRCP_INTERNAL_DISCONNECT_REQ:
            avrcpSendCommonCfmMessageToApp( AVRCP_DISCONNECT_IND, 
                                    avrcp_device_not_connected, 0, avrcp);
            break;



        case AVRCP_INTERNAL_PASSTHROUGH_REQ:
            avrcpSendCommonCfmMessageToApp(AVRCP_PASSTHROUGH_CFM, 
                                    avrcp_device_not_connected, 0, avrcp);
            break;

        case AVRCP_INTERNAL_UNITINFO_REQ:
            avrcpSendUnitInfoCfmToClient(avrcp, 
                                 avrcp_device_not_connected, 0, 0, (uint32) 0);
            break;

        case AVRCP_INTERNAL_SUBUNITINFO_REQ:
           avrcpSendSubunitInfoCfmToClient(avrcp,
                                          avrcp_device_not_connected, 0, 0); 
           
            break;      

        case AVRCP_INTERNAL_VENDORDEPENDENT_REQ:
           avrcpSendCommonCfmMessageToApp(AVRCP_VENDORDEPENDENT_CFM, 
                                 avrcp_device_not_connected, 0, avrcp);
            break;


        case AVRCP_INTERNAL_WATCHDOG_TIMEOUT:
            avrcpHandleInternalWatchdogTimeout(avrcp);
            break;

        case AVRCP_INTERNAL_MESSAGE_MORE_DATA:
            avrcpHandleReceivedData(avrcp);  
            break;

       case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
             avrcpHandleServiceSearchAttributeCfm(avrcp, 
                        (CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *) message);
             break;

        case CL_L2CAP_CONNECT_IND:
            avrcpHandleL2capConnectIndReject(avrcp,
                                       (CL_L2CAP_CONNECT_IND_T *) message);
            break;

       case CL_L2CAP_CONNECT_CFM:
       case CL_L2CAP_DISCONNECT_IND:
            break;

       case MESSAGE_MORE_DATA: /* Fall Through */
            avrcpHandleReceivedData(avrcp);        
            break;

       case MESSAGE_MORE_SPACE:
       case MESSAGE_SOURCE_EMPTY:
                /* ignore */
                break;

        default:
            if(id > AVRCP_MSG_BASE  && id < AVRCP_MSG_BOTTOM)
            {
                /* Ignore these messages. It may happen due to cross over of messages */
                AVRCP_INFO(("Ignoring the Received Message(%d) due to wrong state (%d)",
                         id, avrcp->state));
            }
            else
            {
                AVRCP_DEBUG((" AVRCP handleUnexpected - State 0x%x MsgId 0x%x\n",
                      avrcp->state, id ));

            }
            break;
    }

    return;
    

}


/*****************************************************************************/
void avrcpProfileHandler(Task task, MessageId id, Message message)
{
    AVRCP *avrcp = (AVRCP *) task;
    avrcpState profileState = avrcp->state;
    bool consumed=TRUE;

    /* consume only initializing messages */
    if (profileState == avrcpInitialising)
    {
        consumed = avrcpHandleInitializingMessages(avrcp, id, message);
    }
    else if (profileState == avrcpReady)
    {
        consumed = avrcpHandleReadyMessages(avrcp, id, message);
    }
    else if (profileState == avrcpConnecting)
    {
        consumed = avrcpHandleConnectingMessages(avrcp, id, message);
    }
    else
    {
        consumed = avrcpHandleConnectedMessages(avrcp, id, message);
    }

    if(!consumed)
    {

        avrcpHandleUnhandledMessages(avrcp, id, message);
    }

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
            {
                free(cleanTask->sent_data);
                AVRCP_INFO(("avrcpDataCleanUp: Cleanup Stored Data"));
            }
            cleanTask->sent_data = 0;
        }
        break;

    default:
        AVRCP_DEBUG((" AVRCP handleUnexpected in "
                   " avrcpDataCleanUp- MsgId 0x%x\n",id));
        break;
    }
}

