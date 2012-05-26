/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_voice_handler.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_indicators_handler.h"
#include "hfp_parse.h"
#include "hfp_send_data.h"
#include "hfp_voice_handler.h"

#include <panic.h>
#include <string.h>


static void sendVoiceRecognitionCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_VOICE_RECOGNITION_ENABLE_CFM, hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleVoiceRecognitionEnable

DESCRIPTION
	Enable/ disable voice dialling at the AG.

RETURNS
	void
*/
void hfpHandleVoiceRecognitionEnable(HFP *hfp, const HFP_INTERNAL_AT_BVRA_REQ_T *req)
{
	/* Only send the cmd if the AG and local device support the voice dial feature */
	if ((hfp->hfpSupportedFeatures & HFP_VOICE_RECOGNITION) &&
            (hfp->agSupportedFeatures & AG_VOICE_RECOGNITION))
	{
		char *bvra;

		if (req->enable)
			bvra = "AT+BVRA=1\r";
		else
			bvra = "AT+BVRA=0\r";

		/* Send the AT cmd over the air */
		hfpSendAtCmd(&hfp->task, strlen(bvra), bvra);
	}
	else
	{
		/* Send a cfm to the app.to tell it this feayure is not supported */
		sendVoiceRecognitionCfmToApp(hfp, hfp_fail);
	}
}


/****************************************************************************
NAME	
	hfpHandleVoiceRecognitionEnableError

DESCRIPTION
	Send error message to the app, the request has failed.

RETURNS
	void
*/
void hfpHandleVoiceRecognitionEnableError(HFP *hfp)
{
	/* Send an error message */
	sendVoiceRecognitionCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleBvraAtAck

DESCRIPTION
	Tell the app whether the AG accepted or rejected the AT+BVRA cmd.

RETURNS
	void
*/
void hfpHandleBvraAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm to the app. */
	sendVoiceRecognitionCfmToApp(hfp, status);

	/* This is necessary when dealing with 0.96 AGs */
	if (!hfp->indicator_status.indexes.call_setup && (status == hfp_success) &&
		supportedProfileIsHfp(hfp->hfpSupportedProfile))
    {
		hfpSendIndicatorCallSetupToApp(hfp, hfp_outgoing_call_setup);
    }
}


/****************************************************************************
NAME	
	hfpHandleVoiceRecognitionStatus

DESCRIPTION
	Voice recognition status indication received from the AG.

AT INDICATION
	+BVRA

RETURNS
	void
*/
void hfpHandleVoiceRecognitionStatus(Task profileTask, const struct hfpHandleVoiceRecognitionStatus *ind)
{
	HFP *hfp = (HFP *) profileTask;

	/* 
		Send a message to the application telling it the current status of the 
		voice recognition engine at the AG. 
	*/
	MAKE_HFP_MESSAGE(HFP_VOICE_RECOGNITION_IND);
	message->enable = ind->enable;
	message->hfp = hfp;
	MessageSend(hfp->clientTask, HFP_VOICE_RECOGNITION_IND, message);

	/* This is necessary when dealing with 0.96 AGs */
	if (!hfp->indicator_status.indexes.call_setup && 
        supportedProfileIsHfp(hfp->hfpSupportedProfile))
	{
		if (!ind->enable && !hfp->audio_sink)
			hfpSendIndicatorCallSetupToApp(hfp, hfp_no_call_setup);
	}
}
