/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    l2cap_handler.h
    
DESCRIPTION

*/

#ifndef	CONNECTION_L2CAP_HANDLER_H_
#define	CONNECTION_L2CAP_HANDLER_H_

/* If we don't get an L2CAP_REGISTER_CFM by the time this expires, assume it failed */
#define L2CAP_REGISTER_TIMEOUT	(1000)


/****************************************************************************
NAME	
	connectionHandleL2capRegisterReq

DESCRIPTION
	Used by client tasks to register a PSM so remote devices can connect to 
	it. 

RETURNS
	void
*/
void connectionHandleL2capRegisterReq(connectionL2capState *l2capState, const CL_INTERNAL_L2CAP_REGISTER_REQ_T *req);


/****************************************************************************
NAME	
	connectionHandleL2capRegisterCfm

DESCRIPTION
	Confirmation that a PSM has been registered with the L2CAP layer of
	BlueStack.

RETURNS
	void
*/
void connectionHandleL2capRegisterCfm(connectionL2capState *l2capState, const L2CA_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	connectionHandleL2capUnregisterReq

DESCRIPTION
	Request to unregister a particular psm.

RETURNS
	void
*/
void connectionHandleL2capUnregisterReq(const CL_INTERNAL_L2CAP_UNREGISTER_REQ_T *req);


/****************************************************************************
NAME	
	connectionHandleL2capConnectReq

DESCRIPTION
	Request to initiate an L2CAP connection.

RETURNS
	void
*/
void connectionHandleL2capConnectReq(const CL_INTERNAL_L2CAP_CONNECT_REQ_T *req);


/****************************************************************************
NAME	
	connectionHandleL2capConnectCfm

DESCRIPTION
	Response to an L2CAP connect request

RETURNS
	void
*/
void connectionHandleL2capConnectCfm(const L2CA_CONNECT_CFM_T *cfm);


/****************************************************************************
NAME	
	connectionHandleL2capConfigCfm

DESCRIPTION
	Have received a confirm in response to a config request we sent out

RETURNS
	void
*/
void connectionHandleL2capConfigCfm(const L2CA_CONFIG_CFM_T *cfm);


/****************************************************************************
NAME	
	connectionHandleL2capConfigInd

DESCRIPTION
	Have received a config indication with channel params the remote end 
	wishes to use, need to respond either accepting or rejecting these.

RETURNS
	void
*/
void connectionHandleL2capConfigInd(L2CA_CONFIG_IND_T *ind);


/****************************************************************************
NAME	
	connectionHandleL2capConnectInd

DESCRIPTION
	Indication that the remove device is trying to connect to this device.

RETURNS
	void
*/
void connectionHandleL2capConnectInd(const L2CA_CONNECT_IND_T *ind);


/****************************************************************************
NAME	
	connectionHandleL2capConnectRes

DESCRIPTION
	Handle a response from the client task telling us whether to proceed
	with establishing the L2CAP connection. 

RETURNS
	void
*/
void connectionHandleL2capConnectRes(const CL_INTERNAL_L2CAP_CONNECT_RES_T *res);


/****************************************************************************
NAME	
	connectionHandleL2capDisconnectCfm

DESCRIPTION
	L2CAP connection has been disconnected by the local end.

RETURNS
	void
*/
void connectionHandleL2capDisconnectCfm(const L2CA_DISCONNECT_CFM_T *cfm);


/****************************************************************************
NAME	
	connectionHandleL2capDisconnectInd

DESCRIPTION
	L2CAP connection has been disconnected by the remote end.

RETURNS
	void
*/
void connectionHandleL2capDisconnectInd(const L2CA_DISCONNECT_IND_T *ind);


/****************************************************************************
NAME	
	connectionHandleL2capDisconnectReq

DESCRIPTION
	Request by the local device to disconnect the L2CAP connection

RETURNS
	void
*/
void connectionHandleL2capDisconnectReq(const CL_INTERNAL_L2CAP_DISCONNECT_REQ_T *req);


/****************************************************************************
NAME	
	connectionHandleL2capConnectTimeout

DESCRIPTION
	Connect timer has expired and we still haven't got a connection (if we 
	had this timer would not have fired) so disconnect everything and tell
	the client task.

RETURNS
	void
*/
void connectionHandleL2capConnectTimeout(const CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND_T *ind);


/****************************************************************************
NAME	
	connectionHandleQosSetupCfm

DESCRIPTION
	Confirmation of QOS parameters

RETURNS
	void
*/
void connectionHandleQosSetupCfm(const DM_HCI_QOS_SETUP_CFM_T* cfm);


/****************************************************************************
NAME	
	connectionHandleL2capInterlockDisconnectRsp

DESCRIPTION
	The client has read the CL_L2CAP_DISCONNECT_IND message so it is
    now safe to inform the remote device.

RETURNS
	void
*/
void connectionHandleL2capInterlockDisconnectRsp(const CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP_T* msg);


#endif	/* CONNECTION_L2CAP_HANDLER_H_ */
