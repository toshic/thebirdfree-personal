/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_voice_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_VOICE_HANDLER_H_
#define HFP_VOICE_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleVoiceRecognitionEnable

DESCRIPTION
	Enable/ disable voice dialling at the AG.

RETURNS
	void
*/
void hfpHandleVoiceRecognitionEnable(HFP *hfp, const HFP_INTERNAL_AT_BVRA_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleVoiceRecognitionEnableError

DESCRIPTION
	Send error message to the app, the request has failed.

RETURNS
	void
*/
void hfpHandleVoiceRecognitionEnableError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleBvraAtAck

DESCRIPTION
	Tell the app whether the AG accepted or rejected the AT+BVRA cmd.

RETURNS
	void
*/
void hfpHandleBvraAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_VOICE_HANDLER_H_ */
