/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_sound_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_parse.h"
#include "hfp_send_data.h"
#include "hfp_sound_handler.h"

#include <panic.h>
#include <stdio.h>
#include <string.h>


/* Send a cfm message to the app. */
static void sendVgsCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_SPEAKER_VOLUME_CFM, hfp, status);
}

/* Send a cfm message to the app. */
static void sendVgmCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_MICROPHONE_VOLUME_CFM, hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleVgsRequest

DESCRIPTION
	Send a volume gain update to the AG.

RETURNS
	void
*/
void hfpHandleVgsRequest(HFP *hfp, const HFP_INTERNAL_AT_VGS_REQ_T *req)
{
	uint16 old_vol = hfp->vol_setting;
		
	/* Store the latest volume setting requested by the client */
	hfp->vol_setting = (req->volume_gain & 0xf);

	if (old_vol == 0xff)
	{
		char *vgs = "AT+VGS=00\r";

		/* Send the AT cmd over the air */
		hfpSendAtCmd(&hfp->task, strlen(vgs), vgs);
	}
}


/****************************************************************************
NAME	
	hfpHandleVgsRequestError

DESCRIPTION
	Received a request to send a VGS cmd but we're in the wrong state
	for the profile so send back an error immediately.

RETURNS
	void
*/
void hfpHandleVgsRequestError(HFP *hfp)
{
	/* Send back cfm with status set to fail */
	sendVgsCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleVgsAtAck

DESCRIPTION
	Received an ack for the AT+VGS cmd. 

RETURNS
	void
*/
void hfpHandleVgsAtAck(HFP *hfp, hfp_lib_status status)
{	
	/* Pass the confirmation on to the app */
	sendVgsCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	sendSpeakerVolumeIndToApp

DESCRIPTION
	We're in the right state to inform the app that the volume gain has
	been changed from the AG.

RETURNS
	void
*/
static void sendSpeakerVolumeIndToApp(HFP *hfp, uint16 gain)
{
	/* This is generated in response to receiving a volume indication from the AG */
	MAKE_HFP_MESSAGE(HFP_SPEAKER_VOLUME_IND);
	message->hfp = hfp;
	message->volume_gain = gain;
	MessageSend(hfp->clientTask, HFP_SPEAKER_VOLUME_IND, message);
}


/****************************************************************************
NAME	
	hfpHandleSpeakerGain

DESCRIPTION
	Called from the auto generated parser when a +VGS indication is
	received from the AG.

AT INDICATION
	+VGS

RETURNS
	void
*/
void hfpHandleSpeakerGain(Task profileTask, const struct hfpHandleSpeakerGain *ind)
{
	HFP *hfp = (HFP *) profileTask;

	/* 
		Don't pass the message to the app if the AG has sent us an out of range value 
		or the headset app has not specified we support remote volume control.	
	*/
	if ((ind->gain <= 15))
	{
		if ((supportedProfileIsHfp(hfp->hfpSupportedProfile) && 
            (hfp->hfpSupportedFeatures & HFP_REMOTE_VOL_CONTROL)) ||
			supportedProfileIsHsp(hfp->hfpSupportedProfile))
		{
			/* Send an internal message so we can go through the state machine */
			switch(hfp->state)
			{
			case hfpSlcConnected:
			case hfpIncomingCallEstablish:
			case hfpOutgoingCallEstablish:
			case hfpOutgoingCallAlerting:
				/* Only allowed if we are an HFP device */
				checkHfpProfile(hfp->hfpSupportedProfile);
				sendSpeakerVolumeIndToApp(hfp, ind->gain);
				break;

			case hfpActiveCall:
				/* Allowed for both HSP and HFP */
				sendSpeakerVolumeIndToApp(hfp, ind->gain);
				break;

			case hfpSlcConnecting:
				/* Ignore the message, the AG should not be sending it in this state */
				break;

			case hfpInitialising:
			case hfpReady:
			default:
				/* Panic in debug and ignore in release lib variants */
				HFP_DEBUG(("Received AT VGS under invalid state: 0x%x \n", hfp->state));
				break;
			}
		}		
	}
}


/****************************************************************************
NAME	
	hfpHandleVgsInd

DESCRIPTION
	Send the VGM request to the AG.

RETURNS
	void
*/
void hfpHandleVgmRequest(HFP *hfp, const HFP_INTERNAL_AT_VGM_REQ_T *req)
{
	char vgm[15];

	/* Create the AT cmd we're sending */
	strcpy(vgm, "AT+VGM=00\r");
	vgm[7] = '0' + (req->mic_gain & 0xf) / 10;
	vgm[8] = '0' + (req->mic_gain & 0xf) % 10;			
	
	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(vgm), vgm);
}


/****************************************************************************
NAME	
	hfpHandleVgmRequestError

DESCRIPTION
	Received a request to send a VGM cmd but we're in the wrong state
	for the profile so send back an error immediately.

RETURNS
	void
*/
void hfpHandleVgmRequestError(HFP *hfp)
{
	/* Send back cfm with status set to fail */
	sendVgmCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleVgmAtAck

DESCRIPTION
	Received an ack from the AG for the AT+VGM cmd we sent out.

RETURNS
	void
*/
void hfpHandleVgmAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm to the app */
	sendVgmCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	sendMicVolumeIndToApp

DESCRIPTION
	Pass on the received microphone gain indication to the app.

RETURNS
	void
*/
static void sendMicVolumeIndToApp(HFP *hfp, uint16 mic_gain)
{	
	/* This is generated in response to receiving a mic indication from the AG */
	MAKE_HFP_MESSAGE(HFP_MICROPHONE_VOLUME_IND);
	message->hfp = hfp;
	message->mic_gain = mic_gain;
	MessageSend(hfp->clientTask, HFP_MICROPHONE_VOLUME_IND, message);
}


/****************************************************************************
NAME	
	hfpHandleMicrophoneGain

DESCRIPTION
	Received a +VGM indication from the AG.

AT INDICATION
	+VGM

RETURNS
	void
*/
void hfpHandleMicrophoneGain(Task profileTask, const struct hfpHandleMicrophoneGain *ind)
{
	/* Don't pass the message to the app if the AG has sent us an out of range value */
	if (ind->gain <= 15)
	{
		/* Send an internal message so we can go through the state machine */
		HFP *hfp = (HFP *) profileTask;

		switch(hfp->state)
		{
		case hfpSlcConnected:
		case hfpIncomingCallEstablish:
		case hfpOutgoingCallEstablish:
		case hfpOutgoingCallAlerting:
			/* Only allowed if we are an HFP device */
			checkHfpProfile(hfp->hfpSupportedProfile);
			sendMicVolumeIndToApp(hfp, ind->gain);
			break;

		case hfpActiveCall:
			/* Allowed for both HSP and HFP */
			sendMicVolumeIndToApp(hfp, ind->gain);
			break;

		case hfpSlcConnecting:
			/* Ignore the message, the AG should not be sending it in this state */
			break;

		case hfpInitialising:
		case hfpReady:
		default:
			/* Panic in debug and ignore in release lib variants */
			HFP_DEBUG(("Received AT VGM under invalid state: 0x%x \n", hfp->state));
			break;
		}
	}
}

