/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles messages received from the AGHFP library
*/


#include "audioAdaptor_private.h"
#include "audioAdaptor_aghfp_slc.h"
#include "audioAdaptor_aghfp_msg_handler.h"
#include "audioAdaptor_init.h"
#include "audioAdaptor_profile_slc.h"
#include "audioAdaptor_streammanager.h"
#include "audioAdaptor_configure.h"
#include "audioAdaptor_usb_hid.h"
#include "audioAdaptor_aghfp_call.h"
#include "audioAdaptor_statemanager.h"
#include "audioAdaptor_a2dp_stream_control.h"
#include "audioAdaptor_dev_instance.h"

#include <audio.h>
#include <pcm.h>
#include <file.h>
#include <kalimba.h>
#include <kalimba_standard_messages.h>
#include <string.h>
#include <panic.h>
#include <stdlib.h>


/****************************************************************************
  LOCAL FUNCTIONS
*/
      
/****************************************************************************
NAME
    aghfpUnhandledMessage

DESCRIPTION
    For debug purposes, so any unhandled AGHFP messages are discovered.     

*/
static void aghfpUnhandledMessage(devInstanceTaskData *theInst, uint16 id)
{
    DEBUG_AGHFP(("UNHANDLED AGHFP MESSAGE: inst:[0x%x] id:[0x%x] aghfp_state:[%d]\n",(uint16)theInst, id, theInst->aghfp_state));
}

    
/****************************************************************************
NAME
    connectAudio

DESCRIPTION
    Connects the specified SCO/eSCO channel to the DSP.

*/
static void connectAudio(Sink audio_sink)
{
    DEBUG_AGHFP(("    connectAudio(0x%X)\n", (uint16)audio_sink));
        
    if (audio_sink)
    {
        audio_codec_type codec = audio_codec_none;
    
        usbHidIssueHidCommand(AppEventHfpAudioConnected);
        
        if(!the_app->bidirect_faststream)
        {
              /* Decide the plugin */
            the_app->aghfp_audio_plugin = initScoPlugin();

            /* Connect the audio plugin */
            AudioConnect(the_app->aghfp_audio_plugin, 
                         audio_sink, 
                         the_app->sink_type,
                         the_app->codecTask,
                         0x0a, 
                         0,  
                         TRUE, 
                         AUDIO_MODE_CONNECTED, 
                         (void*)codec );
        
            /* Switch the DSP to SCO mode */
            PanicFalse(KalimbaSendMessage(KALIMBA_ENCODER_SELECT, EncoderSco, 0, 0, 0));
            the_app->active_encoder = EncoderSco;
        }
    }
}


/****************************************************************************
NAME
    disconnectAudio

DESCRIPTION
    Disconnects the specified SCO/eSCO channel from the DSP.

*/
static void disconnectAudio (devInstanceTaskData *inst)
{
    DEBUG_AGHFP(("    disconnectAudio(0x%X)\n", (uint16)inst->audio_sink));
    if (inst->audio_sink)
    {
        if (the_app->active_encoder == EncoderSco)
        {
            AudioDisconnect();    
            /* Switch the DSP to IDLE mode */ 
            PanicFalse(KalimbaSendMessage(KALIMBA_ENCODER_SELECT, EncoderNone, 0, 0, 0));
            the_app->active_encoder = EncoderNone;
        }
        
        inst->audio_sink = 0;
        
        usbHidIssueHidCommand(AppEventHfpAudioDisconnected);
    }
}


/****************************************************************************
NAME
    acceptAghfpConnectInd

DESCRIPTION
    Decide if the incoming AGHFP connection can be connected.
    Returns the device instance if successful, otherwise returns NULL.

*/
static devInstanceTaskData *acceptAghfpConnectInd(bdaddr *bd_addr)
{
    devInstanceTaskData *inst;
    uint16 i;
    
    /* Make sure no AGHFP connections active, as only 1 connection allowed */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {            
            switch (getAghfpState(inst))
            {
                case AghfpStateDisconnected:               
                case AghfpStateDisconnecting:
                {
                    /* These states are okay, an incoming connection is allowed */
                    break;
                }
                default:
                {
                    /* No incoming connection is allowed in other states */                    
                    return NULL;
                }
            }
        }
    }

    /* Check there isn't already an instance for this device */
    inst = devInstanceFindFromBddr(bd_addr, TRUE);    
   
    return inst;
}


