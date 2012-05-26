/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_dtmf_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_DTMF_HANDLER_H_
#define HFP_DTMF_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleDtmfRequest

DESCRIPTION
	HAndle a request to send a DTMF tone to the AG.

RETURNS
	void
*/
void hfpHandleDtmfRequest(HFP *hfp, const HFP_INTERNAL_AT_VTS_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleDtmfRequestError

DESCRIPTION
	Send an error to the application as the profile instance is in the 
	wrong state.

RETURNS
	void
*/
void hfpHandleDtmfRequestError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleVtsAtAck

DESCRIPTION
	Have received an ack from the AG for the AT+VTS that we sent.

RETURNS
	void
*/
void hfpHandleVtsAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_DTMF_HANDLER_H_ */
