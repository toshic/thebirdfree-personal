/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_sound.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"

#include <panic.h>


/****************************************************************************
NAME	
	HfpSendSpeakerVolume

DESCRIPTION
	Request to send the local speaker setting (volume) to the AG. The HFP
	specification limits the value of volume to be in the range 0-15 and
	the Hfp profile library will enforce this. The request is issued on the 
	SLC associated with the hfp profile instance passed in by the application. 
	The message returned indicates whether the command was recognised by the 
	AG or not. 

	The AG may autonomously send volume gain indications to the HFP device,
	the application will be notified of these using the HFP_SPEAKER_VOLUME_IND
	message.

MESSAGE RETURNED
	HFP_SPEAKER_VOLUME_CFM

RETURNS
	void
*/
void HfpSendSpeakerVolume(HFP *hfp, uint16 volume)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));

	if (volume > 15)
		HFP_DEBUG(("Invalid volume gain passed in 0x%x\n", volume));
#endif

	{
		/* Create and send an internal message */
		MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_VGS_REQ);
		message->volume_gain = volume;
		MessageSend(&hfp->task, HFP_INTERNAL_AT_VGS_REQ, message);
	}
}


/****************************************************************************
NAME	
	HfpSendMicrophoneVolume

DESCRIPTION
	Request to send the local microphone setting (volume) to the AG. The HFP
	specification limits the value of volume to be in the range 0-15 and
	the Hfp profile library will enforce this. The request is issued on the 
	SLC associated with the hfp profile instance passed in by the application. 
	The message returned indicates whether the command was recognised by the 
	AG or not.

	The AG may autonomously send microphone gain indications to the HFP device.
	The application will be notified of these using the 
	HFP_MICROPHONE_VOLUME_IND message.

MESSAGE RETURNED
	HFP_MICROPHONE_VOLUME_CFM

RETURNS
	void
*/
void HfpSendMicrophoneVolume(HFP *hfp, uint16 volume)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));

	if (volume > 15)
		HFP_DEBUG(("Invalid volume gain passed in 0x%x\n", volume));
#endif

	{
		/* Create and send an internal message */
		MAKE_HFP_MESSAGE(HFP_INTERNAL_AT_VGM_REQ);
		message->mic_gain = volume;
		MessageSend(&hfp->task, HFP_INTERNAL_AT_VGM_REQ, message);
	}
}
