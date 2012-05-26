/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_hs_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_HS_HANDLER_H_
#define HFP_HS_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleHsButtonPress

DESCRIPTION
	Send a button press AT cmd to the AG.

RETURNS
	void
*/
void hfpHandleHsButtonPress(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleHsButtonPressError

DESCRIPTION
	The HFP library received a request to send a button press to the AG,
	but this profile instance is in the wrong state, so send back an error
	message to the app instead.

RETURNS
	void
*/
void hfpHandleHsButtonPressError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleHsCkpdAtAck

DESCRIPTION
	Send a button press cfm to the app telling it whether the AT+CKPD
	succeeded or not.

RETURNS
	void
*/
void hfpHandleHsCkpdAtAck(HFP *hfp, hfp_lib_status status);



#endif /* HFP_HS_HANDLER_H_ */