/****************************************************************************
NAME
    acceptAghfpAudioConnectInd

DESCRIPTION
    Returns if the incoming AGHFP audio connection can be connected.

*/
static bool acceptAghfpAudioConnectInd(void)
{
    bool accept = FALSE;
    
    switch (the_app->app_state)
    {
        case AppStateIdle:
        {
            /* Can always accept an audio connection request in this state */
            accept = TRUE;
            break;        
        }
        case AppStateStreaming:
        {
            /* Must already have an audio connection open, reject the request */
            accept = FALSE;
            break;        
        }
        case AppStateInCall:                
        {    /* Being in InCall state not dependant on audio channel, accept if HFP/HSP audio */
            accept = TRUE;
            break;
        }        
        default:
        {
            break;
        }
    }
    
    return accept;
}


/****************************************************************************
NAME
    handleAghfpInitCfm

DESCRIPTION
    Handles the AGHFP library initialisation result.

*/
static void handleAghfpInitCfm(const AGHFP_INIT_CFM_T *msg)
{                
    if (the_app->supported_profiles & ProfileAghsp)
    {
        /* HSP was initialised */
        if (msg->status == aghfp_success)
        {
            DEBUG_AGHFP(("   AGHSP success\n"));
            the_app->aghsp = msg->aghfp;
            initProfileCfm(ProfileAghsp, TRUE);
        }
        else
        {
            DEBUG_AGHFP(("   AGHSP fail\n"));
            initProfileCfm(ProfileAghsp, FALSE);
        }
        return;
    }            
    if (the_app->supported_profiles & ProfileAghfp)
    {
        /* HFP was initialised */
        if (msg->status == aghfp_success)
        {
            DEBUG_AGHFP(("   AGHFP success\n"));
            the_app->aghfp = msg->aghfp;
            initProfileCfm(ProfileAghfp, TRUE);
        }
        else
        {    
            DEBUG_AGHFP(("   AGHFP fail\n"));
            initProfileCfm(ProfileAghfp, FALSE);
        }
        return;
    }                                                                               
}


/****************************************************************************
NAME
    handleAghfpSlcConnectInd

DESCRIPTION
    Handles the AGHFP library incoming connection message.

*/
static void handleAghfpSlcConnectInd(AGHFP_SLC_CONNECT_IND_T *ind)
{
    devInstanceTaskData *dev_inst;
    mvdProfiles profile = ProfileNone;
            
    if (ind->aghfp == the_app->aghfp)
    {
        profile = ProfileAghfp;
    }
    else if (ind->aghfp == the_app->aghsp)
    {
        profile = ProfileAghsp;
    }
            
    if (profile != ProfileNone)
    {
        /* test to see if the connection can be accepted */
        dev_inst = acceptAghfpConnectInd(&ind->bd_addr);
        if (dev_inst != NULL)
        {
            /* update state */
            setAghfpState(dev_inst, AghfpStatePaged);
            /* Store aghfp pointer to this instance */
            dev_inst->aghfp = ind->aghfp;
            /* update the remote profiles supported */
            dev_inst->remote_profiles |= profile;
            
            profileSlcAcceptConnectInd(dev_inst);
            
            /* Accept the connection */
            AghfpSlcConnectResponse(ind->aghfp, TRUE, &ind->bd_addr);
            DEBUG_AGHFP(("   Accepted incoming\n"));
            return;
        }
    }
    
    /* Reject the connection */
    AghfpSlcConnectResponse(ind->aghfp, FALSE, &ind->bd_addr);
    DEBUG_AGHFP(("   Rejected incoming\n"));
}


/****************************************************************************
NAME
    handleAghfpSlcConnectCfm

DESCRIPTION
    Handles the AGHFP library connection confirmation message.

*/
static void handleAghfpSlcConnectCfm(const AGHFP_SLC_CONNECT_CFM_T *cfm)
{
    devInstanceTaskData *inst = devInstanceFindFromAG(cfm->aghfp);
    mvdAvrcpState current_state = getAghfpState(inst);
    
    if (inst == NULL)
        return;

    switch (current_state)
    {
        case AghfpStatePaging:
        case AghfpStatePaged:
        {
            if ( cfm->status == aghfp_success )
            {    /* SLC connect attempt succeeded */    
                the_app->s_connect_attempts = 0;
                /* Store aghfp pointer to this instance */
                inst->aghfp = cfm->aghfp;
                /* Store sink */
                inst->aghfp_sink = cfm->rfcomm_sink;
                /* Update state */
                setAghfpState(inst, AghfpStateConnected);
				/* Get the current role */
				ConnectionGetRole(&the_app->task, inst->aghfp_sink);
                /* PIN is authorised */
                configureSetPinAuthorised(&inst->bd_addr, TRUE);
                /* Profile connection complete function */
                profileSlcConnectCfm(inst, ProfileAghfp, current_state == AghfpStatePaged ? TRUE : FALSE);                                
            }
            else
            {    
                /* SLC connect attempt failed */          
                the_app->s_connect_attempts += 1;
                             
                if ( (current_state == AghfpStatePaging) && 
                         ( (configureIsPinRequested(inst) && !configureIsPinListExhausted(inst)) || 
                           (the_app->s_connect_attempts < MAX_PROFILE_CONNECT_ATTEMPTS) ) )
                {    /* Try connection attempt again */
                    configureSetPinAuthorised(&inst->bd_addr, FALSE);
                    AghfpSlcConnect(inst->aghfp, &inst->bd_addr);
                }
                else if ( cfm->status == aghfp_connect_key_missing )
                {
                    /* Delete link key from our side and try again */
                    ConnectionSmDeleteAuthDevice( &inst->bd_addr );
                    AghfpSlcConnect(inst->aghfp, &inst->bd_addr);
                }
                else
                {
                    inst->aghfp = 0;
                    setAghfpState(inst, AghfpStateDisconnected);
                    profileSlcConnectCfm(inst, ProfileAghfp, FALSE);
                    MessageSend(&inst->task, APP_INTERNAL_DESTROY_REQ, 0);
                }
            }
            break;
        }      
        case AghfpStateConnected:
        {
            DEBUG_AGHFP(("  - ignored\n"));
            break;
        }
        default:
        {
            aghfpUnhandledMessage(inst, AGHFP_SLC_CONNECT_CFM);
        }
    }
}


