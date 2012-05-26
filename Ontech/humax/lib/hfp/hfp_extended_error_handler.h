/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    hfp_extended_error_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_EXTENDED_ERROR_HANDLER_H_
#define HFP_EXTENDED_ERROR_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleExtendedErrorReq

DESCRIPTION
	Request extended error information from the AG.

RETURNS
	void
*/
void hfpHandleExtendedErrorReq(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleExtendedErrorReqError

DESCRIPTION
	Extended error information can't be obtained from the AG because the 
    profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleExtendedErrorReqError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleCmeeAtAck

DESCRIPTION
	Extended error information request command has been acknowledged by the AG.

RETURNS
	void
*/
void hfpHandleCmeeAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_EXTENDED_ERROR_HANDLER_H_ */
