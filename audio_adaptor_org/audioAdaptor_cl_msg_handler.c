/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles messages received from the Connection library
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_init.h"
#include "audioAdaptor_test.h"
#include "audioAdaptor_cl_msg_handler.h"
#include "audioAdaptor_scan.h"
#include "audioAdaptor_configure.h"
#include "audioAdaptor_profile_slc.h"
#include "audioAdaptor_dev_instance.h"
#include "audioAdaptor_a2dp.h"
#include "audioAdaptor_a2dp_slc.h"
#include "audioAdaptor_a2dp_msg_handler.h"
#include "audioAdaptor_statemanager.h"

#include <string.h>
#include <panic.h>
#include <stdlib.h>


/****************************************************************************
  LOCAL FUNCTIONS
*/

/****************************************************************************
NAME
    unexpectedClMessage

DESCRIPTION
    For debug purposes, so any unhandled CL messages are discovered.     

*/
static void unexpectedClMessage(mvdAppState state, MessageId id)
{
    DEBUG_CL(("Unexpected CL message 0x%X in state %u\n", (uint16)id, (uint16)state));
}


/****************************************************************************
NAME
    handleClInitCfm

DESCRIPTION
    Handles the CL library initialisation result.

*/
static void handleClInitCfm(const CL_INIT_CFM_T *cfm)
{
    mvdAppState app_state = the_app->app_state;
    
    switch (app_state)
    {
        case AppStateInitialising:
        {
            if (cfm->status == success)
            {
                /* Configure Mode4 Security Settings */
                ConnectionSmSecModeConfig(&the_app->task, cl_sm_wae_acl_owner_none, FALSE, TRUE);
                
                /* Turn off all SDP security */
                ConnectionSmSetSecurityLevel(protocol_l2cap, 1, ssp_secl4_l0, TRUE, FALSE, FALSE);
                
                if (cfm->version == bluetooth2_1)
                {
                    /* EIR inquiry mode */
                    ConnectionWriteInquiryMode(&the_app->task, inquiry_mode_eir);
                }
                
                /* Set default role switch policy */
                ConnectionSetRoleSwitchParams(NULL);
                
                if (!testEnterDutMode())
                {
                    initProfile();
                }
            }
            else
            {
                DEBUG_CL((" App failed to init CL\n"));
                Panic();
            }
            break;
        }    
        default:
        {
            unexpectedClMessage(app_state, CL_INIT_CFM);
            break;
        }
    }
}


/****************************************************************************
NAME
    handleClSmPinCodeInd

DESCRIPTION
    Handles the CL library PIN code indication message.

*/
static void handleClSmPinCodeInd(const CL_SM_PIN_CODE_IND_T *ind)
{
    devInstanceTaskData *inst = devInstanceFindFromBddr(&ind->bd_addr, TRUE);    
    char *pin;
    uint16 pin_idx;
    mvdProfiles remote_profiles;
    bool pin_authorised;
    
    if (inst != NULL)
    {                          
        /* See if this device is known about */
        if (!inst->paired_list_read && configureGetDeviceInfo (&ind->bd_addr, &pin_idx, &remote_profiles, &pin_authorised))
        {
            inst->pin_idx = pin_idx;
            inst->start_pin_idx = inst->pin_idx;
            inst->pin_authorised = pin_authorised;
            inst->remote_profiles |= remote_profiles;
            DEBUG_CL(("Read paired device details pin_idx[%d] start_idx[%d] auth[%d] profiles[0x%x]\n",pin_idx, pin_idx, pin_authorised, inst->remote_profiles));
        }       
        inst->paired_list_read = TRUE;
        
        /* Try and retrieve a pin code to use */
        pin = configureGetPinCode(inst);
        if (pin != NULL)
        {
            DEBUG_CL(("    Trying pin code %c%c%c%c\n",pin[0], pin[1], pin[2], pin[3]));
            ConnectionSmPinCodeResponse(&ind->bd_addr, 4, (unsigned char *)pin);
            return;
        }        
        
        /* Device intance can be deleted as all pin codes have been tried */
        MessageSend(&inst->task, APP_INTERNAL_DESTROY_REQ, 0);
        ConnectionSmPinCodeResponse(&ind->bd_addr, 0, NULL);
        DEBUG_CL(("    No pin code returned\n"));
    }
    else
    {
        ConnectionSmPinCodeResponse(&ind->bd_addr, 0, NULL);
        DEBUG_CL(("    No pin code returned\n"));
    }
}


