/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    hfp_current_calls_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_CURRENT_CALLS_HANDLER_H_
#define HFP_CURRENT_CALLS_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleCurrentCallsGetReq

DESCRIPTION
	Request current call list from the AG.

RETURNS
	void
*/
void hfpHandleCurrentCallsGetReq(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleCurrentCallsGetReqError

DESCRIPTION
	Current call list can't be obtained from the AG because the 
    profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleCurrentCallsGetReqError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleClccAtAck

DESCRIPTION
	Current call list request command has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleClccAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_CURRENT_CALLS_HANDLER_H_ */
