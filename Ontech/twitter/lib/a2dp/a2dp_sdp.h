/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_sdp.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_SDP_H_
#define A2DP_SDP_H_

#include "a2dp.h"


/****************************************************************************
NAME
	a2dpHandleSdpRegisterCfm

DESCRIPTION
    Handle a CL_SDP_REGISTER_CFM message from the connection lib indicating
    the outcome of the service register request.
*/
void a2dpHandleSdpRegisterCfm(A2DP *a2dp, const CL_SDP_REGISTER_CFM_T *cfm);


#endif /* A2DP_SDP_H_ */