/****************************************************************************
NAME
    handleClDmInquireResult

DESCRIPTION
    Handles the CL library inquiry result message.

*/
static void handleClDmInquireResult(const CL_DM_INQUIRE_RESULT_T *result)
{
    mvdAppState app_state = the_app->app_state;
    
    switch ( app_state )
    {
        case AppStateInquiring:
        {
            switch (result->status)
            {
            case inquiry_status_result:
                scanStoreInquireResult(result);
                break;
                
            case inquiry_status_ready:
                scanProcessNextInquireResult();
                break;
            }
            break;
        }    
        case AppStateIdle:
        {
            /* Silently ignore primitive - Inquiry scan will have timed out */
            DEBUG_CL((" - ignored\n"));
            break;
        }    
        default:
        {
            unexpectedClMessage(app_state, CL_DM_INQUIRE_RESULT);
            break;
        }
    }
}


/****************************************************************************
NAME
    handleClSdpOpenSearchCfm

DESCRIPTION
    Handles the CL library SDP open confirmation message.

*/
static void handleClSdpOpenSearchCfm(const CL_SDP_OPEN_SEARCH_CFM_T *cfm)
{
    devInstanceTaskData *inst = devInstanceFindFromBddr(&the_app->search_bdaddr, FALSE); 
                
    if ( (cfm->status == sdp_open_search_ok) && (inst != NULL) )
    {          
        if ( configureIsPinRequested(inst) )
        {
            (void)configureSetPinAuthorised(&the_app->search_bdaddr, TRUE);
        }
        (void)scanKickNextSdpSearch();
    }
    else
    {
        ConnectionSdpCloseSearchRequest(&the_app->task);  /* Generates a CL_SDP_CLOSE_SEARCH_CFM */
    }    
}


/****************************************************************************
NAME
    handleClSdpCloseSearchCfm

DESCRIPTION
    Handles the CL library SDP close confirmation message.

*/
static void handleClSdpCloseSearchCfm(void)
{
    devInstanceTaskData *inst = devInstanceFindFromBddr(&the_app->search_bdaddr, FALSE); 

    if ((inst != NULL) && configureIsPinRequested(inst) && !configureIsPinListExhausted(inst) )
    { 
        (void)configureSetPinAuthorised(&the_app->search_bdaddr, FALSE);
        scanKickFirstSdpSearch();    /* Pin code rejected, attempt to open a search session again */
    }
    else
    {
        scanSearchComplete(inst);
    }
}


/****************************************************************************
NAME
    handleClSdpServiceSearchCfm

DESCRIPTION
    Handles the CL library SDP service search confirmation message.

*/
static void handleClSdpServiceSearchCfm(CL_SDP_SERVICE_SEARCH_CFM_T *cfm)
{        
    mvdAppState app_state = the_app->app_state;
    
    switch (app_state)
    {
        case AppStateSearching:
        {
            devInstanceTaskData *inst = devInstanceFindFromBddr(&cfm->bd_addr, FALSE);
            
            if ((inst != NULL) && configureIsPinRequested(inst))
            {
                if (cfm->status==sdp_response_success)
                {
                    (void)configureSetPinAuthorised(&cfm->bd_addr, TRUE);
                }
                else
                {
                    if ( configureSetPinAuthorised(&cfm->bd_addr, FALSE) )
                    {    /* Authorisation came from bd_addr, resubmit SDP service search using next pin code in list */
                        (void)scanRepeatSdpSearch();
                    }
                }
            }
                
            (void)scanHandleSdpSearchResult(cfm);
            (void)scanKickNextSdpSearch();
            break;
        }    
        case AppStateIdle:
        {
            /* Silently ignore primitive - may occur if inquiry scan timeout is reached */
            DEBUG_CL((" - ignored\n"));
            break;
        }    
        default:
        {
            unexpectedClMessage(app_state, CL_SDP_SERVICE_SEARCH_CFM);
            break;
        }
    }
}