/****************************************************************************
NAME
    handleAghfpSlcDisconnectInd

DESCRIPTION
    Handles the AGHFP library disconnection message.

*/
static void handleAghfpSlcDisconnectInd(const AGHFP_SLC_DISCONNECT_IND_T *ind)
{
    devInstanceTaskData *inst = devInstanceFindFromAG(ind->aghfp);
    
    if (inst == NULL)
        return;

    switch (getAghfpState(inst))
    {
        case AghfpStatePaging:
        case AghfpStatePaged:    
        {
            inst->aghfp_sink = 0;
            inst->audio_sink = 0;
            /* Reset aghfp pointer for this instance */
            inst->aghfp = 0;
                    
            /* Connection process aborted - indicate this by sending a connect fail confirmation */
            setAghfpState(inst, AghfpStateDisconnected);
            profileSlcConnectCfm(inst, ProfileAghfp, FALSE);
            break;
        }            
        case AghfpStateDisconnecting:
        {
            inst->audio_sink = 0;
            /* Reset aghfp pointer for this instance */
            inst->aghfp = 0;
                    
            setAghfpState(inst, AghfpStateDisconnected);
            profileSlcDisconnectInd(inst, ProfileAghfp);
            break;
        }    
        case AghfpStateAudioStreaming:
        case AghfpStateAudioClosing:
        case AghfpStateCallActive:
        case AghfpStateCallShutdown:
        {
            disconnectAudio(inst);            
            /* Fall through to AghfpStateConnected case */
        }
        case AghfpStateConnected:
        case AghfpStateAudioOpening:
        case AghfpStateCallSetup:
        {            
            inst->audio_sink = 0;
            /* Reset aghfp pointer for this instance */
            inst->aghfp = 0;
                
            setAghfpState(inst, AghfpStateDisconnected);
            profileSlcDisconnectInd(inst, ProfileAghfp);
            break;
        }
        default:
        {
            aghfpUnhandledMessage(inst, AGHFP_SLC_DISCONNECT_IND);
            break;
        }        
    }

    inst->responding_profiles &= ~ProfileAghfp;
    MessageSend(&inst->task, APP_INTERNAL_DESTROY_REQ, 0);
}


/****************************************************************************
NAME
    handleAghfpCallMgrCreateInd

DESCRIPTION
    Handles the AGHFP library call create indication message.

*/
static void handleAghfpCallMgrCreateInd(AGHFP *aghfp, Sink audio_sink, uint16 id)
{
    devInstanceTaskData *inst = devInstanceFindFromAG(aghfp);
    
    if (inst == NULL)
        return;
        
    switch (getAghfpState(inst))
    {
        case AghfpStateCallSetup:
        {
            if (inst->audio_sink == 0)
            {
                inst->audio_sink = audio_sink;
                connectAudio(inst->audio_sink);
            }
            break;
        }    
        default:
        {
            aghfpUnhandledMessage(inst, id);
            break;
        }
    }    
}


