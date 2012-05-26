/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_dial_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_DIAL_HANDLER_H_
#define HFP_DIAL_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleLastNumberRedial

DESCRIPTION
	Issue a last number redial request.

RETURNS
	void
*/
void hfpHandleLastNumberRedial(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleLastNumberRedialError

DESCRIPTION
	The last number edial request received from the app has arrive while the
	profile instance is not in the correct state so send back an error
	response to the app.

RETURNS
	void
*/
void hfpHandleLastNumberRedialError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleBldnAtAck

DESCRIPTION
	Ack received for AT+BLDN AT cmd.

RETURNS
	void
*/
void hfpHandleBldnAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleDialNumberRequest

DESCRIPTION
	Issue a request to dial the supplied number.

RETURNS
	void
*/
void hfpHandleDialNumberRequest(HFP *hfp, const HFP_INTERNAL_AT_ATD_NUMBER_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleDialNumberRequestError

DESCRIPTION
	received a dial number request while the profile instance was in the
	wrong state so send back an error message indicating this.

RETURNS
	void
*/
void hfpHandleDialNumberRequestError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleAtdNumberAtAck

DESCRIPTION
	Received an ack for the ATD cmd.

RETURNS
	void
*/
void hfpHandleAtdNumberAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleDialMemoryRequest

DESCRIPTION
	Send a dial memory location request to the AG.

RETURNS
	void
*/
void hfpHandleDialMemoryRequest(HFP *hfp, const HFP_INTERNAL_AT_ATD_MEMORY_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleDialMemoryRequestError

DESCRIPTION
	Received a dial memory request while the profile instance was in the
	wrong state so send back an error message indicating this.

RETURNS
	void
*/
void hfpHandleDialMemoryRequestError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleAtdMemoryAtAck

DESCRIPTION
	Received an ack for the ATD> cmd.

RETURNS
	void
*/
void hfpHandleAtdMemoryAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_DIAL_HANDLER_H_ */
