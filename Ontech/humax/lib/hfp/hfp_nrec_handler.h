/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_nrec_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_NREC_HANDLER_H_
#define HFP_NREC_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleNrEcDisable

DESCRIPTION
	Send a request to the AG to disable its Noise Reduction (NR) and Echo
	Cancellation (EC) capabilities.

RETURNS
	void
*/
void hfpHandleNrEcDisable(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleNrEcDisableError

DESCRIPTION
	Send an error message to the application, the profile instance is in
	the wrong state.

RETURNS
	void
*/
void hfpHandleNrEcDisableError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleNrecAtAck

DESCRIPTION
	Received an ack from the AG for the AT+NREC cmd.

RETURNS
	void
*/
void hfpHandleNrecAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_NREC_HANDLER_H_ */
