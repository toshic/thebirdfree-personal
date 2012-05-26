/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	profile_handler.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_audio_handler.h"
#include "hfp_call_handler.h"
#include "hfp_caller_id_handler.h"
#include "hfp_common.h"
#include "hfp_current_calls_handler.h"
#include "hfp_dial_handler.h"
#include "hfp_dtmf_handler.h"
#include "hfp_extended_error_handler.h"
#include "hfp_hs_handler.h"
#include "hfp_indicators_handler.h"
#include "hfp_multiple_calls_handler.h"
#include "hfp_network_operator_handler.h"
#include "hfp_nrec_handler.h"
#include "hfp_ok.h"
#include "hfp_receive_data.h"
#include "hfp_rfc.h"
#include "hfp_ring_handler.h"
#include "hfp_sdp.h"
#include "hfp_security_handler.h"
#include "hfp_send_data.h"
#include "hfp_slc_handler.h"
#include "hfp_sound_handler.h"
#include "hfp_voice_handler.h"
#include "hfp_voice_tag_handler.h"
#include "hfp_csr_features_handler.h"
#include "hfp_subscriber_num_handler.h"
#include "hfp_response_hold_handler.h"
#include "hfp_current_calls_handler.h"
#include "hfp_network_operator_handler.h"
#include "hfp_extended_error_handler.h"
#include "init.h"
#include "profile_handler.h"

#include <print.h>

/*lint -e525 -e725 -e830 */

typedef enum
{ 
	hfpUnexpectedClPrim,
	hfpUnexpectedHfpPrim,
	hfpUnexpectedMessage
} hfpUnexpectedReasonCode;


/****************************************************************************
NAME	 
    handleUnexpected	

DESCRIPTION
    This function is called as a result of a message arriving when this
	library was not expecting it.

RETURNS
    void	
*/
#ifdef HFP_DEBUG_LIB
static void handleUnexpected(hfpUnexpectedReasonCode code, hfpState state, uint16 type)
{
    HFP_DEBUG(("hfp handleUnexpected - Code 0x%x State 0x%x MsgId 0x%x\n", code, state, type));
}
#else
#define handleUnexpected(code, state, type)
#endif


