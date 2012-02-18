/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    rfc_handler.h
    
DESCRIPTION

*/

#ifndef	CONNECTION_RFC_HANDLER_H_
#define	CONNECTION_RFC_HANDLER_H_

/* If we don't get an rfc_REGISTER_CFM by the time this expires, assume it failed */
#define RFCOMM_REGISTER_TIMEOUT	    D_SEC(2)


/****************************************************************************
NAME	
	connectionHandleRfcommRegisterReq

DESCRIPTION
    This function handler is called in receipt of an RFC_REGISTER_CFM from
    Bluestack

RETURNS
	
*/
void connectionHandleRfcommRegisterReq(connectionRfcommState* rfcommState, const CL_INTERNAL_RFCOMM_REGISTER_REQ_T* req);


/****************************************************************************
NAME	
	connectionHandleRfcommRegisterCfm

DESCRIPTION
    This function 

RETURNS
	
*/
void connectionHandleRfcommRegisterCfm(connectionRfcommState* rfcommState, const RFC_REGISTER_CFM_T* cfm);


/****************************************************************************
NAME	
	connectionHandleRfcommRegisterTimeout

DESCRIPTION
	RFCOMM register timeout message has fired, means we didn't get a cfm
	for the last register request we issued.

RETURNS
	void
*/
void connectionHandleRfcommRegisterTimeout(const CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND_T *ind);


/****************************************************************************
NAME	
	connectionHandleRfcommConnectReq

DESCRIPTION
    This function handler is called in response to a CL_INTERNAL_RFCOMM_CONNECT_REQ
    message as a result of a client application requesing an RFCOMM connection

RETURNS
	
*/
void connectionHandleRfcommConnectReq(connectionRfcommState* rfcommState, const CL_INTERNAL_RFCOMM_CONNECT_REQ_T* req);


/****************************************************************************
NAME	
	connectionHandleRfcommConnectTimeout

DESCRIPTION
	RFCOMM CONNECT timeout message has fired, means we haven't established
    an RFCOMM connection within the client's specified timeout

RETURNS
	void
*/
void connectionHandleRfcommConnectTimeout(connectionRfcommState* rfcommState, const CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND_T *ind);


/****************************************************************************
NAME	
	connectionHandleRfcommConnectRes

DESCRIPTION
    This function handler is called in response to a CL_INTERNAL_RFCOMM_CONNECT_RES
    message as a result of a remote device requesting an RFCOMM connection

RETURNS
	
*/
void connectionHandleRfcommConnectRes(const CL_INTERNAL_RFCOMM_CONNECT_RES_T* res);


/****************************************************************************
NAME	
	connectionHandleRfcommDisconnectReq

DESCRIPTION
    This function handler is called in response to a CL_INTERNAL_RFCOMM_DISCONNECT_REQ
    message as a result of the local device requesting the disconnection of the 
    connection identified by the connection sink

RETURNS
	
*/
void connectionHandleRfcommDisconnectReq(const CL_INTERNAL_RFCOMM_DISCONNECT_REQ_T* req);


/****************************************************************************
NAME	
	connectionHandleRfcommControlReq

DESCRIPTION
    This function handler is called in response to a CL_INTERNAL_RFCOMM_CONTROL_REQ
    message as a result of the client device requesting to send it's control
    status signals to the peer device

RETURNS
	
*/
void connectionHandleRfcommControlReq(const CL_INTERNAL_RFCOMM_CONTROL_REQ_T* req);


/****************************************************************************
NAME	
	connectionHandleRfcommStartCfm

DESCRIPTION
    This function handler is called in response to an RFC_START_CFM message
    being sent by Bluestack.  This message is sent to indicate the status of 
    the requested RFCOMM MUX session.
  
RETURNS
	
*/
void connectionHandleRfcommStartCfm(connectionRfcommState* rfcommState, const RFC_START_CFM_T* cfm);


/****************************************************************************
NAME	
	connectionHandleRfcommStartInd

DESCRIPTION
    This function handler is called in response to an RFC_START_IND message
    being sent by Bluestack.  This message is sent to indicate that a peer
    device is attempting to establish a MUX session.  Send back an RFC_START_RES
  
RETURNS
	
*/
void connectionHandleRfcommStartInd(const RFC_START_IND_T* ind);


/****************************************************************************
NAME	
	connectionHandleRfcommStartCmpInd

DESCRIPTION
    This function handler is called in response to an RFC_STARTCMP_IND message
    being sent by Bluestack.  This message is sent to indicate that the
    underlying L2CAP connection has been established.  Providing the result
    code is Success then we need to store the L2CAP MTU size
  
RETURNS
	
*/
void connectionHandleRfcommStartCmpInd(const RFC_STARTCMP_IND_T* ind);


/****************************************************************************
NAME	
	connectionHandleRfcommParnegInd

DESCRIPTION
    This function handler is called in response to an RFC_PARNEG_IND message
    being sent by Bluestack.  
  
RETURNS
	
*/
void connectionHandleRfcommParnegInd(const RFC_EX_PARNEG_IND_T* ind);


/****************************************************************************
NAME	
	connectionHandleRfcommParnegCfm

DESCRIPTION
    This function handler is called in response to an RFC_PARNEG_CFM message
    being sent by Bluestack.  
  
RETURNS
	
*/
void connectionHandleRfcommParnegCfm(const RFC_PARNEG_CFM_T* cfm);


/****************************************************************************
NAME	
	connectionHandleRfcommControlInd

DESCRIPTION
    This function handler is called in response to an RFC_CONTROL_IND message
    being sent by Bluestack. 
  
RETURNS
	
*/
void connectionHandleRfcommControlInd(const RFC_CONTROL_IND_T* ind);


/****************************************************************************
NAME	
	connectionHandleRfcommReleaseInd

DESCRIPTION
    This function handler is called in response to an RFC_RELEASE_IND message
    being sent by Bluestack.  This indicates that the RFCOMM connection has
    been disconnected
  
RETURNS
	
*/
void connectionHandleRfcommReleaseInd(const RFC_EX_RELEASE_IND_T* ind);


/****************************************************************************
NAME	
	connectionHandleRfcommEstablishInd

DESCRIPTION
    This function handler is called in response to an RFC_ESTABLISH_IND message
    being sent by Bluestack.  
  
RETURNS
	
*/
void connectionHandleRfcommEstablishInd(const RFC_EX_ESTABLISH_IND_T* ind);


/****************************************************************************
NAME	
	connectionHandleRfcommEstablishCfm

DESCRIPTION
    This function handler is called in response to an RFC_ESTABLISH_CFM 
    message being sent by Bluestack.  
  
RETURNS
	
*/
void connectionHandleRfcommEstablishCfm(const RFC_ESTABLISH_CFM_T* cfm);


/****************************************************************************
NAME	
	connectionHandleRfcommPortNegInd

DESCRIPTION
    This function handler is called in response to an RFC_PORTNEG_IND message
    being sent by Bluestack.
  
RETURNS
	
*/
void connectionHandleRfcommPortNegInd(const RFC_PORTNEG_IND_T* ind);


#endif	/* CONNECTION_RFC_HANDLER_H_ */


