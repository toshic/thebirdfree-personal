/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_caller_id_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_CALLER_ID_HANDLER_H_
#define HFP_CALLER_ID_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleCallerIdEnableReq

DESCRIPTION
	Attempt to enable the caller id functionality at the AG.

RETURNS
	void
*/
void hfpHandleCallerIdEnableReq(HFP *hfp, const HFP_INTERNAL_AT_CLIP_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleCallerIdEnableReqError

DESCRIPTION
	Caller id at the AG cnnot be enabled because the profile instance is in 
	the wrong state.

RETURNS
	void
*/
void hfpHandleCallerIdEnableReqError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleClipAtAck

DESCRIPTION
	Caller id enable/disable command has been acknowledged by the AG.

RETURNS
	void
*/
void hfpHandleClipAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_CALLER_ID_HANDLER_H_ */
