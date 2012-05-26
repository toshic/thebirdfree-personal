/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_hfp_msg_handler.c
@brief    Handle hfp library messages arriving at the app.
*/


#include "headset_debug.h"
#include "headset_hfp_handler.h"
#include "headset_hfp_msg_handler.h"
#include "headset_private.h"
#include "headset_statemanager.h"
#include "headset_states.h"

#include "headset_csr_features.h"

#include <hfp.h>
#include <panic.h>


#ifdef DEBUG_HFP_MSG
#define HFP_MSG_DEBUG(x) DEBUG(x)
#else
#define HFP_MSG_DEBUG(x) 
#endif


/****************************************************************************
  FUNCTIONS
*/

/*************************************************************************
NAME    
    unhandledHfpState
    
DESCRIPTION
    Indicates if a message arrived in an unhandled state.
    
*/
static void unhandledHfpState(headsetHfpState state, MessageId id)
{
	state = state;
	id = id;

    HFP_MSG_DEBUG(("Unhandled HFP: current state %d message id 0x%x\n", state, id));
}


/****************************************************************************/
void handleHFPMessage( Task task, MessageId id, Message message )
{
    headsetHfpState lHfpState = stateManagerGetHfpState();
    
    switch (id)
    {
        
    case HFP_INIT_CFM:
        HFP_MSG_DEBUG(("HFP_INIT_CFM\n"));
        hfpHandlerInitCfm( (HFP_INIT_CFM_T *) message );
        break;
        
    case HFP_SLC_CONNECT_IND:
        HFP_MSG_DEBUG(("HFP_SLC_CONNECT_IND\n"));            
        hfpHandlerConnectInd( (HFP_SLC_CONNECT_IND_T *) message );
        break;
        
    case HFP_SLC_CONNECT_CFM:
        HFP_MSG_DEBUG(("HFP_SLC_CONNECT_CFM\n"));  
        hfpHandlerConnectCfm( (HFP_SLC_CONNECT_CFM_T *) message );
        break;
        
    case HFP_SLC_DISCONNECT_IND:
        HFP_MSG_DEBUG(("HFP_SLC_DISCONNECT_IND\n"));
        hfpHandlerDisconnectInd( (HFP_SLC_DISCONNECT_IND_T *) message );
        break;
    
    case HFP_IN_BAND_RING_IND:
        HFP_MSG_DEBUG(("HFP_IN_BAND_RING_IND\n"));
        hfpHandlerInbandRingInd( (HFP_IN_BAND_RING_IND_T *) message );
        break;
        
    case HFP_CALL_IND:
        HFP_MSG_DEBUG(("HFP_CALL_IND [%d]\n",((HFP_CALL_IND_T *)message)->call   ));        
        switch (lHfpState)
        {
            /* Allowed states */
            case headsetConnDiscoverable:
            case headsetHfpConnectable:
            case headsetHfpConnected:
            case headsetIncomingCallEstablish:
            case headsetOutgoingCallEstablish:
            case headsetActiveCall: 
                hfpHandlerCallInd( (HFP_CALL_IND_T *) message );
                break;

            case headsetTWCWaiting:
            case headsetTWCOnHold:
            case headsetTWCMulticall:
                hfpHandlerThreeWayCallInd( (HFP_CALL_IND_T *)message );
                break;

            /* Disallowed states */
            case headsetPoweringOn:            
            case headsetTestMode:      
            default:    
                unhandledHfpState(lHfpState, id);
			    break ;
        }        
        break;
        
    case HFP_CALL_SETUP_IND:
        HFP_MSG_DEBUG(("HFP_CALL_SETUP_IND [%d]\n", ((HFP_CALL_SETUP_IND_T * )message)->call_setup ));
        switch(lHfpState)
        {
            /* Allowed states */
            case headsetConnDiscoverable:
            case headsetHfpConnectable:
            case headsetHfpConnected:
            case headsetIncomingCallEstablish:
            case headsetOutgoingCallEstablish:
                hfpHandlerCallSetupInd( (HFP_CALL_SETUP_IND_T *) message );
                break;
            
            case headsetTWCWaiting:
            case headsetTWCOnHold:
            case headsetTWCMulticall:
            case headsetActiveCall:
                hfpHandlerThreeWayCallSetupInd( (HFP_CALL_SETUP_IND_T *)message );
                break;
            
            /* Ignored states
            case headsetActiveCall:
                break;*/
                
            /* Disallowed states */
            case headsetPoweringOn:            
            case headsetTestMode:      
            default:            
                unhandledHfpState(lHfpState, id);
			    break ;
        }
        break;  
        
    case HFP_RING_IND:
        HFP_MSG_DEBUG(("HFP_RING_IND\n"));
        switch(lHfpState)
        {
            /* Allowed states */            
            case headsetHfpConnected:
            case headsetIncomingCallEstablish:
            case headsetOutgoingCallEstablish:
            case headsetActiveCall: 
            case headsetTWCWaiting:
            case headsetTWCOnHold:
            case headsetTWCMulticall:
                hfpHandlerRingInd();
                break;
            
            /* Disallowed states */
            case headsetPoweringOn:
            case headsetConnDiscoverable:
            case headsetHfpConnectable:
            case headsetTestMode:      
            default:          
                unhandledHfpState(lHfpState, id);
			    break ;
        }
        break;  
        
    case HFP_VOICE_RECOGNITION_IND:
        HFP_MSG_DEBUG(("HFP_VOICE_RECOGNITION_IND %d\n",((HFP_VOICE_RECOGNITION_IND_T *) message)->enable));
        switch(lHfpState)
        {
            /* Allowed states */     
            case headsetHfpConnected:
            case headsetOutgoingCallEstablish:
                hfpHandlerVoiceRecognitionInd( (HFP_VOICE_RECOGNITION_IND_T *) message );
                break;

            /* Disallowed states */
            case headsetActiveCall:
            case headsetIncomingCallEstablish:
            case headsetPoweringOn:
            case headsetConnDiscoverable:
            case headsetHfpConnectable:
            case headsetTestMode:  
            default:
                unhandledHfpState(lHfpState, id);
                break;                
        }
        break;
         
    case HFP_VOICE_RECOGNITION_ENABLE_CFM:
        HFP_MSG_DEBUG(("HFP_VOICE_RECOGNITION_ENABLE_CFM %d\n",((HFP_VOICE_RECOGNITION_ENABLE_CFM_T *) message)->status));
        switch(lHfpState)
        {
            /* Allowed states */     
            case headsetHfpConnected:
                hfpHandlerVoiceRecognitionCfm( (HFP_VOICE_RECOGNITION_ENABLE_CFM_T *) message );
                break;
                
            /* Disallowed states */
            case headsetActiveCall:
            case headsetIncomingCallEstablish:
            case headsetOutgoingCallEstablish:
            case headsetPoweringOn:
            case headsetConnDiscoverable:
            case headsetHfpConnectable:
            case headsetTestMode:  
            default:
                unhandledHfpState(lHfpState, id);
                break;                
        }
        break;
		
	case HFP_LAST_NUMBER_REDIAL_CFM:
		HFP_MSG_DEBUG(("HFP_LAST_NUMBER_REDIAL_CFM %d\n",((HFP_LAST_NUMBER_REDIAL_CFM_T *) message)->status));
        switch(lHfpState)
        {
            /* Allowed states */     
            case headsetHfpConnected:
			case headsetActiveCall:
				hfpHandlerLastNoRedialCfm( (HFP_LAST_NUMBER_REDIAL_CFM_T *) message );
                break;
                
            /* Disallowed states */            
            case headsetIncomingCallEstablish:
            case headsetOutgoingCallEstablish:
            case headsetPoweringOn:
            case headsetConnDiscoverable:
            case headsetHfpConnectable:
            case headsetTestMode:  
            default:
                unhandledHfpState(lHfpState, id);
                break;                
        }
        break;
        
    case HFP_ENCRYPTION_CHANGE_IND:
        HFP_MSG_DEBUG(("HFP_ENCRYPTION_CHANGE_IND %d\n",((HFP_ENCRYPTION_CHANGE_IND_T *)message)->encrypted));
        switch(lHfpState)
        {
            /* Allowed states */     
            case headsetHfpConnected:
            case headsetConnDiscoverable:
            case headsetHfpConnectable:
            case headsetActiveCall:
            case headsetIncomingCallEstablish:
            case headsetOutgoingCallEstablish:
                hfpHandlerEncryptionChangeInd( (HFP_ENCRYPTION_CHANGE_IND_T *) message );
                break;
                
            /* Disallowed states */            
            case headsetPoweringOn:            
            case headsetTestMode:  
            default:
                unhandledHfpState(lHfpState, id);
                break;                
        }
        break;
        
    case HFP_SPEAKER_VOLUME_IND:
        HFP_MSG_DEBUG(("HFP_SPEAKER_VOLUME_IND %d\n",((HFP_SPEAKER_VOLUME_IND_T *)message)->volume_gain));
        hfpHandlerSpeakerVolumeInd( (HFP_SPEAKER_VOLUME_IND_T *) message );
        break;
        
    case HFP_AUDIO_CONNECT_IND:
        HFP_MSG_DEBUG(("HFP_AUDIO_CONNECT_IND\n"));
        hfpHandlerAudioConnectInd( (HFP_AUDIO_CONNECT_IND_T *) message );
        break;
        
    case HFP_AUDIO_CONNECT_CFM:
        HFP_MSG_DEBUG(("HFP_AUDIO_CONNECT_CFM\n"));
        hfpHandlerAudioConnectCfm( (HFP_AUDIO_CONNECT_CFM_T *) message );
        break;
        
    case HFP_AUDIO_DISCONNECT_IND:
        HFP_MSG_DEBUG(("HFP_AUDIO_DISCONNECT_IND\n"));
        hfpHandlerAudioDisconnectInd( (HFP_AUDIO_DISCONNECT_IND_T *) message );
        break;
        
	/* CSR Specific Messages */
	case HFP_CSR_SUPPORTED_FEATURES_CFM:
		HFP_MSG_DEBUG(("HFP_CSR_SUPPORTED_FEATURES_CFM\n"));
		break ;		
		
	case HFP_CSR_FEATURE_NEGOTIATION_IND:
		csr2csrFeatureNegotiationInd((HFP_CSR_FEATURE_NEGOTIATION_IND_T *)message );
		break ;
    
    /* three way calling msgs */
    case HFP_CALL_WAITING_ENABLE_CFM:
		HFP_MSG_DEBUG(("HFP_CALL_WAITING_ENABLE_CFM\n"));
		hfpHandlerCallWaitingEnableCfm((HFP_CALL_WAITING_ENABLE_CFM_T *)message);
		break;
    case HFP_CALL_WAITING_IND:            
		hfpHandlerCallWaitingInd((HFP_CALL_WAITING_IND_T *)message);
		HFP_MSG_DEBUG(("HFP_CALL_WAITING_IND\n"));
		break;
    case HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM:
		HFP_MSG_DEBUG(("HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM\n"));
 		hfpHandlerReleaseHeldRejectWaitingCallCfm((HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM_T *)message);
		break;
    case HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM:
		HFP_MSG_DEBUG(("HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM\n"));
 		hfpHandlerReleaseActiveAcceptOtherCallCfm((HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM_T *)message);
		break;
    case HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM:
 		HFP_MSG_DEBUG(("HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM\n"));
 		hfpHandlerHoldActiveAcceptOtherCallCfm((HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM_T *)message);
		break;
    case HFP_ADD_HELD_CALL_CFM:
		HFP_MSG_DEBUG(("HFP_ADD_HELD_CALL_CFM\n"));
		hfpHandlerAddHeldCallCfm((HFP_ADD_HELD_CALL_CFM_T *)message);
		break;
    case HFP_EXPLICIT_CALL_TRANSFER_CFM:
		HFP_MSG_DEBUG(("HFP_EXPLICIT_CALL_TRANSFER_CFM\n"));
		hfpHandlerExplicitCallTransferCfm((HFP_EXPLICIT_CALL_TRANSFER_CFM_T *)message);
		break;
    /* end three way calling msgs */

    /* Ignored messages */
    case HFP_UNRECOGNISED_AT_CMD_IND:    
    case HFP_MICROPHONE_VOLUME_CFM:
    case HFP_MICROPHONE_VOLUME_IND:
    case HFP_HS_BUTTON_PRESS_CFM:
    case HFP_CALLER_ID_IND:    
    case HFP_DIAL_NUMBER_CFM:
    case HFP_DIAL_MEMORY_CFM:
    case HFP_SERVICE_IND:
    case HFP_DISABLE_NREC_CFM:
    case HFP_CALLER_ID_ENABLE_CFM:
    case HFP_DTMF_CFM:
	case HFP_SINK_CFM:
    case HFP_VOICE_TAG_NUMBER_CFM:
    case HFP_ANSWER_CALL_CFM:
    case HFP_REJECT_CALL_CFM:
    case HFP_TERMINATE_CALL_CFM:
    case HFP_EXTRA_INDICATOR_INDEX_IND:
    case HFP_EXTRA_INDICATOR_UPDATE_IND:
    case HFP_SPEAKER_VOLUME_CFM:
    case HFP_SIGNAL_IND:
	case HFP_ROAM_IND:
	case HFP_BATTCHG_IND:
	case HFP_REMOTE_AG_PROFILE15_IND:
	case HFP_ENCRYPTION_KEY_REFRESH_IND:
	case HFP_CSR_TXT_IND:	
	case HFP_CSR_MODIFY_INDICATORS_CFM:
    case HFP_CSR_NEW_SMS_IND:  
    case HFP_CSR_NEW_SMS_NAME_IND:        
	case HFP_CSR_SMS_CFM:   
	case HFP_CSR_MODIFY_AG_INDICATORS_IND: 
	case HFP_CSR_AG_INDICATORS_DISABLE_IND:   
	case HFP_CSR_AG_REQUEST_BATTERY_IND:
        break;        
        
    default:    
        HFP_MSG_DEBUG(("HFP UNHANDLED MSG: 0x%x\n",id));
        break;
    }    
}
