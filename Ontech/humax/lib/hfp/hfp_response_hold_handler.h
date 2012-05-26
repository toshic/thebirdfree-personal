/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    hfp_response_hold_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_RESPONSE_HOLD_HANDLER_H_
#define HFP_RESPONSE_HOLD_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleBtrhStatusReq

DESCRIPTION
	Request response hold status from the AG.

RETURNS
	void
*/
void hfpHandleBtrhStatusReq(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleBtrhStatusReqError

DESCRIPTION
	Response hold status can't be obtained from the AG because the 
    profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleBtrhStatusReqError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleBtrhStatusAtAck

DESCRIPTION
	Response hold status request command has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleBtrhStatusAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleBtrHoldReq

DESCRIPTION
	Request to AG to hold incoming call.

RETURNS
	void
*/
void hfpHandleBtrhHoldReq(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleBtrhHoldReqError

DESCRIPTION
	Request to AG to hold incoming call can't be made because the profile 
    instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleBtrhHoldReqError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleBtrhHoldAtAck

DESCRIPTION
	Request to AG to hold incoming call has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleBtrhHoldAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleBtrAcceptReq

DESCRIPTION
	Request to AG to accept the currently held incoming call.

RETURNS
	void
*/
void hfpHandleBtrhAcceptReq(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleBtrhAcceptReqError

DESCRIPTION
	Request to AG to accept the currently held incoming call can't be made 
    because the profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleBtrhAcceptReqError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleBtrhAcceptAtAck

DESCRIPTION
	Request to AG to accept the currently held incoming call has been 
    acknowledged (completed) by the AG.

RETURNS
	void
*/
void hfpHandleBtrhAcceptAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleBtrRejectReq

DESCRIPTION
	Request to AG to reject the currently held incoming call.

RETURNS
	void
*/
void hfpHandleBtrhRejectReq(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleBtrhRejectReqError

DESCRIPTION
	Request to AG to reject the currently held incoming call can't be made 
    because the profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleBtrhRejectReqError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleBtrhRejectAtAck

DESCRIPTION
	Request to AG to reject the currently held incoming call has been 
    acknowledged (completed) by the AG.

RETURNS
	void
*/
void hfpHandleBtrhRejectAtAck(HFP *hfp, hfp_lib_status status);

#endif /* HFP_RESPONSE_HOLD_HANDLER_H_ */
