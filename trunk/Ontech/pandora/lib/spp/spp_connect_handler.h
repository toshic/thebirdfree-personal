/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2005-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    spp_connect_handler.h
    
DESCRIPTION
	
*/
#ifndef SPP_CONNECT_HANDLER_H_
#define SPP_CONNECT_HANDLER_H_


#include "spp_private.h"

#include <connection.h>


/*********************************************************************
NAME	
	sppHandleConnectRequest

DESCRIPTION
	Handles internal connect request by initiating an SDP search for 
    the rfcomm channel number on the remote device.
*/
void sppHandleConnectRequest(SPP *spp, const SPP_INTERNAL_CONNECT_REQ_T *message);


/****************************************************************************
NAME	
	sppHandleInternalRfcommConnectRequest

DESCRIPTION
	Handles internal connect request by issuing a connect request to 
    the connection library.
*/
void sppHandleInternalRfcommConnectRequest(SPP *spp, const SPP_INTERNAL_RFCOMM_CONNECT_REQ_T *message);


/****************************************************************************
NAME	
	sppHandleConnectResponse

DESCRIPTION
	Respond to a request to create a conection from the remote device.
*/
void sppHandleConnectResponse(SPP *spp, const SPP_INTERNAL_CONNECT_RES_T *res);


/****************************************************************************
NAME	
	sppSendConnectCfmToApp

DESCRIPTION
	Sends connect confirm back to the client task.
*/
void sppSendConnectCfmToApp(spp_connect_status status, SPP *spp);


/****************************************************************************
NAME	
	sppHandleRfcommConnectCfm

DESCRIPTION
	Handles rfcomm connect confirm received from the connection library.
*/
void sppHandleRfcommConnectCfm(SPP *spp, const CL_RFCOMM_CONNECT_CFM_T *cfm);


/****************************************************************************
NAME	
	sppHandleRfcommConnectInd

DESCRIPTION
	Handles a CL_RFCOMM_CONNECT_IND message from the connection library
    indicating that another device is requesting connection to this one.
*/
void sppHandleRfcommConnectInd(SPP *spp, const CL_RFCOMM_CONNECT_IND_T *ind);


/****************************************************************************
NAME	
	sppHandleConnectIndReject

DESCRIPTION
	Reject an incoming connection.
*/
void sppHandleConnectIndReject(SPP *spp, const CL_RFCOMM_CONNECT_IND_T *ind);


/****************************************************************************
NAME	
	sppHandleRfcommDisconnectInd

DESCRIPTION
	Handles an CL_RFCOMM_DISCONNECT_IND message sent by the connection library
    to indicate that the rfcomm connection has been disconnected.
*/
void sppHandleRfcommDisconnectInd(SPP *spp, const CL_RFCOMM_DISCONNECT_IND_T *ind);


/****************************************************************************
NAME	
	sppHandleInternalDisconnectReq

DESCRIPTION
	Handle internal message sent to request a disconnect.
*/
void sppHandleInternalDisconnectReq(SPP *spp);


/****************************************************************************
NAME	
	sppSendDisconnectIndToApp

DESCRIPTION
	Send an SPP_DISCONNECT_IND message to the client task.
*/
void sppSendDisconnectIndToApp(SPP *spp, spp_disconnect_status status);


/****************************************************************************
NAME	
	sppHandleFreeSppTask

DESCRIPTION
	Handle the message requesting that the SPP task be freed.
*/
void sppHandleFreeSppTask(SPP *spp);


#endif /* SPP_CONNECT_HANDLER_H_ */
