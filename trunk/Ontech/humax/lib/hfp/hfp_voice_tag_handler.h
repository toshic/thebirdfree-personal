/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    hfp_voice_tag_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_VOICE_TAG_HANDLER_H_
#define HFP_VOICE_TAG_HANDLER_H_



/****************************************************************************
NAME	
	hfpHandleGetVoiceTagReq

DESCRIPTION
	Issue an "attach number to a voice tag" request to the AG to retrieve
	a phone number.

RETURNS
	void
*/
void hfpHandleGetVoiceTagReq(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleGetVoiceTagReq

DESCRIPTION
	Issue an "attach number to a voice tag" request to the AG to retrieve
	a phone number.

RETURNS
	void
*/
void hfpHandleGetVoiceTagReqError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleBinpAtAck

DESCRIPTION
	Received an ack from the app in response to the AT+BINP cmd.

RETURNS
	void
*/
void hfpHandleBinpAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_VOICE_TAG_HANDLER_H_ */
