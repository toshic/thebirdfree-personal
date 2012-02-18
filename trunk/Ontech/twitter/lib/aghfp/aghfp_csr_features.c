/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1
*/

#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_send_data.h"

#include "aghfp_csr_features.h"

#include "aghfp_call_manager.h"

#include <panic.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <util.h>


void aghfpCsrSupportedFeaturesResponse (AGHFP *aghfp, bool callerName, bool rawText, bool smsInd, bool battLevel, bool pwrSource , uint16 codecs)
{
    /*send AT command to HF*/
    
    char buf[4];
	
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CSRSF: ");
	
	
    sprintf(buf, "%d", callerName);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
	
    sprintf(buf, "%d", rawText);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
	
    sprintf(buf, "%d", smsInd);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
	
    sprintf(buf, "%d", battLevel);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
	
    sprintf(buf, "%d", pwrSource);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
	
    sprintf(buf, "%d", codecs);
	aghfpAtCmdString(aghfp, buf);
	
	aghfpAtCmdEnd(aghfp);
    
}


void aghfpFeatureNegotiate ( AGHFP * aghfp , uint16 indicator , uint16 value )
{
    char buf[4];
	
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+CSRFN: ");
	
	
    sprintf(buf, "%d", indicator);
	aghfpAtCmdString(aghfp, buf);
	aghfpAtCmdString(aghfp, ",");
	
    sprintf(buf, "%d", value);
	aghfpAtCmdString(aghfp, buf);
	
	aghfpAtCmdEnd(aghfp);
}


void aghfpHandleFeatureNegotiation(Task task, const struct aghfpHandleFeatureNegotiation * feature )
{
    AGHFP * aghfp = (AGHFP *)task ;
    
    MAKE_AGHFP_MESSAGE(AGHFP_CSR_FEATURE_NEGOTIATION_IND) ;
    
    message->aghfp = aghfp ;
    message->indicator = feature->ind ;
    message->value = feature->val ;
    
    MessageSend ( aghfp->client_task , AGHFP_CSR_FEATURE_NEGOTIATION_IND , message ) ;
}


void aghfpHandleReponseCSRSupportedFeatures(Task task , const struct aghfpHandleReponseCSRSupportedFeatures * features )
{
    AGHFP * aghfp = (AGHFP *)task ;
        
    MAKE_AGHFP_MESSAGE(AGHFP_CSR_SUPPORTED_FEATURES_IND);
    
    message->aghfp      = aghfp ;
    message->callerName = features->callerName;
	message->rawText    = features->rawText;
	message->smsInd     = features->smsInd;
	message->battLevel  = features->battLevel;
	message->pwrSource  = features->pwrSource;
	message->codecs     = features->codecs;    
    
    MessageSend ( aghfp->client_task , AGHFP_CSR_SUPPORTED_FEATURES_IND , message );
}

void aghfpSendCsrFeatureNegotiationReqInd(AGHFP * aghfp)
{
    MAKE_AGHFP_MESSAGE(AGHFP_CSR_FEATURE_NEGOTIATION_REQ_IND);
    
    message->aghfp = aghfp ;

	/* Prompt the app to negotiate the codec */
	MessageSend(aghfp->client_task, AGHFP_CSR_FEATURE_NEGOTIATION_REQ_IND, message);
}

void AghfpClearAppCodecNegotiationPending(AGHFP * aghfp)
{
	aghfp->app_pending_codec_negotiation = FALSE;
}


void AghfpStartAudioAfterAppCodecNegotiation(AGHFP * aghfp)
{
	if (!aghfpCallManagerActiveNotComplete(aghfp))
	{
		MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AUDIO_CONNECT_REQ);
		AGHFP_DEBUG(("	  Normal Audio Connection\n"));
		
		/* Assume that the audio parameters have been previously stored using aghfpStoreAudioParams() */
		message->audio_params = aghfp->audio_params;
		message->packet_type = aghfp->audio_packet_type;
		
		MessageSend(&aghfp->task, AGHFP_INTERNAL_AUDIO_CONNECT_REQ, message);
	}
	else
	{
		/* Answer the call again now that the WBS negotiation is complete. */
		aghfpManageCall(aghfp, CallEventAnswer, CallFlagOpenAudio);
	}
}