/****************************************************************************
NAME
    handleAghfpCallMgrCreateCfm

DESCRIPTION
    Handles the AGHFP library call create confirmation message.

*/
static void handleAghfpCallMgrCreateCfm(const AGHFP_CALL_MGR_CREATE_CFM_T *cfm)
{
    devInstanceTaskData *inst = devInstanceFindFromAG(cfm->aghfp);
    
    if (inst == NULL)
        return;
        
    switch (getAghfpState(inst))
    {
        case AghfpStateCallSetup:
        case AghfpStateCallShutdown:
        {
            if (cfm->status == aghfp_success)
            {    /* Call create attempt succeeded */
                setAghfpState(inst, AghfpStateCallActive);

                if (inst->audio_sink == 0)
                {
                    inst->audio_sink = cfm->audio_sink;
                    connectAudio(inst->audio_sink);
                }
                
                /* Continue with current comm action */
                aghfpCallOpenComplete(TRUE);
            }
            else
            {    /* Call create attempt failed */
                setAghfpState(inst, AghfpStateConnected);
                disconnectAudio(inst);
                aghfpCallOpenComplete(FALSE);
            }
            break;
        }    
        default:
        {
            aghfpUnhandledMessage(inst, AGHFP_CALL_MGR_CREATE_CFM);
            break;
        }
    }
}        


/****************************************************************************
NAME
    handleAghfpCallMgrTerminateInd

DESCRIPTION
    Handles the AGHFP library call terminate indication message.

*/
static void handleAghfpCallMgrTerminateInd(const AGHFP_CALL_MGR_TERMINATE_IND_T *ind)
{
    devInstanceTaskData *inst = devInstanceFindFromAG(ind->aghfp);
    
    if (inst == NULL)
        return;
        
    switch (getAghfpState(inst))
    {
        case AghfpStateCallActive:
        case AghfpStateCallShutdown:
        {
            if (ind->status == aghfp_success)
            {
                setAghfpState(inst, AghfpStateConnected);
                   disconnectAudio(inst);
            }        
            
            aghfpCallCloseComplete();
            break;
        }    
        default:
        {
            aghfpUnhandledMessage(inst, AGHFP_CALL_MGR_TERMINATE_IND);
            break;
        }
    }        
}


/****************************************************************************
NAME
    handleAghfpAudioConnectInd

DESCRIPTION
    Handles the AGHFP library audio connection indication message.

*/
static void handleAghfpAudioConnectInd(const AGHFP_AUDIO_CONNECT_IND_T *ind)
{
    devInstanceTaskData *inst = devInstanceFindFromAG(ind->aghfp);
    
    if (inst == NULL)
        return;
        
    switch (getAghfpState(inst))
    {
        case AghfpStateConnected:
        {
            if (!inst->audio_sink && acceptAghfpAudioConnectInd())
            {
                setAghfpState(inst, AghfpStateAudioOpening);
            }
            /* Fall through to AghfpStateCallActive state */
        }    
        case AghfpStateCallActive:
        {
            if (!inst->audio_sink && acceptAghfpAudioConnectInd())
            {
                AghfpAudioConnectResponse(inst->aghfp, TRUE, AUDIO_PACKET_TYPES, NULL);            
            }
            else
            {
                AghfpAudioConnectResponse(inst->aghfp, FALSE, AUDIO_PACKET_TYPES, NULL);            
            }
            break;
        }    
        default:
        {
            aghfpUnhandledMessage(inst, AGHFP_AUDIO_CONNECT_IND);
            break;
        }
    }
}


/****************************************************************************
NAME
    handleAghfpAudioConnectCfm

DESCRIPTION
    Handles the AGHFP library call create confirmation message.

*/
static void handleAghfpAudioConnectCfm(const AGHFP_AUDIO_CONNECT_CFM_T *ind)
{
    devInstanceTaskData *inst = devInstanceFindFromAG(ind->aghfp);
    
    if (inst == NULL)
        return;
 
    switch (getAghfpState(inst))
    {
        case AghfpStateAudioOpening:
        {
            if (ind->status == aghfp_success)
            {
                setAghfpState(inst, AghfpStateAudioStreaming);
            }
            else
            {
                setAghfpState(inst, AghfpStateConnected);
            }
            /* Fall through to AghfpStateCallActive case */
        }
        case AghfpStateCallActive:
        {
            if (ind->status == aghfp_success)
            {
                switch (ind->link_type)
                {
                    case sync_link_unknown:  /* Assume SCO if we don't know */
                    case sync_link_sco:
                        the_app->sink_type = AUDIO_SINK_SCO;
                    break;
                    case sync_link_esco:
                        the_app->sink_type = AUDIO_SINK_ESCO;
                        break;
                    default:
                        the_app->sink_type = AUDIO_SINK_INVALID;
                        break;
                }
                
                if (inst->audio_sink == 0)
                {
                    inst->audio_sink = ind->audio_sink;
                                          
                    connectAudio(inst->audio_sink);
                }
                streamManagerOpenComplete(TRUE);
            }
            else
            {
                streamManagerOpenComplete(FALSE);
            }
            break;
        }
        default:
        {
            aghfpUnhandledMessage(inst, AGHFP_AUDIO_CONNECT_CFM);
            break;
        }
    }
}


