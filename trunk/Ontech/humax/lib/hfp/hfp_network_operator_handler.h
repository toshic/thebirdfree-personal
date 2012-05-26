/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    hfp_network_operator_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_NETWORK_OPERATOR_HANDLER_H_
#define HFP_NETWORK_OPERATOR_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleNetworkOperatorReq

DESCRIPTION
	Request network operator information from the AG.

RETURNS
	void
*/
void hfpHandleNetworkOperatorReq(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleNetworkOperatorReqError

DESCRIPTION
	Network operator information can't be obtained from the AG because the 
    profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleNetworkOperatorReqError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleCopsFormatAtAck

DESCRIPTION
	Network operator reporting format command has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleCopsFormatAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleCopsReqAtAck

DESCRIPTION
	Network operator information request command has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleCopsReqAtAck(HFP *hfp, hfp_lib_status status);


#endif /* HFP_NETWORK_OPERATOR_HANDLER_H_ */