/****************************************************************************
NAME
    handleClRole

DESCRIPTION
    Checks the current role.

*/
static void handleClRole(devInstanceTaskData *inst, hci_role role, hci_status status, bool role_switch)
{
    Sink sink = 0;
    
    if (inst->a2dp)
        sink = inst->a2dp_sig_sink;
    else if (inst->aghfp_sink)
        sink = inst->aghfp_sink;
        
    /* store the current role */
    if ((status == hci_success) && (inst->role != role))
    {
        inst->role = role;
        DEBUG_CL(("    Current role: inst[0x%x] role:[%d]\n", (uint16)inst, inst->role));
        if (sink)
        {
            if (inst->role == hci_role_master)
            {
                /* Set link supervision timeout 5 seconds */
                ConnectionSetLinkSupervisionTimeout(sink, 0x1F80);
            }
#ifdef DUAL_STREAM 
            /* only care about role switching if two streams are going to be active */
            /* set this device to be master - only if no A2DP streaming active so as not to interrupt stream */
            if ((inst->role != hci_role_master) && role_switch && (inst->a2dp_state != A2dpStateStreaming))
            {         
                ConnectionSetRole(&the_app->task, sink, hci_role_master);
            }
#endif    
        }
#ifdef DUAL_STREAM         
        if (a2dpSlcIsMediaOpen() > 1)
        {
            /* For SBC codec update the audio quality based on current configuration */
            if (a2dpSeidIsSbc(inst->a2dp_seid))
            {
                a2dpMsgChooseSbcAudioQuality(inst);
            }
        }
#endif        
    }        
}


/****************************************************************************
NAME
    handleClRoleInd

DESCRIPTION
    Handles a role change indication.

*/
static void handleClRoleInd(const CL_DM_ROLE_IND_T *ind)
{
    devInstanceTaskData *inst = devInstanceFindFromBddr(&ind->bd_addr, FALSE);    
          
    if (inst != NULL)
    {
        handleClRole(inst, ind->role, ind->status, FALSE);     
    }
}


/****************************************************************************
NAME
    handleClRoleCfm

DESCRIPTION
    Handles the role switch result.

*/
static void handleClRoleCfm(const CL_DM_ROLE_CFM_T *cfm)
{
    bdaddr addr;
    devInstanceTaskData *inst = NULL;
 
    if (SinkGetBdAddr(cfm->sink, &addr))
        inst = devInstanceFindFromBddr(&addr, FALSE);               
    
    if (inst != NULL)
    {
        handleClRole(inst, cfm->role, cfm->status, TRUE);   
    }
}


