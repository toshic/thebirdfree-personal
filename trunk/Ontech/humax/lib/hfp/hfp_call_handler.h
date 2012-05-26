/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_call_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_CALL_HANDLER_H_
#define HFP_CALL_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleAnswerCall

DESCRIPTION
	Answer an incoming call.

RETURNS
	void
*/
void hfpHandleAnswerCall(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleAnswerCallError

DESCRIPTION
	Received an answer call request but the profile lib is in the wrong 
	state. Send back an error message.

RETURNS
	void
*/
void hfpHandleAnswerCallError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleRejectCall

DESCRIPTION
	Reject an incoming call.

RETURNS
	void
*/
void hfpHandleRejectCall(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleRejectCallError

DESCRIPTION
	Received a request to reject a call when the profile instance is in the 
	wrong state, so send back an error message.

RETURNS
	void
*/
void hfpHandleRejectCallError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleAtaAtAck

DESCRIPTION
	Tell the app whether the AG accepted or rejected the ATA cmd.

RETURNS
	void
*/
void hfpHandleAtaAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleChupAtAck

DESCRIPTION
	Tell the app whether the AG accepted or rejected the AT+CHUP cmd.

RETURNS
	void
*/
void hfpHandleChupAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleTerminateCall

DESCRIPTION
	Terminate an ongoing call process.

RETURNS
	void
*/
void hfpHandleTerminateCall(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleTerminateCallError

DESCRIPTION
	Send an error message because we received the terminate call request
	when the profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleTerminateCallError(HFP *hfp);


#endif /* HFP_CALL_HANDLER_H_ */
