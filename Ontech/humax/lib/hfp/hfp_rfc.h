/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_rfc.h
    
DESCRIPTION
	
*/

#ifndef HFP_RFC_H_
#define HFP_RFC_H_


/****************************************************************************
NAME	
	hfpHandleRfcommAllocateChannel

DESCRIPTION
	Request an RFCOMM channel to be allocated.

RETURNS
	void
*/
void hfpHandleRfcommAllocateChannel(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleRfcommRegisterCfm

DESCRIPTION
	Rfcomm channel has been allocated.

RETURNS
	void
*/
void hfpHandleRfcommRegisterCfm(HFP *hfp, const CL_RFCOMM_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME	
	hfpHandleRfcommConnectRequest

DESCRIPTION
	Issue a request to the connection lib to create an RFCOMM connection.

RETURNS
	void
*/
void hfpHandleRfcommConnectRequest(HFP *hfp, const HFP_INTERNAL_RFCOMM_CONNECT_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleRfcommConnectResponse

DESCRIPTION
	Response to an incoming RFCOMM connect request.

RETURNS
	void
*/
void hfpHandleRfcommConnectResponse(HFP *hfp, bool response, const bdaddr *addr, uint8 server_channel, const rfcomm_config_params *config);


/****************************************************************************
NAME	
	hfpHandleRfcommConnectCfm

DESCRIPTION
	Outcome of the RFCOMM connect request or response.

RETURNS
	void
*/
void hfpHandleRfcommConnectCfm(HFP *hfp, const CL_RFCOMM_CONNECT_CFM_T *cfm);


/****************************************************************************
NAME	
	hfpHandleRfcommConnectInd

DESCRIPTION
	Notification of an incoming rfcomm connection, pass this up to the app
	to decide whather to accept this or not.

RETURNS
	void
*/
void hfpHandleRfcommConnectInd(HFP *hfp, const CL_RFCOMM_CONNECT_IND_T *ind);


/****************************************************************************
NAME	
	hfpHandleRfcommDisconnectRequest

DESCRIPTION
	Issue an RFCOMM disconnect to the connection lib.

RETURNS
	void
*/
void hfpHandleRfcommDisconnectRequest(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleRfcommDisconnectInd

DESCRIPTION
	Indication that the RFCOMM connection has been disconnected.
*/
void hfpHandleRfcommDisconnectInd(HFP *hfp, const CL_RFCOMM_DISCONNECT_IND_T *ind);


/***************************************************************************
NAME	
	hfpHandleRfcommControlInd

DESCRIPTION
	Control indication from the connection lib, needs to be replied to.
*/
void hfpHandleRfcommControlInd(HFP *hfp, const CL_RFCOMM_CONTROL_IND_T *ind);


#endif /* HFP_RFC_H_ */