/****************************************************************************
NAME	
	hfpProfileHandler

DESCRIPTION
	All messages for this profile lib are handled by this function

RETURNS
	void
*/
void hfpProfileHandler(Task task, MessageId id, Message message)
{
	HFP *hfp = (HFP *) task;
	hfpState profileState = hfp->state;

	/* Check the message id */
	switch (id)
	{
	    case HFP_INTERNAL_INIT_REQ:
	    {
		    PRINT(("HFP_INTERNAL_INIT_REQ\n"));
		    switch(profileState)
		    {
		        case hfpInitialising:
			        hfpHandleInternalInitReq(hfp);
			        break;

		        default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
		    }
		}
		return;

    	case HFP_INTERNAL_INIT_CFM:
	    {
	        PRINT(("HFP_INTERNAL_INIT_CFM\n"));
    		switch(profileState)
	    	{		
		        case hfpInitialising:
			        hfpHandleInternalInitCfm(hfp, (HFP_INTERNAL_INIT_CFM_T *) message);
			        break;
		
		        default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
    		}
	    }
	    return;

	    case HFP_INTERNAL_SDP_REGISTER_CFM:
	    {
		    PRINT(("HFP_INTERNAL_SDP_REGISTER_CFM\n"));
    		switch(profileState)
	    	{		
    		    case hfpInitialising:
			        hfpHandleSdpInternalRegisterInit(hfp, (HFP_INTERNAL_SDP_REGISTER_CFM_T *) message);
			        break;
		
		        case hfpReady:
                case hfpSlcConnecting:
			        hfpHandleSdpInternalRegisterCfm(hfp, (HFP_INTERNAL_SDP_REGISTER_CFM_T *) message);
			        break;

		        default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
    		}
        }
        return;

	    case HFP_INTERNAL_SLC_CONNECT_REQ:
	    {
		    PRINT(("HFP_INTERNAL_SLC_CONNECT_REQ\n"));
    		switch(profileState)
	    	{
		        case hfpReady:
			        hfpHandleSlcConnectRequest(hfp, (HFP_INTERNAL_SLC_CONNECT_REQ_T *) message);
			        break;
				
		        case hfpSlcConnecting:
		        case hfpSlcConnected:
		        case hfpIncomingCallEstablish:
		        case hfpOutgoingCallEstablish:
		        case hfpOutgoingCallAlerting:
		        case hfpActiveCall:
			        /* If this profile instance is already connecting/connected  then
				       reject the SLC connect request and inform the app. */
			        hfpSendSlcConnectCfmToApp(hfp_connect_failed_busy, hfp);
			        break;

		        default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
    		}
	    }
	    return;
		
	    case HFP_INTERNAL_SLC_CONNECT_RES:
	    {
		    PRINT(("HFP_INTERNAL_SLC_CONNECT_RES\n"));
		    switch(profileState)
		    {		
		        case hfpSlcConnecting:
			        hfpHandleSlcConnectResponse(hfp, (HFP_INTERNAL_SLC_CONNECT_RES_T *) message);
			        break;

		        case hfpReady:
		        case hfpSlcConnected:
		        case hfpIncomingCallEstablish:
		        case hfpOutgoingCallEstablish:
		        case hfpOutgoingCallAlerting:
		        case hfpActiveCall:
			        /* 
				        If the app is sending us an SLC connect response when we're not 
				        connecting then send it an error message since we currently don't 
				        have an SLC being established.
			        */
			        hfpSendSlcConnectCfmToApp(hfp_connect_failed_busy, hfp);
			        break;

        		default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
		    }
		}
		return;

        case HFP_INTERNAL_RFCOMM_CONNECT_REQ:
        {
            PRINT(("HFP_INTERNAL_RFCOMM_CONNECT_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                    hfpHandleRfcommConnectRequest(hfp, (HFP_INTERNAL_RFCOMM_CONNECT_REQ_T *) message);
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_SLC_DISCONNECT_REQ:
        {
            PRINT(("HFP_INTERNAL_SLC_DISCONNECT_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    hfpHandleDisconnectRequest(hfp);
                    break;

                case hfpReady:
                    /* Send disconnect message with error - nothing to disconnect */
                    hfpHandleDisconnectRequestFail(hfp);
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_SLC_DISCONNECT_IND:
        {
            PRINT(("HFP_INTERNAL_SLC_DISCONNECT_IND\n"));
            switch(profileState)
            {
                case hfpReady:
                case hfpSlcConnecting:
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    hfpHandleSlcDisconnectIndication(hfp, (HFP_INTERNAL_SLC_DISCONNECT_IND_T *) message);
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_GET_SINK_REQ:
        {
            PRINT(("HFP_INTERNAL_GET_SINK_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    /* If we have a valid sink send it back */
                    hfpHandleSinkRequest(hfp);
                    break;

                case hfpReady:
                case hfpSlcConnecting:
                    /* Send an error message we're in the wrong state */
                    hfpHandleSinkRequestFail(hfp);
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_WAIT_AT_TIMEOUT_IND:
        {
            PRINT(("HFP_INTERNAL_WAIT_AT_TIMEOUT_IND\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    hfpHandleWaitAtTimeout(hfp);
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_INDICATOR_LIST_IND:
        {
            PRINT(("HFP_INTERNAL_AT_INDICATOR_LIST_IND\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    hfpHandleIndicatorListInd(hfp, (HFP_INTERNAL_AT_INDICATOR_LIST_IND_T *) message);
                    break;
                    
                case hfpReady:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    /* This command issued as part of SLC connect process.  We can get here if underlying RfComm
                    connection is closed before SLC connect has completed. */
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_INDICATOR_STATUS_IND:
        {
            PRINT(("HFP_INTERNAL_AT_INDICATOR_STATUS_IND\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    hfpHandleIndicatorInitialStatusInd(hfp, (HFP_INTERNAL_AT_INDICATOR_STATUS_IND_T *) message);
                    break;

                case hfpReady:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    /* This command issued as part of SLC connect process.  We can get here if underlying RfComm
                    connection is closed before SLC connect has completed. */
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_CALL_HOLD_SUPPORT_IND:
        {
            PRINT(("HFP_INTERNAL_AT_CALL_HOLD_SUPPORT_IND\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    hfpHandleCallHoldSupportInd(hfp);
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }        
        return;
        
        case HFP_INTERNAL_AT_BRSF_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_BRSF_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    hfpHandleBrsfRequest(hfp);
                    break;

                case hfpReady:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    /* This command issued as part of SLC connect process.  We can get here if underlying RfComm
                    connection is closed before SLC connect has completed. */
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_CIND_TEST_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CIND_TEST_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    hfpHandleCindTestRequest(hfp);
                    break;

                case hfpReady:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    /* This command issued as part of SLC connect process.  We can get here if underlying RfComm
                    connection is closed before SLC connect has completed. */
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_CIND_READ_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CIND_READ_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    hfpHandleCindReadRequest(hfp);
                    break;

                case hfpReady:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    /* This command issued as part of SLC connect process.  We can get here if underlying RfComm
                    connection is closed before SLC connect has completed. */
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_CMER_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CMER_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    hfpHandleCmerRequest(hfp);
                    break;

                case hfpReady:
                    /* Only allowed if we are an HFP device */
                    checkHfpProfile(hfp->hfpSupportedProfile);
                    /* This command issued as part of SLC connect process.  We can get here if underlying RfComm
                    connection is closed before SLC connect has completed. */
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_CKPD_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CKPD_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpActiveCall:
                    if (supportedProfileIsHsp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HSP device */
                        hfpHandleHsButtonPress(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                    {
                        /* Send an error message to the application. */
                        hfpHandleHsButtonPressError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_ATA_REQ:
        {
            PRINT(("HFP_INTERNAL_ATA_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */			
                        hfpHandleAnswerCall(hfp);
                    }
                    else
                case hfpSlcConnecting:
                case hfpReady:
                    {
                        /* Send an error message to the application */
                        hfpHandleAnswerCallError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_CHUP_REJECT_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CHUP_REJECT_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleRejectCall(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {
                        /* Send an error message */
                        hfpHandleRejectCallError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_CHUP_TERMINATE_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CHUP_TERMINATE_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleTerminateCall(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                case hfpIncomingCallEstablish:
                    {
                        /* Send an error message */
                        hfpHandleTerminateCallError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_BLDN_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_BLDN_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleLastNumberRedial(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {
                        /* Send an error message to the app. */
                        hfpHandleLastNumberRedialError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_ATD_NUMBER_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_ATD_NUMBER_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */			
                        hfpHandleDialNumberRequest(hfp, (HFP_INTERNAL_AT_ATD_NUMBER_REQ_T *) message);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {
                        /* Send an error message */
                        hfpHandleDialNumberRequestError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_ATD_MEMORY_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_ATD_MEMORY_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleDialMemoryRequest(hfp, (HFP_INTERNAL_AT_ATD_MEMORY_REQ_T *) message);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {
                        /* Send an error message */
                        hfpHandleDialMemoryRequestError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_BVRA_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_BVRA_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleVoiceRecognitionEnable(hfp, (HFP_INTERNAL_AT_BVRA_REQ_T *) message);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {
                        /* Send error message */
                        hfpHandleVoiceRecognitionEnableError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_VGS_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_VGS_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting: 		  
                case hfpActiveCall:
                    /* Allowed for HSP and HFP */
                    hfpHandleVgsRequest(hfp, (HFP_INTERNAL_AT_VGS_REQ_T *) message);
                    break;

                case hfpReady:
                case hfpSlcConnecting:
                    /* Send error message to indicate request failed */
                    hfpHandleVgsRequestError(hfp);
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_VGM_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_VGM_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    /* Allowed for HSP and HFP */
                    hfpHandleVgmRequest(hfp, (HFP_INTERNAL_AT_VGM_REQ_T *) message);
                    break;

                case hfpReady:
                case hfpSlcConnecting:
                    /* Send an error message */
                    hfpHandleVgmRequestError(hfp);
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_CLIP_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CLIP_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleCallerIdEnableReq(hfp, (HFP_INTERNAL_AT_CLIP_REQ_T *) message);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {
                        /* Send error response */
                        hfpHandleCallerIdEnableReqError(hfp);
                    }
                    break;
                    
                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;                    

        case HFP_INTERNAL_AT_BINP_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_BINP_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleGetVoiceTagReq(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {
                        /* Send an error message */
                        hfpHandleGetVoiceTagReqError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_NREC_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_NREC_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:	
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleNrEcDisable(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {
                        /* Send an error message */	
                        hfpHandleNrEcDisableError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_VTS_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_VTS_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:		
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleDtmfRequest(hfp, (HFP_INTERNAL_AT_VTS_REQ_T *) message);
                    }
                    else
                    {
                case hfpReady:
                case hfpSlcConnecting:
                        /* Send an error message */	
                        hfpHandleDtmfRequestError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_CCWA_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CCWA_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleCallWaitingNotificationEnable(hfp, (HFP_INTERNAL_AT_CCWA_REQ_T *) message);
                    }
                    else
                    {
                case hfpReady:
                case hfpSlcConnecting:
                        /* Send an error message */
                        hfpHandleCallWaitingNotificationEnableError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_CHLD_0_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CHLD_0_REQ\n"));
            switch(profileState)
            {		
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:		
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleChldZero(hfp);
                    }
                    else                    
                case hfpReady:
                case hfpSlcConnecting:
                case hfpSlcConnected:
                    {
                        /* Send an error message */
                        hfpHandleChldZeroError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case HFP_INTERNAL_AT_CHLD_1_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CHLD_1_REQ\n"));
            switch(profileState)
            {		
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:		
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */                        
                        hfpHandleChldOne(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                case hfpSlcConnected:
                    {
                        /* Send an error message */
                        hfpHandleChldOneError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_CHLD_1X_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CHLD_1X_REQ\n"));
            switch(profileState)
            {		
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:		
                case hfpActiveCall:
                    if (supportedProfileIsHfp15(hfp->hfpSupportedProfile) && supportedProfileIsHfp15(hfp->agSupportedProfile))
                    {	/* Only allowed if we are an HFP v1.5 device talking to an AG supporting HFP v1.5 */
                        hfpHandleChldOneIdx(hfp, (HFP_INTERNAL_AT_CHLD_1X_REQ_T *)message);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                case hfpSlcConnected:
                    {	/* Send an error message */
                        hfpHandleChldOneIdxError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_CHLD_2_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CHLD_2_REQ\n"));
            switch(profileState)
            {		
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleChldTwo(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                case hfpSlcConnected:
                    {
                        /* Send an error message */
                        hfpHandleChldTwoError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_CHLD_2X_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CHLD_2X_REQ\n"));
            switch(profileState)
            {		
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp15(hfp->hfpSupportedProfile) && supportedProfileIsHfp15(hfp->agSupportedProfile))
                    {	
                        /* Only allowed if we are an HFP v1.5 device talking to an AG supporting HFP v1.5 */
                        hfpHandleChldTwoIdx(hfp, (HFP_INTERNAL_AT_CHLD_2X_REQ_T *)message);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                case hfpSlcConnected:
                    {	/* Send an error message */
                        hfpHandleChldTwoIdxError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_CHLD_3_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CHLD_3_REQ\n"));
            switch(profileState)
            {		
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleChldThree(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                case hfpSlcConnected:
                    {
                        /* Send an error message */
                        hfpHandleChldThreeError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_CHLD_4_REQ:
        {    
            PRINT(("HFP_INTERNAL_AT_CHLD_4_REQ\n"));
            switch(profileState)
            {		
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                        /* Only allowed if we are an HFP device */
                        hfpHandleChldFour(hfp);
                    
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                case hfpSlcConnected:
                    {
                        /* Send an error message */
                        hfpHandleChldFourError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_CNUM_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CNUM_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp15(hfp->hfpSupportedProfile) && supportedProfileIsHfp15(hfp->agSupportedProfile))
                    {
                    	/* Only allowed if we are an HFP v1.5 device talking to an AG supporting HFP v1.5 */
                        hfpHandleSubscriberNumberGetReq(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {	
                        /* Send error response */
                        hfpHandleSubscriberNumberGetReqError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_BTRH_STATUS_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_BTRH_STATUS_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp15(hfp->hfpSupportedProfile) && supportedProfileIsHfp15(hfp->agSupportedProfile))
                    {
                    	/* Only allowed if we are an HFP v1.5 device talking to an AG supporting HFP v1.5 */
                        hfpHandleBtrhStatusReq(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {	
                        /* Send error response */
                        hfpHandleBtrhStatusReqError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_BTRH_HOLD_REQ:
        {    
            PRINT(("HFP_INTERNAL_AT_BTRH_HOLD_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp15(hfp->hfpSupportedProfile) && supportedProfileIsHfp15(hfp->agSupportedProfile))
                    {	
                        /* Only allowed if we are an HFP v1.5 device talking to an AG supporting HFP v1.5 */
                        hfpHandleBtrhHoldReq(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {	
                        /* Send error response */
                        hfpHandleBtrhHoldReqError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_BTRH_ACCEPT_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_BTRH_ACCEPT_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp15(hfp->hfpSupportedProfile) && supportedProfileIsHfp15(hfp->agSupportedProfile))
                    {	
                        /* Only allowed if we are an HFP v1.5 device talking to an AG supporting HFP v1.5 */
                        hfpHandleBtrhAcceptReq(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {	
                        /* Send error response */
                        hfpHandleBtrhAcceptReqError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_BTRH_REJECT_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_BTRH_STATUS_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp15(hfp->hfpSupportedProfile) && supportedProfileIsHfp15(hfp->agSupportedProfile))
                    {	
                        /* Only allowed if we are an HFP v1.5 device talking to an AG supporting HFP v1.5 */
                        hfpHandleBtrhRejectReq(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {	
                        /* Send error response */
                        hfpHandleBtrhRejectReqError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_CLCC_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CLCC_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp15(hfp->hfpSupportedProfile) && supportedProfileIsHfp15(hfp->agSupportedProfile))
                    {	
                        /* Only allowed if we are an HFP v1.5 device talking to an AG supporting HFP v1.5 */
                        hfpHandleCurrentCallsGetReq(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {	
                        /* Send error response */
                        hfpHandleCurrentCallsGetReqError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_COPS_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_COPS_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp15(hfp->hfpSupportedProfile) && supportedProfileIsHfp15(hfp->agSupportedProfile))
                    {	
                        /* Only allowed if we are an HFP v1.5 device talking to an AG supporting HFP v1.5 */
                        hfpHandleNetworkOperatorReq(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {	
                        /* Send error response */
                        hfpHandleNetworkOperatorReqError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AT_CMEE_REQ:
        {
            PRINT(("HFP_INTERNAL_AT_CMEE_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp15(hfp->hfpSupportedProfile) && supportedProfileIsHfp15(hfp->agSupportedProfile))
                    {	
                        /* Only allowed if we are an HFP v1.5 device talking to an AG supporting HFP v1.5 */
                        hfpHandleExtendedErrorReq(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {	
                        /* Send error response */
                        hfpHandleExtendedErrorReqError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AUDIO_TRANSFER_REQ:
        {
            PRINT(("HFP_INTERNAL_AUDIO_TRANSFER_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {	
                        /* Only allowed if we are an HFP device */
                        hfpHandleAudioTransferReq(hfp, (HFP_INTERNAL_AUDIO_TRANSFER_REQ_T *) message);
                    }
                    else if (supportedProfileIsHsp(hfp->hfpSupportedProfile))
                    {   
                        /* In HSP audio transfer performed using button press */
                        hfpHandleHsButtonPress(hfp);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                    {	
                        /* Send an error message */
                        hfpHandleAudioTransferReqError(hfp, (HFP_INTERNAL_AUDIO_TRANSFER_REQ_T *) message);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AUDIO_CONNECT_REQ:
        {
            PRINT(("HFP_INTERNAL_AUDIO_CONNECT_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnected:		
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {	
                        /* Only allowed if we are an HFP device */
                        hfpHandleAudioConnectReq(hfp, (HFP_INTERNAL_AUDIO_CONNECT_REQ_T *) message);
                    }
                    else
                case hfpReady:
                case hfpSlcConnecting:
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                    {	
                        /* Send an error message */
                        hfpHandleAudioConnectReqError(hfp, (HFP_INTERNAL_AUDIO_CONNECT_REQ_T *) message);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AUDIO_CONNECT_RES:
        {
            PRINT(("HFP_INTERNAL_AUDIO_CONNECT_RES\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                case hfpSlcConnected:		
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    hfpHandleAudioConnectRes(hfp, (HFP_INTERNAL_AUDIO_CONNECT_RES_T *) message);			
                    break;

                case hfpReady:
                    hfpHandleAudioConnectResError(hfp, (HFP_INTERNAL_AUDIO_CONNECT_RES_T *) message);			
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_AUDIO_DISCONNECT_REQ:
        {
            PRINT(("HFP_INTERNAL_AUDIO_DISCONNECT_REQ\n"));
            switch(profileState)
            {
                case hfpSlcConnecting:
                case hfpSlcConnected:		
                case hfpIncomingCallEstablish:
                case hfpOutgoingCallEstablish:
                case hfpOutgoingCallAlerting:
                case hfpActiveCall:
                    if (supportedProfileIsHfp(hfp->hfpSupportedProfile))
                    {
                    	/* Only allowed if we are an HFP device */
                        hfpHandleAudioDisconnectReq(hfp);
                    }
                    else
                case hfpReady:
                    {
                    	/* Send an error message */
                        hfpHandleAudioDisconnectReqError(hfp);
                    }
                    break;

                default:
    	            handleUnexpected(hfpUnexpectedHfpPrim, profileState, id);
    	            break;
            }
        }
        return;

        case HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ:
        {
            PRINT(("HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ\n"));
            hfpHandleCsrSupportedFeaturesReq(hfp, (HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ_T*)message);
        }
        return;
        
        case HFP_INTERNAL_CSR_SUPPORTED_FEATURES_ACK:
        {
            PRINT(("HFP_INTERNAL_CSR_SUPPORTED_FEATURES_ACK\n"));
            hfpHandleCsrSupportedFeaturesAck(hfp, (HFP_INTERNAL_CSR_SUPPORTED_FEATURES_ACK_T*)message);
        }
        return;
        
        case HFP_INTERNAL_CSR_POWER_LEVEL_REQ:
        {
            PRINT(("HFP_INTERNAL_CSR_POWER_LEVEL_REQ\n"));
            hfpHandleCsrPowerLevelReq(hfp, (HFP_INTERNAL_CSR_POWER_LEVEL_REQ_T*)message);
        }
        return;
        
        case HFP_INTERNAL_CSR_POWER_SOURCE_REQ:
        {
            PRINT(("HFP_INTERNAL_CSR_POWER_SOURCE_REQ\n"));
            hfpHandleCsrPowerSourceReq(hfp, (HFP_INTERNAL_CSR_POWER_SOURCE_REQ_T*)message);
        }
        return;
        
        case HFP_INTERNAL_CSR_MOD_INDS_REQ:
        {
            PRINT(("HFP_INTERNAL_CSR_MOD_INDS_REQ\n"));
            hfpHandleCsrModIndsReq(hfp, (HFP_INTERNAL_CSR_MOD_INDS_REQ_T*)message);
        }
        return;
        
        case HFP_INTERNAL_CSR_MOD_INDS_DISABLE_REQ:
        {
            PRINT(("HFP_INTERNAL_CSR_MOD_INDS_DISABLE_REQ\n"));
            hfpCsrMofifyIndicatorsDisableReq(hfp);
        }
        return;
        
        case HFP_INTERNAL_CSR_GET_SMS_REQ:
        {
            PRINT(("HFP_INTERNAL_CSR_GET_SMS_REQ\n"));
            hfpHandleCsrGetSmsReq(hfp, (HFP_INTERNAL_CSR_GET_SMS_REQ_T*)message);
        }
        return;
        
        case HFP_INTERNAL_CSR_AG_BAT_REQ:
        {
            PRINT(("HFP_INTERNAL_CSR_AG_BAT_REQ\n"));
            hfpHandleInternalResponseCSRBatRequest(hfp);
        }
        return;	
        
        case HFP_INTERNAL_CSR_FEATURE_NEGOTIATION_RES:
        {
            PRINT(("HFP_INTERNAL_CSR_FEATURE_NEGOTIATION_RES\n")) ;
            hfpHandleFeatureNegotiationRes(hfp , (HFP_INTERNAL_CSR_FEATURE_NEGOTIATION_RES_T*)message) ;
        }
        return;
	}
	
	switch (id)
	{
	    case CL_RFCOMM_REGISTER_CFM:
	    {
		    PRINT(("CL_RFCOMM_REGISTER_CFM\n"));
    		switch(profileState)
	    	{		
		        case hfpInitialising:
			        hfpHandleRfcommRegisterCfm(hfp, (CL_RFCOMM_REGISTER_CFM_T *) message);
			        break;
		
		        default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
    		}
	    }
	    return;

        case CL_RFCOMM_CONNECT_IND:
        {
            PRINT(("CL_RFCOMM_CONNECT_IND\n"));        
   		    switch(profileState)
		    {		
		        case hfpReady:
			        /* Handle the connect indication */
			        hfpHandleRfcommConnectInd(hfp, (CL_RFCOMM_CONNECT_IND_T *) message);
			        break;

		        case hfpSlcConnecting:
		        case hfpSlcConnected:
		        case hfpIncomingCallEstablish:
		        case hfpOutgoingCallEstablish:
		        case hfpOutgoingCallAlerting:
		        case hfpActiveCall:
			        /* Reject the connect request - this shouldn't happen too often as we 
				       unregister our service record after we establish an SLC */
			        hfpHandleSlcConnectIndReject(hfp, (CL_RFCOMM_CONNECT_IND_T *) message);
			        break;

		        default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;

        case CL_RFCOMM_CONNECT_CFM:
        {
            PRINT(("CL_RFCOMM_CONNECT_CFM\n"));
            switch(profileState)
	    	{
                case hfpSlcConnecting:
			        /* Handle the connect cfm */
                    hfpHandleRfcommConnectCfm(hfp, (CL_RFCOMM_CONNECT_CFM_T *) message);
                    break;

		        default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;

	    case CL_RFCOMM_DISCONNECT_IND:
		{
		    PRINT(("CL_RFCOMM_DISCONNECT_IND\n"));
            switch(profileState)
	    	{
    	    	case hfpReady:
                case hfpSlcConnecting:
		        case hfpSlcConnected:
		        case hfpIncomingCallEstablish:
		        case hfpOutgoingCallEstablish:
		        case hfpOutgoingCallAlerting:
		        case hfpActiveCall:
			        hfpHandleRfcommDisconnectInd(hfp, (CL_RFCOMM_DISCONNECT_IND_T *) message);
			        break;

		        default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;

	    case CL_RFCOMM_CONTROL_IND:
	    {
		    PRINT(("CL_RFCOMM_CONTROL_IND\n"));
            switch(profileState)
	    	{
                case hfpSlcConnecting:
        		case hfpSlcConnected:
        		case hfpIncomingCallEstablish:
        		case hfpOutgoingCallEstablish:
        		case hfpOutgoingCallAlerting:
        		case hfpActiveCall:
			        hfpHandleRfcommControlInd(hfp, (CL_RFCOMM_CONTROL_IND_T *) message);
			        break;

		        default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;

	    case CL_SDP_REGISTER_CFM:
	    {
		    PRINT(("CL_SDP_REGISTER_CFM\n"));
    		switch(profileState)
	    	{		
        		case hfpInitialising:
        		case hfpReady:
        		case hfpSlcConnecting:
			        /* Handle the register cfm */
			        hfpHandleSdpRegisterCfm(hfp, (CL_SDP_REGISTER_CFM_T *) message);
			        break;
		
		        default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;

	    case CL_SDP_UNREGISTER_CFM:
	    {
		    PRINT(("CL_SDP_UNREGISTER_CFM\n"));
    		switch(profileState)
	    	{		
    		    case hfpReady:
		        case hfpSlcConnected:
	            case hfpIncomingCallEstablish:
		        case hfpOutgoingCallEstablish:
		        case hfpOutgoingCallAlerting:
		        case hfpActiveCall:
			        /* Handle the unregister cfm */
			        handleSdpUnregisterCfm(hfp, (CL_SDP_UNREGISTER_CFM_T *) message);
			        break;

		        default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;

        case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
        {
		    PRINT(("CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM\n"));
    		switch(profileState)
		    {		
		        case hfpSlcConnecting:
		        case hfpSlcConnected:		
		        case hfpIncomingCallEstablish:
		        case hfpOutgoingCallEstablish:
		        case hfpOutgoingCallAlerting:
		        case hfpActiveCall:
			        /* Currently we only look for attributes during SLC establishment and connection */
			        hfpHandleServiceSearchAttributeCfm(hfp, (CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *) message);
			        break;

		        case hfpReady:
			        break;

                default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;

	    case CL_DM_SYNC_REGISTER_CFM:
	    {
		    PRINT(("CL_DM_SYNC_REGISTER_CFM\n"));
    		switch(profileState)
		    {
		        case hfpInitialising:		
		        case hfpReady:
		        case hfpSlcConnecting:
		        case hfpSlcConnected:
		        case hfpIncomingCallEstablish:
		        case hfpOutgoingCallEstablish:
		        case hfpOutgoingCallAlerting:
		        case hfpActiveCall:
			        /* Registered for Synchronous connection indications */
			        hfpHandleSyncRegisterCfm(hfp);
			        break;
		
                default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;
	    
        case CL_DM_SYNC_CONNECT_CFM:
        {
		    PRINT(("CL_DM_SYNC_CONNECT_CFM\n"));
    		switch(profileState)
		    {
		        case hfpReady:          /* Could be in these states if SLC had been disconnected while */
		        case hfpSlcConnecting:  /* creating Synchronous connection.                            */
		        case hfpSlcConnected:
		        case hfpIncomingCallEstablish:
		        case hfpOutgoingCallEstablish:
		        case hfpOutgoingCallAlerting:
		        case hfpActiveCall:
			        /* Audio connected */
			        hfpHandleSyncConnectCfm(hfp, (CL_DM_SYNC_CONNECT_CFM_T *) message);
			        break;
		
                default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case CL_DM_SYNC_CONNECT_IND:
        {
		    PRINT(("CL_DM_SYNC_CONNECT_IND\n"));
		    switch(profileState)
		    {
                case hfpSlcConnecting:
		        case hfpSlcConnected:
        		case hfpIncomingCallEstablish:
        		case hfpOutgoingCallEstablish:
        		case hfpOutgoingCallAlerting:
        		case hfpActiveCall:
			        /* Incomming audio connect request */
			        hfpHandleSyncConnectInd(hfp, (CL_DM_SYNC_CONNECT_IND_T *) message);
			        break;
		
		        case hfpInitialising:		
                case hfpReady:
                    /* Reject outright, instance is in the wrong state */
			        hfpHandleSyncConnectIndReject(hfp, (CL_DM_SYNC_CONNECT_IND_T *) message);
                    break;
		
                default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;
        
        case CL_DM_SYNC_DISCONNECT_IND:
        {
		    PRINT(("CL_DM_SYNC_DISCONNECT_IND\n"));
		    switch(profileState)
		    {
		        case hfpReady:
		        case hfpSlcConnecting:
		        case hfpSlcConnected:
		        case hfpIncomingCallEstablish:
		        case hfpOutgoingCallEstablish:
		        case hfpOutgoingCallAlerting:
		        case hfpActiveCall:
			        /* Audio disconnected */
			        hfpHandleSyncDisconnectInd(hfp, (CL_DM_SYNC_DISCONNECT_IND_T *) message);
			        break;
		
                default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;

    	case CL_SM_ENCRYPTION_CHANGE_IND:
    	{
		    PRINT(("CL_SM_ENCRYPTION_CHANGE_IND\n"));
		    switch(profileState)
		    {		
		        case hfpSlcConnecting:		
		        case hfpSlcConnected:
		        case hfpIncomingCallEstablish:
		        case hfpOutgoingCallEstablish:
		        case hfpOutgoingCallAlerting:
		        case hfpActiveCall:
			        /* We have received an indication that the encryption status of the sink has changed */
			        hfpHandleEncryptionChangeInd(hfp, (CL_SM_ENCRYPTION_CHANGE_IND_T *) message);
			        break;

                default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;
		
	    case CL_SM_ENCRYPTION_KEY_REFRESH_IND:
	    {
		    PRINT(("CL_SM_ENCRYPTION_KEY_REFRESH_IND\n"));
    		switch(profileState)
		    {		
		        case hfpSlcConnecting:		
		        case hfpSlcConnected:
		        case hfpIncomingCallEstablish:
		        case hfpOutgoingCallEstablish:
		        case hfpOutgoingCallAlerting:
		        case hfpActiveCall:
			        /* We have received an indication that the encryption key of the sink has been refreshed */
			        hfpHandleEncryptionKeyRefreshInd(hfp, (CL_SM_ENCRYPTION_KEY_REFRESH_IND_T *) message);
			        break;

                default:
                    handleUnexpected(hfpUnexpectedClPrim, profileState, id);
    	            break;
            }
        }
        return;

	    case CL_DM_ROLE_CFM:
		    return;
    }
    
    switch (id)
    {        
	    case MESSAGE_MORE_DATA:
	    {
		    PRINT(("MESSAGE_MORE_DATA\n"));
    		switch(profileState)
	    	{		
        		case hfpSlcConnecting:		
        		case hfpSlcConnected:
        		case hfpIncomingCallEstablish:
        		case hfpOutgoingCallEstablish:
        		case hfpOutgoingCallAlerting:
        		case hfpActiveCall:
			        /* We have received more data into the RFCOMM buffer */
			        hfpHandleReceivedData(hfp, ((MessageMoreData *) message)->source);
			        break;

        		default:
			        handleUnexpected(hfpUnexpectedMessage, profileState, id);
			        break;
		    }
		}
		return;
		
    	/* Ignored primitives */
    	case MESSAGE_MORE_SPACE:
        case MESSAGE_SOURCE_EMPTY:
            return;

    	default:
            handleUnexpected(hfpUnexpectedMessage, profileState, id);
			break;
	}
}

/*lint +e525 +e725 +e830 */
