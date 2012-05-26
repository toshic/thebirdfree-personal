/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_slc_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_SLC_HANDLER_H_
#define HFP_SLC_HANDLER_H_


/****************************************************************************
NAME	
	hfpHandleSlcConnectRequest

DESCRIPTION
	Initiate the creation of a profile service level connection.

RETURNS
	void
*/
void hfpHandleSlcConnectRequest(HFP *hfp, const HFP_INTERNAL_SLC_CONNECT_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleSlcConnectResponse

DESCRIPTION
	Respond to a request to create an SLC from the remote device.

RETURNS
	void
*/
void hfpHandleSlcConnectResponse(HFP *hfp, const HFP_INTERNAL_SLC_CONNECT_RES_T *res);


/****************************************************************************
NAME	
	hfpHandleSlcConnectIndReject

DESCRIPTION
	Reject the connect request outright because this profile instance is not 
	in the correct state.

RETURNS
	void
*/
void hfpHandleSlcConnectIndReject(HFP *hfp, const CL_RFCOMM_CONNECT_IND_T *ind);


/****************************************************************************
NAME	
	hfpSendSlcConnectCfmToApp

DESCRIPTION
	Send a HFP_SLC_CONNECT_CFM message to the app telling it the outcome
	of the connect attempt.

RETURNS
	void
*/
void hfpSendSlcConnectCfmToApp(hfp_connect_status status, HFP *hfp);


/****************************************************************************
NAME	
	hfpSendRemoteAGProfileVerIndToApp

DESCRIPTION
	Send a HFP_REMOTE_AG_PROFILE15_IND message to the app telling it the remote
	AG supports profile hfp version 1.5.

RETURNS
	void
*/
void hfpSendRemoteAGProfileVerIndToApp(hfp_profile profile, HFP *hfp);



/****************************************************************************
NAME	
	hfpHandleBrsfRequest

DESCRIPTION
	Send AT+BRSF to the AG.

RETURNS
	void
*/
void hfpHandleBrsfRequest(HFP *hfp);


/****************************************************************************
NAME	
	hfpSendInternalBrsfMessage

DESCRIPTION
	Send an internal message that we've got the AG's supported features.

RETURNS
	void
*/
void hfpHandleSupportedFeaturesNotification(HFP *hfp, uint16 features);


/****************************************************************************
NAME	
	hfpHandleBrsfAtAck

DESCRIPTION
	Called when we receive OK/ ERROR in response to the AT+BRSF cmd. This 
	indicates whether the AG recognised the cmd.

RETURNS
	void
*/
void hfpHandleBrsfAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleCindTestRequest

DESCRIPTION
	Send AT+CIND=? to the AG.

RETURNS
	void
*/
void hfpHandleCindTestRequest(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleCindReadRequest

DESCRIPTION
	Send AT+CIND? to the AG.

RETURNS
	void
*/
void hfpHandleCindReadRequest(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleCmerRequest

DESCRIPTION
	Send AT+CMER to the AG.

RETURNS
	void
*/
void hfpHandleCmerRequest(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleCmerAtAck

DESCRIPTION
	Called when we receive OK/ ERROR in response to the AT+CMER cmd. If we're
	not getting call hold params from the AG then the SLC is complete.

RETURNS
	void
*/
void hfpHandleCmerAtAck(HFP *hfp, hfp_lib_status status);


/****************************************************************************
NAME	
	hfpHandleDisconnectRequestFail

DESCRIPTION
	Received a disconnect request from the app when we haven't got a 
	connecting and are not attempting to connect. Return error message.

RETURNS
	void
*/
void hfpHandleDisconnectRequestFail(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleDisconnectRequest

DESCRIPTION
	We're in the right state and have received a disconnect request, 
	handle it here.

RETURNS
	void
*/
void hfpHandleDisconnectRequest(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleSlcDisconnectIndication

DESCRIPTION
	Handle disconnection of a profile service level connection.

RETURNS
	void
*/
void hfpHandleSlcDisconnectIndication(HFP *hfp, const HFP_INTERNAL_SLC_DISCONNECT_IND_T *ind);


/****************************************************************************
NAME	
	hfpSendSlcDisconnectIndToApp

DESCRIPTION
	Send a HFP_SLC_DISCONNECT_IND message to the app notifying it that
	the SLC has been disconnected.

RETURNS
	void
*/
void hfpSendSlcDisconnectIndToApp(HFP *hfp, hfp_disconnect_status status);


/****************************************************************************
NAME	
	hfpHandleSinkRequest

DESCRIPTION
	If we have a valid sink return it otherwise send back an error

RETURNS
	void
*/
void hfpHandleSinkRequest(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleSinkRequestFail

DESCRIPTION
	Return an error, the profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleSinkRequestFail(HFP *hfp);


#endif /* HFP_SLC_HANDLER_H_ */