/****************************************************************************
NAME
    handleAghfpAudioDisconnectInd

DESCRIPTION
    Handles the AGHFP library audio disconnect message.

*/
static void handleAghfpAudioDisconnectInd(const AGHFP_AUDIO_DISCONNECT_IND_T *ind)
{
    devInstanceTaskData *inst = devInstanceFindFromAG(ind->aghfp);
    
    if (inst == NULL)
        return;
        
    switch (getAghfpState(inst))
    {
        case AghfpStateAudioStreaming:
        case AghfpStateAudioClosing:
        {
            if (inst->audio_sink)
            {
                setAghfpState(inst, AghfpStateConnected);
                disconnectAudio(inst);            
                streamManagerCloseComplete();
            }
            break;
        }    
        case AghfpStateCallActive:
        case AghfpStateCallShutdown:
        {
            if (inst->audio_sink)
            {
                disconnectAudio(inst);            
                streamManagerCloseComplete();
            }
            break;
        }    
        default:
        {
            aghfpUnhandledMessage(inst, AGHFP_AUDIO_DISCONNECT_IND);
            break;
        }
        break;
    }
}



/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    aghfpMsgHandleLibMessage

DESCRIPTION
    Handles AGHFP library messages and calls the relevant handler function.
    
*/
void aghfpMsgHandleLibMessage(MessageId id, Message message)
{
    switch(id)
    {
        case AGHFP_INIT_CFM:
        {
            DEBUG_AGHFP(("AGHFP_INIT_CFM status = %u [0x%X]\n", ((AGHFP_INIT_CFM_T *)message)->status,(uint16)((AGHFP_INIT_CFM_T *)message)->aghfp));
            handleAghfpInitCfm((AGHFP_INIT_CFM_T *)message);
            break;
        }
        case AGHFP_SLC_CONNECT_IND:
        {
            AGHFP_SLC_CONNECT_IND_T *ind = (AGHFP_SLC_CONNECT_IND_T *)message;
            DEBUG_AGHFP(("AGHFP_SLC_CONNECT_IND aghfp=0x%X from: 0x%X 0x%X 0x%lX\n", (uint16)ind->aghfp, ind->bd_addr.nap, ind->bd_addr.uap, ind->bd_addr.lap));
            handleAghfpSlcConnectInd(ind);
            break;
        }                
        case AGHFP_SLC_CONNECT_CFM:
        {
            AGHFP_SLC_CONNECT_CFM_T *cfm = (AGHFP_SLC_CONNECT_CFM_T *)message;
            DEBUG_AGHFP(("AGHFP_SLC_CONNECT_CFM aghfp = 0x%X status = %u\n", (uint16)cfm->aghfp, cfm->status));
            handleAghfpSlcConnectCfm(cfm);
            break;            
        }
        case AGHFP_SLC_DISCONNECT_IND:
        {
            AGHFP_SLC_DISCONNECT_IND_T *ind = (AGHFP_SLC_DISCONNECT_IND_T *)message;
            DEBUG_AGHFP(("AGHFP_SLC_DISCONNECT_IND aghfp = 0x%X status = %d\n", (uint16)ind->aghfp, (uint16)ind->status));
            handleAghfpSlcDisconnectInd(ind);
            break;
        }        
        case AGHFP_CALL_MGR_CREATE_WAITING_RESPONSE_IND:
        {
            DEBUG_AGHFP(("AGHFP_CALL_MGR_CREATE_WAITING_RESPONSE_IND\n"));
            handleAghfpCallMgrCreateInd(((AGHFP_CALL_MGR_CREATE_WAITING_RESPONSE_IND_T *)message)->aghfp,
                        ((AGHFP_CALL_MGR_CREATE_WAITING_RESPONSE_IND_T *)message)->audio_sink, 
                        AGHFP_CALL_MGR_CREATE_WAITING_RESPONSE_IND);
            break;
        }
        case AGHFP_CALL_MGR_CREATE_ALERTING_REMOTE_IND:
        {
            DEBUG_AGHFP(("AGHFP_CALL_MGR_CREATE_ALERTING_REMOTE_IND\n"));
            handleAghfpCallMgrCreateInd(((AGHFP_CALL_MGR_CREATE_ALERTING_REMOTE_IND_T *)message)->aghfp,
                        ((AGHFP_CALL_MGR_CREATE_ALERTING_REMOTE_IND_T *)message)->audio_sink, 
                        AGHFP_CALL_MGR_CREATE_ALERTING_REMOTE_IND);
            break;
        }        
        case AGHFP_CALL_MGR_CREATE_CFM:
        {
            DEBUG_AGHFP(("AGHFP_CALL_MGR_CREATE_CFM status = %u\n", ((AGHFP_CALL_MGR_CREATE_CFM_T *)message)->status));
            handleAghfpCallMgrCreateCfm((AGHFP_CALL_MGR_CREATE_CFM_T *)message);
            break;
        }
        case AGHFP_CALL_MGR_TERMINATE_IND:
        {
            DEBUG_AGHFP(("AGHFP_CALL_MGR_TERMINATE_IND\n"));
            handleAghfpCallMgrTerminateInd((AGHFP_CALL_MGR_TERMINATE_IND_T *)message);
            break;
        }
        case AGHFP_AUDIO_CONNECT_IND:
        {
            DEBUG_AGHFP(("AGHFP_AUDIO_CONNECT_IND\n"));
            handleAghfpAudioConnectInd((AGHFP_AUDIO_CONNECT_IND_T *)message);
            break;
        }
        case AGHFP_AUDIO_CONNECT_CFM:
        {
            DEBUG_AGHFP(("AGHFP_AUDIO_CONNECT_CFM status = %u\n", ((AGHFP_AUDIO_CONNECT_CFM_T *)message)->status));
            handleAghfpAudioConnectCfm((AGHFP_AUDIO_CONNECT_CFM_T *)message);
            break;
        }        
        case AGHFP_AUDIO_DISCONNECT_IND:
        {
            DEBUG_AGHFP(("AGHFP_AUDIO_DISCONNECT_IND\n"));
            handleAghfpAudioDisconnectInd((AGHFP_AUDIO_DISCONNECT_IND_T *)message);
            break;
        }
        case AGHFP_HS_CALL_ANSWER_IND:
        {
            DEBUG_AGHFP(("AGHFP_HS_CALL_ANSWER_IND\n"));
            usbHidIssueHidCommand(AppEventHfpAta);
            break;
        }    
        case AGHFP_HS_CALL_HANG_UP_IND:
        {
            DEBUG_AGHFP(("AGHFP_HS_CALL_HANG_UP_IND\n"));
            usbHidIssueHidCommand(AppEventHfpChup);
            break;
        }        
        case AGHFP_HS_AUDIO_REQUEST_IND:
        {
            DEBUG_AGHFP(("AGHFP_HS_AUDIO_REQUEST_IND\n"));
            if ( the_app->voip_call_active )
            {
                usbHidIssueHidCommand(AppEventHfpChup);
            }
            else
            {
                usbHidIssueHidCommand(AppEventHfpAta);
            }
            break;
        }        
        case AGHFP_ANSWER_IND:
        {
            DEBUG_AGHFP(("AGHFP_ANSWER_IND\n"));
            usbHidIssueHidCommand(AppEventHfpAta);
            break;
        }   
        case AGHFP_CALL_HANG_UP_IND:
        {
            DEBUG_AGHFP(("AGHFP_CALL_HANG_UP_IND\n"));
            usbHidIssueHidCommand(AppEventHfpChup);
            break;
        }    
        case AGHFP_DIAL_IND:
        {
            DEBUG_AGHFP(("AGHFP_DIAL_IND\n"));
            usbHidIssueHidCommand(AppEventHfpAtd);
            break;
        }    
        case AGHFP_MEMORY_DIAL_IND:
        {
            DEBUG_AGHFP(("AGHFP_MEMORY_DIAL_IND\n"));
            usbHidIssueHidCommand(AppEventHfpAtd);
            break;
        }    
        case AGHFP_LAST_NUMBER_REDIAL_IND:
        {
            DEBUG_AGHFP(("AGHFP_LAST_NUMBER_REDIAL_IND\n"));
            usbHidIssueHidCommand(AppEventHfpBldn);
            break;
        }
        case AGHFP_CALLER_ID_SETUP_IND:
        {
            DEBUG_AGHFP(("AGHFP_CALLER_ID_SETUP_IND\n"));
            /* Ignore this message for now, OK sent by library */
            DEBUG_AGHFP(("  - ignored\n"));
            break;
        }    
        case AGHFP_CALL_WAITING_SETUP_IND:
        {
            DEBUG_AGHFP(("AGHFP_CALL_WAITING_SETUP_IND\n"));
            /* Ignore this message for now, OK sent by library */
            DEBUG_AGHFP(("  - ignored\n"));
            break;
        }    
        case AGHFP_CALL_HOLD_IND:
        {
            DEBUG_AGHFP(("AGHFP_CALL_HOLD_IND\n"));
            /* Ignore this message for now, OK sent by library */
            DEBUG_AGHFP(("  - ignored\n"));
            break;
        }    
        case AGHFP_NREC_SETUP_IND:
        {
            DEBUG_AGHFP(("AGHFP_NREC_SETUP_IND\n"));
            /* noise reduction not supported so send error */
            DEBUG_AGHFP(("  - unsupported, send ERROR\n"));
            AghfpSendError(((AGHFP_NREC_SETUP_IND_T *)message)->aghfp);
            break;
        }    
        case AGHFP_VOICE_RECOGNITION_SETUP_IND:
        {
            DEBUG_AGHFP(("AGHFP_VOICE_RECOGNITION_SETUP_IND\n"));
            /* Ensure voice recongition activation is disabled before setting up call */
            /* on receipt of AGHFP_VOICE_RECOGNITION_ENABLE_CFM.                      */
            AghfpVoiceRecognitionEnable(((AGHFP_VOICE_RECOGNITION_SETUP_IND_T *)message)->aghfp, FALSE);
            break;
        }    
        case AGHFP_PHONE_NUMBER_REQUEST_IND:
        {
            DEBUG_AGHFP(("AGHFP_PHONE_NUMBER_REQUEST_IND\n"));
            /* Ignore this requiest, send ERROR */
            DEBUG_AGHFP(("  - ignored, send ERROR\n"));
            AghfpSendError(((AGHFP_PHONE_NUMBER_REQUEST_IND_T *)message)->aghfp);
            break;
        }    
        case AGHFP_TRANSMIT_DTMF_CODE_IND:
        {
            DEBUG_AGHFP(("AGHFP_TRANSMIT_DTMF_CODE_IND\n"));
            /* Ignore this message for now, send OK even though we can't generate a DTMF tone */
            DEBUG_AGHFP(("  - ignored, send OK\n"));
            AghfpSendOk(((AGHFP_TRANSMIT_DTMF_CODE_IND_T *)message)->aghfp);
            break;
        }    
        case AGHFP_SYNC_MICROPHONE_GAIN_IND:
        {
            bool mute_state = (((AGHFP_SYNC_MICROPHONE_GAIN_IND_T*)message)->gain == 0);
            DEBUG_AGHFP(("AGHFP_SYNC_MICROPHONE_GAIN_IND\n"));
            usbHidSignalMuteState(mute_state);
            break;
        }    
        case AGHFP_SYNC_SPEAKER_VOLUME_IND:
        {
            uint8 vol = ((AGHFP_SYNC_SPEAKER_VOLUME_IND_T*)message)->volume;
            DEBUG_AGHFP(("AGHFP_SYNC_SPEAKER_VOLUME_IND\n"));
            if (vol > the_app->speaker_volume)  usbHidSignalVolumeUp();
            else if (vol < the_app->speaker_volume)    usbHidSignalVolumeDown();
            the_app->speaker_volume = vol;
            break;
        }
        case AGHFP_RESPONSE_HOLD_STATUS_REQUEST_IND:
        {
            DEBUG_AGHFP(("AGHFP_RESPONSE_HOLD_STATUS_IND\n"));
            /* Just send OK back since we do not support Response and Hold */
            AghfpSendOk(((AGHFP_RESPONSE_HOLD_STATUS_REQUEST_IND_T *)message)->aghfp);
            break;
        }    
        case AGHFP_SET_RESPONSE_HOLD_STATUS_IND:
        {
            DEBUG_AGHFP(("AGHFP_RESPONSE_HOLD_IND\n"));
            DEBUG_AGHFP(("  - ignored, send OK\n"));
            /* Not in a hold state so don't need AghfpConfirmResponseHoldState so just send OK*/
            AghfpSendOk(((AGHFP_SET_RESPONSE_HOLD_STATUS_IND_T *)message)->aghfp);
            break;
        }    
        case AGHFP_SUBSCRIBER_NUMBER_IND:
        {
            DEBUG_AGHFP(("AGHFP_SUBSCRIBER_NUMBER_IND\n"));
            /* Ignore this message for now */
            DEBUG_AGHFP(("  - ignored\n"));
            AghfpSendSubscriberNumbersComplete(((AGHFP_SUBSCRIBER_NUMBER_IND_T *)message)->aghfp);
            break;
        }    
        case AGHFP_SUBSCRIBER_NUMBER_CFM:
        {
            DEBUG_AGHFP(("AGHFP_SUBSCRIBER_NUMBER_CFM\n"));
            /* Ignore this message for now */
            DEBUG_AGHFP(("  - ignored\n"));
            break;
        }
        case AGHFP_CURRENT_CALLS_IND:
        {
            DEBUG_AGHFP(("AGHFP_CURRENT_CALLS_IND\n"));
            /* Current calls list not supported so just send ok */
            DEBUG_AGHFP(("  - no current calls so just OK\n"));
            AghfpSendOk(((AGHFP_CURRENT_CALLS_IND_T *)message)->aghfp);
            break;
        }    
        case AGHFP_NETWORK_OPERATOR_IND:
        {
            char nwOP[4] = "VoIP";
            DEBUG_AGHFP(("AGHFP_NETWORK_OPERATOR_IND\n"));
            /* we don't have a network operator so just send a generic string */
            DEBUG_AGHFP(("  - respond with operator name VoIP\n"));
            AghfpSendNetworkOperator(((AGHFP_NETWORK_OPERATOR_IND_T *)message)->aghfp, 0, 4, (uint8*)nwOP);
            break;
        }
        /* AGHFP function call confirms - for debug output */
        case AGHFP_NETWORK_OPERATOR_CFM:
        {
            DEBUG_AGHFP(("AGHFP_NETWORK_OPERATOR_CFM\n"));
            break;
        }        
        case AGHFP_SEND_SERVICE_INDICATOR_CFM:
        {
            DEBUG_AGHFP(("AGHFP_SEND_SERVICE_INDICATOR_CFM\n"));
            break;
        }    
        case AGHFP_SEND_CALL_INDICATOR_CFM:
        {
            DEBUG_AGHFP(("AGHFP_SEND_CALL_INDICATOR_CFM\n"));
            break;
        }    
        case AGHFP_SEND_CALL_SETUP_INDICATOR_CFM:
        {
            DEBUG_AGHFP(("AGHFP_CALL_SETUP_INDICATOR_CFM\n"));
            break;
        }    
        case AGHFP_SEND_RING_ALERT_CFM:
        {
            DEBUG_AGHFP(("AGHFP_SEND_RING_ALERT_CFM\n"));
            break;
        }    
        case AGHFP_SEND_CALLER_ID_CFM:
        {
            DEBUG_AGHFP(("AGHFP_SEND_CALLER_ID_CFM\n"));
            break;
        }    
        case AGHFP_INBAND_RING_ENABLE_CFM:
        {
            DEBUG_AGHFP(("AGHFP_INBAND_RING_ENABLE\n"));
            break;
        }    
        case AGHFP_SEND_CALL_WAITING_NOTIFICATION_CFM:
        {
            DEBUG_AGHFP(("AGHFP_SEND_CALL_WAITING_NOTIFICATION_CFM\n"));
            break;
        }    
        case AGHFP_VOICE_RECOGNITION_ENABLE_CFM:
        {
            DEBUG_AGHFP(("AGHFP_VOICE_RECOGNITION_ENABLE_CFM\n"));
            usbHidIssueHidCommand(AppEventHfpBvra);
            break;
        }    
        case AGHFP_SEND_PHONE_NUMBER_FOR_VOICE_TAG_CFM:
        {
            DEBUG_AGHFP(("AGHFP_SEND_PHONE_NUMBER_FOR_VOICE_TAG_CFM\n"));
            break;
        }    
        case AGHFP_SET_REMOTE_MICROPHONE_GAIN_CFM:
        {
            DEBUG_AGHFP(("AGHFP_SET_REMOTE_MICROPHONE_GAIN_CFM\n"));
            break;
        }    
        case AGHFP_SET_REMOTE_SPEAKER_VOLUME_CFM:
        {
            DEBUG_AGHFP(("AGHFP_SET_REMOTE_SPEAKER_VOLUME_CFM\n"));
            break;
        }    
        case AGHFP_USER_DATA_CFM:
        {
            DEBUG_AGHFP(("AGHFP_USER_DATA_CFM status=%d\n", ((AGHFP_USER_DATA_CFM_T*)message)->status));
            break;
        }    
        case AGHFP_SEND_ERROR_CFM:
        {
            DEBUG_AGHFP(("AGHFP_SEND_ERROR_CFM status=%d\n", ((AGHFP_SEND_ERROR_CFM_T*)message)->status));
            break;
        }    
        case AGHFP_SET_SERVICE_STATE_CFM:
        {
            DEBUG_AGHFP(("AGHFP_SET_SERVICE_STATE_CFM\n"));
            break;
        }    
        case AGHFP_UNRECOGNISED_AT_CMD_IND:
        {
            DEBUG_AGHFP(("AGHFP_UNRECOGNISED_AT_CMD_IND\n"));
            AghfpSendError(((AGHFP_UNRECOGNISED_AT_CMD_IND_T *)message)->aghfp);
            break;
        }    
        case AGHFP_CSR_SUPPORTED_FEATURES_IND:
        {
            DEBUG_AGHFP(("AGHFP_CSR_SUPPORTED_FEATURES_IND\n"));
            break;
        } 
        default:
        {
            DEBUG_AGHFP(("Unhandled AGHFP message 0x%X\n", (uint16)id));            
            break;
        }
    }
}