/****************************************************************************
NAME
    handleClDmRemoteFeaturesConfirm

DESCRIPTION
    Handles the retrieved supported features of the remote device.

*/
static void handleClDmRemoteFeaturesConfirm(const CL_DM_REMOTE_FEATURES_CFM_T *cfm)
{
    uint16 i;    
    devInstanceTaskData *inst = NULL;
    devInstanceTaskData *temp_inst = NULL;
    bool found = FALSE;
    uint16 features[4];
    a2dpAudioQuality quality = A2DP_AUDIO_QUALITY_UNKNOWN;
    a2dpAudioQuality lowest_quality = A2DP_AUDIO_QUALITY_UNKNOWN;
    uint16 signalling_conns = 0;
    uint16 media_conns = 0;
    
    if (cfm->status == hci_success)
    {
        /* Look at all possible connections to find the device instance associated with the Sink returned */
        /* Look at all possible connections to get number of signalling connections, and media connections */
        for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
        {
            inst = the_app->dev_inst[i];
            if (inst != NULL)
            {         
                if ((cfm->sink == inst->aghfp_sink) || (cfm->sink == inst->a2dp_sig_sink))
                {
                    /* store the instance that was found */
                    found = TRUE;
                    temp_inst = inst;
                }   
                if (inst->a2dp_sig_sink)
                {
                    signalling_conns++;
                    if (inst->a2dp_media_sink)
                        media_conns++;
                }    
                if (inst->a2dp_audio_quality < lowest_quality)
                    lowest_quality = inst->a2dp_audio_quality;
            }
        }
    
        if (!found)
            return;
        
        /* retrieve the instance which was found */
        inst = temp_inst;

        /* Get supported features that both devices support */
        for (i = 0; i < 4; i++)
        {
            features[i] = the_app->local_supported_features[i];
            features[i] &= cfm->features[i];
        }

        /* Determine the sort of audio quality a link could support. */
        if ((features[1] & 0x0600) && (features[2] & 0x0180))
        {
            /* Both sides capable of supporting EDR ACL 2Mbps and/or 3Mbps with three or five slot packets */
            quality = A2DP_AUDIO_QUALITY_HIGH;
        }
        else if (features[0] & 0x0002)
        {
            /* Capable of supporting BR ACL 1Mbps with five slot packets */
            quality = A2DP_AUDIO_QUALITY_MEDIUM;
        }
        else
        {
            /* All other data rate and slot size combinations only capable of supporting a low data rate */
            quality = A2DP_AUDIO_QUALITY_LOW;
        }

        /* Update supported packet types and data rates */
        inst->a2dp_audio_quality = quality;
        
        DEBUG_CL(("    audio quality = %d\n", quality));    

#ifdef DUAL_STREAM        
#ifndef ALLOW_NON_EDR_DUALSTREAM        
        /* If there is a non-high quality link and there is already another connection -
           disconnect this connection as DualStream audio will be low quality */
        if ((signalling_conns > 1) && (inst->a2dp) && ((inst->a2dp_audio_quality != A2DP_AUDIO_QUALITY_HIGH) || (lowest_quality != A2DP_AUDIO_QUALITY_HIGH)))
        {
            setA2dpState(inst, A2dpStateDisconnecting);
            /* remove the media channel connection request from msg queue */
            MessageCancelAll(&inst->task, APP_INTERNAL_CONNECT_MEDIA_CHANNEL_REQ);
            A2dpDisconnectAll(inst->a2dp);
        }
#endif /* ALLOW_NON_EDR_DUALSTREAM */       
        /* The audio quality of one of the links has just been discovered -
            if there are two media connections open set new audio quality for SBC streams */
        if ((media_conns > 1) && (a2dpSeidIsSbc(inst->a2dp_seid)))
        {
            a2dpMsgChooseSbcAudioQuality(inst);
        }    
#endif /* DUAL_STREAM */       
    }
}


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    clMsgHandleLibMessage

DESCRIPTION
    Handles the CL library messages and calls the relevant function.

