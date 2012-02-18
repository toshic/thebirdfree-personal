/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_l2cap_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_L2CAP_HANDLER_H_
#define A2DP_L2CAP_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME
	

DESCRIPTION
    Register AVDTP with L2CAP. 

*/
void a2dpRegisterL2cap(A2DP* a2dp);


/****************************************************************************
NAME
	a2dpHandleL2capRegisterCfm

DESCRIPTION
    Handle the CL_L2CAP_REGISTER_CFM message sent from the connection library
    as a result of calling the ConnectionL2capRegisterRequestLazy API.
*/
void a2dpHandleL2capRegisterCfm(A2DP *a2dp, const CL_L2CAP_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	a2dpHandleL2capConnectInd

DESCRIPTION
	This function is called on receipt of an CL_L2CAP_CONNECT_IND
	indicating that an L2CAP connection has been requested by a remote device.

*/
void a2dpHandleL2capConnectInd(A2DP *a2dp, const CL_L2CAP_CONNECT_IND_T *ind);


/****************************************************************************
NAME	
	a2dpHandleL2capConnectCfm

DESCRIPTION
	This function is called on receipt of an CL_L2CAP_CONNECT_CFM
	indicating that an L2CAP connection attempt has been completed.

*/
void a2dpHandleL2capConnectCfm(A2DP *a2dp, const CL_L2CAP_CONNECT_CFM_T *cfm);


/****************************************************************************
NAME	
	a2dpHandleL2capDisconnectInd
	

DESCRIPTION
	This function is called on receipt of an CL_L2CAP_DISCONNECT_IND
	indicating that a previously connected L2CAP connection has been
	disconnected.

*/
void a2dpHandleL2capDisconnectInd(A2DP* a2dp, const CL_L2CAP_DISCONNECT_IND_T* ind);


#endif /* A2DP_L2CAP_HANDLER_H_ */
