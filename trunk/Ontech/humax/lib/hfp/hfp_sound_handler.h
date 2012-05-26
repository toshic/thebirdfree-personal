/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    hfp_sound_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_SOUND_HANDLER_H_
#define HFP_SOUND_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleVgsRequest

DESCRIPTION
	Send a volume gain update to the AG.

RETURNS
	void
*/
void hfpHandleVgsRequest(HFP *hfp, const HFP_INTERNAL_AT_VGS_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleVgsRequestError

DESCRIPTION
	Received a request to send a VGS cmd but we're in the wrong state
	for the profile so send back an error immediately.

RETURNS
	void
*/
void hfpHandleVgsRequestError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleVgsAtAck

DESCRIPTION
	Received an ack for the AT+VGS cmd. 

RETURNS
	void
*/
void hfpHandleVgsAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleVgmRequest

DESCRIPTION
	Send the VGM request to the AG.

RETURNS
	void
*/
void hfpHandleVgmRequest(HFP *hfp, const HFP_INTERNAL_AT_VGM_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleVgmRequestError

DESCRIPTION
	Received a request to send a VGM cmd but we're in the wrong state
	for the profile so send back an error immediately.

RETURNS
	void
*/
void hfpHandleVgmRequestError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleVgmAtAck

DESCRIPTION
	Received an ack from the AG for the AT+VGM cmd we sent out.

RETURNS
	void
*/
void hfpHandleVgmAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_SOUND_HANDLER_H_ */
