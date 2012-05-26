/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    hfp_subscriber_num_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_SUBSCRIBER_NUM_HANDLER_H_
#define HFP_SUBSCRIBER_NUM_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleSubscriberNumberGetReq

DESCRIPTION
	Request subscriber number information from the AG.

RETURNS
	void
*/
void hfpHandleSubscriberNumberGetReq(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleSubscriberNumberGetReqError

DESCRIPTION
	Subscriber number information can't be obtained from the AG because the 
    profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleSubscriberNumberGetReqError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleCnumAtAck

DESCRIPTION
	Subscriber number information request command has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleCnumAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_SUBSCRIBER_NUM_HANDLER_H_ */