*/
void clMsgHandleLibMessage(MessageId id, Message message)
{
    switch(id)
    {
        case CL_INIT_CFM:
        {
            DEBUG_CL(("CL_INIT_CFM status = %u\n", ((CL_INIT_CFM_T *)message)->status));
            handleClInitCfm((CL_INIT_CFM_T *)message);    
            break;
        }    
        case CL_DM_WRITE_INQUIRY_MODE_CFM:
        {
            DEBUG_CL(("CL_DM_WRITE_INQUIRY_MODE_CFM\n"));
            /* Read the local name to put in our EIR data */
            ConnectionReadInquiryTx(&the_app->task);
            break;
        }    
        case CL_DM_READ_INQUIRY_TX_CFM:
        {
            the_app->inquiry_tx = ((CL_DM_READ_INQUIRY_TX_CFM_T*)message)->tx_power;
            ConnectionReadLocalName(&the_app->task);
            break;
        }    
        case CL_DM_LOCAL_NAME_COMPLETE:
        {
            DEBUG_CL(("CL_DM_LOCAL_NAME_COMPLETE\n"));
            /* Write EIR data and initialise the codec task */
            scanWriteEirData((CL_DM_LOCAL_NAME_COMPLETE_T*)message);
            break;
        }    
        case CL_DM_ACL_OPENED_IND:
        {
            DEBUG_CL(("CL_DM_ACL_OPENED_IND from: 0x%X 0x%X 0x%lX\n", ((CL_DM_ACL_OPENED_IND_T *)message)->bd_addr.nap, ((CL_DM_ACL_OPENED_IND_T *)message)->bd_addr.uap, ((CL_DM_ACL_OPENED_IND_T *)message)->bd_addr.lap));
            /* Ignore this message for now */
            DEBUG_CL((" - ignored\n"));
            break;
        }    
        case CL_DM_ACL_CLOSED_IND:
        {
            DEBUG_CL(("CL_DM_ACL_CLOSED_IND from: 0x%X 0x%X 0x%lX\n", ((CL_DM_ACL_CLOSED_IND_T *)message)->bd_addr.nap, ((CL_DM_ACL_CLOSED_IND_T *)message)->bd_addr.uap, ((CL_DM_ACL_CLOSED_IND_T *)message)->bd_addr.lap));
            /* Ignore this message for now */
            DEBUG_CL((" - ignored\n"));
            break;
        }
        case CL_SM_PIN_CODE_IND:
        {
            DEBUG_CL(("CL_SM_PIN_CODE_IND from: 0x%X 0x%X 0x%lX\n", (uint16)((CL_SM_PIN_CODE_IND_T *)message)->bd_addr.nap, (uint16)((CL_SM_PIN_CODE_IND_T *)message)->bd_addr.uap, (uint32)((CL_SM_PIN_CODE_IND_T *)message)->bd_addr.lap));
            handleClSmPinCodeInd((CL_SM_PIN_CODE_IND_T *)message);
            break;
        }
        case CL_SM_IO_CAPABILITY_REQ_IND:
        {
            DEBUG_CL(("CL_SM_IO_CAPABILITY_REQUEST_IND\n"));
            {
                CL_SM_IO_CAPABILITY_REQ_IND_T *prim = (CL_SM_IO_CAPABILITY_REQ_IND_T *)message;
                ConnectionSmIoCapabilityResponse(&prim->bd_addr, cl_sm_io_cap_no_input_no_output, FALSE, TRUE, FALSE, NULL, NULL);
            }
            break;
        }
        case CL_SM_USER_CONFIRMATION_REQ_IND:
        {
            DEBUG_CL(("CL_SM_USER_CONFIRMATION_REQ_IND\n"));
            /* Shouldn't get this so if we do reject it! */
            ConnectionSmUserConfirmationResponse(&((CL_SM_USER_CONFIRMATION_REQ_IND_T*)message)->bd_addr, FALSE);
            break;
        }
        case CL_SM_AUTHORISE_IND:
        {
            DEBUG_CL(("CL_SM_AUTHORISE_IND\n"));
            {    /* For now, blindly accept this request */
                CL_SM_AUTHORISE_IND_T *prim = (CL_SM_AUTHORISE_IND_T *)message;
                ConnectionSmAuthoriseResponse(&prim->bd_addr, prim->protocol_id, prim->channel, prim->incoming, TRUE);
            }    
            break;
        }
        case CL_SM_AUTHENTICATE_CFM:
        {
            DEBUG_CL(("CL_SM_AUTHENTICATE_CFM status = %u\n", ((CL_SM_AUTHENTICATE_CFM_T *)message)->status));
            if ( ((CL_SM_AUTHENTICATE_CFM_T *)message)->status == auth_status_success )
            {    /* Pin code will be stored on a successful SLC/A2DP connection */
                (void)configureSetPinAuthorised(&((CL_SM_AUTHENTICATE_CFM_T *)message)->bd_addr, TRUE);
            }
            break;
        }
        case CL_SM_SECURITY_LEVEL_CFM:
        {
            DEBUG_CL(("CL_SM_SECURITY_LEVEL_CFM success = %u\n", ((CL_SM_SECURITY_LEVEL_CFM_T *)message)->success));
            break;
        }
        case CL_DM_INQUIRE_RESULT:
        {
            DEBUG_CL(("CL_DM_INQUIRE_RESULT status = %u\n", ((CL_DM_INQUIRE_RESULT_T *)message)->status));
            handleClDmInquireResult((CL_DM_INQUIRE_RESULT_T *)message);        
            break;
        }
        case CL_SDP_OPEN_SEARCH_CFM:
        {   
            DEBUG_CL(("CL_SDP_OPEN_SEARCH_CFM status = %u\n", ((CL_SDP_OPEN_SEARCH_CFM_T *)message)->status)); 
            handleClSdpOpenSearchCfm((CL_SDP_OPEN_SEARCH_CFM_T *)message);       
            break;
        }    
        case CL_SDP_CLOSE_SEARCH_CFM:
        {
            DEBUG_CL(("CL_SDP_CLOSE_SEARCH_CFM status = %u\n", ((CL_SDP_CLOSE_SEARCH_CFM_T *)message)->status));
            handleClSdpCloseSearchCfm();      
            break;
        }    
        case CL_SDP_SERVICE_SEARCH_CFM:
        {
            DEBUG_CL(("CL_SDP_SERVICE_SEARCH_CFM status = %u\n", ((CL_SDP_SERVICE_SEARCH_CFM_T *)message)->status));
            handleClSdpServiceSearchCfm((CL_SDP_SERVICE_SEARCH_CFM_T *)message);          
            break;
        }    
        case CL_SM_SEC_MODE_CONFIG_CFM:
        {
            DEBUG_CL(("CL_SM_SEC_MODE_CONFIG_CFM\n"));
            DEBUG_CL((" - ignored\n"));
            break;
        }    
        case CL_SM_REMOTE_IO_CAPABILITY_IND:
        {
            DEBUG_CL(("CL_SM_REMOTE_IO_CAPABILITY_IND\n"));
            DEBUG_CL((" - ignored\n"));
            break;
        }
        case CL_DM_LINK_SUPERVISION_TIMEOUT_IND:
        {
            DEBUG_CL(("CL_DM_LINK_SUPERVISION_TIMEOUT_IND:\n"));
            DEBUG_CL(("    timeout:[0x%x] bdaddr:[0x%x%x%lx]\n", 
                        ((CL_DM_LINK_SUPERVISION_TIMEOUT_IND_T *)message)->timeout,
                        ((CL_DM_LINK_SUPERVISION_TIMEOUT_IND_T *)message)->bd_addr.nap,
                        ((CL_DM_LINK_SUPERVISION_TIMEOUT_IND_T *)message)->bd_addr.uap,
                        ((CL_DM_LINK_SUPERVISION_TIMEOUT_IND_T *)message)->bd_addr.lap));
            break;
        }
        case CL_DM_ROLE_IND:
        {
            DEBUG_CL(("CL_DM_ROLE_IND\n"));
            handleClRoleInd((CL_DM_ROLE_IND_T *)message);
            break;
        }
        case CL_DM_ROLE_CFM:
        {
            DEBUG_CL(("CL_DM_ROLE_CFM\n"));
            handleClRoleCfm((CL_DM_ROLE_CFM_T *)message);
            break;
        }
        case CL_DM_SNIFF_SUB_RATING_IND:
        {
            DEBUG_CL(("CL_DM_SNIFF_SUB_RATING_IND\n"));
            break;
        }
        case CL_DM_REMOTE_FEATURES_CFM:
        {
            DEBUG_CL(("CL_DM_REMOTE_FEATURES_CFM\n"));
            handleClDmRemoteFeaturesConfirm((CL_DM_REMOTE_FEATURES_CFM_T *)message);
            return;
        }
        default:
        {
            DEBUG_CL(("Unhandled CL message 0x%X\n", (uint16)id));    
            break;
        }
    }
}


