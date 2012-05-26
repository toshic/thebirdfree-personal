/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    hfp_audio_handler.h
    
DESCRIPTION
	
*/

#ifndef HFP_AUDIO_HANDLER_H_
#define HFP_AUDIO_HANDLER_H_


/****************************************************************************
NAME	
	hfpManageSyncDisconnect

DESCRIPTION
    Used to inform hfp of a synchronous (audio) disconnection.
    
RETURNS
	void
*/
void hfpManageSyncDisconnect(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleSyncConnectInd

DESCRIPTION
	Incoming Synchronous connect notification, accept if we recognise the sink 
	reject otherwise.

RETURNS
	void
*/
void hfpHandleSyncConnectInd(HFP *hfp, const CL_DM_SYNC_CONNECT_IND_T *ind);


/****************************************************************************
NAME	
	hfpHandleSyncConnectIndReject

DESCRIPTION
	Incoming Synchronous connect notification, reject outright, profile is in 
	the wrong state.  This is probably a synchronous connect indication for a
	different task.

RETURNS
	void
*/
void hfpHandleSyncConnectIndReject(HFP *hfp, const CL_DM_SYNC_CONNECT_IND_T *ind);


/****************************************************************************
NAME	
	hfpHandleSyncConnectCfm

DESCRIPTION
	Confirmation in response to a Synchronous connect request indicating the 
	outcome of the connect attempt.

RETURNS
	void
*/
void hfpHandleSyncConnectCfm(HFP *hfp, const CL_DM_SYNC_CONNECT_CFM_T *cfm);


/****************************************************************************
NAME	
	hfpHandleAudioDisconnectReq

DESCRIPTION
	Attempt to disconnect the Synchronous connection.

RETURNS
	void
*/
void hfpHandleAudioDisconnectReq(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleAudioDisconnectReqError

DESCRIPTION
    Attempt has been made to disconnect an audio connection request.  However,
    HFP library is not in the correct state to process the request.

RETURNS
	void
*/
void hfpHandleAudioDisconnectReqError(HFP *hfp);


/****************************************************************************
NAME	
	hfpHandleSyncDisconnectInd

DESCRIPTION
	The Synchronous connection has been disconnected 

RETURNS
	void
*/
void hfpHandleSyncDisconnectInd(HFP *hfp, const CL_DM_SYNC_DISCONNECT_IND_T *ind);


/****************************************************************************
NAME	
	hfpHandleAudioConnectReq

DESCRIPTION
	Transfer the audio from the AG to the HF or vice versa depending on which
	device currently has it.

RETURNS
	void
*/
void hfpHandleAudioConnectReq(HFP *hfp, const HFP_INTERNAL_AUDIO_CONNECT_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleAudioConnectReqError

DESCRIPTION
	The app has requested that the audio be transferred (either to or from the AG).
	If the profile instance was in the wrong state for this operation to be 
	performed we need to send an immediate response to the app indicating an
	error has ocurred. We send the a SCO status message with the status code 
	set to hfp_fail. We determine which message to send by looking at the action
	the app has requested and the current state of the HFP instance.

RETURNS
	void
*/
void hfpHandleAudioConnectReqError(HFP *hfp, const HFP_INTERNAL_AUDIO_CONNECT_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleAudioConnectRes

DESCRIPTION
    Accept or reject to an incoming audio connection request from remote device.

RETURNS
	void
*/
void hfpHandleAudioConnectRes(HFP *hfp, const HFP_INTERNAL_AUDIO_CONNECT_RES_T *res);


/****************************************************************************
NAME	
	hfpHandleAudioConnectResError

DESCRIPTION
    Attempt has been made to accept/reject an incoming audio connection request.  However,
    HFP library is not in the correct state to process the response.

RETURNS
	void
*/
void hfpHandleAudioConnectResError(HFP *hfp, const HFP_INTERNAL_AUDIO_CONNECT_RES_T *res);


/****************************************************************************
NAME	
	hfpHandleAudioTransferReq

DESCRIPTION
	Transfer the audio from the AG to the HF or vice versa depending on which
	device currently has it.

RETURNS
	void
*/
void hfpHandleAudioTransferReq(HFP *hfp, const HFP_INTERNAL_AUDIO_TRANSFER_REQ_T *req);


/****************************************************************************
NAME	
	hfpHandleAudioTransferReqError

DESCRIPTION
	The app has requested that the audio be transferred (either to or from the AG).
	If the profile instance was in the wrong state for this operation to be 
	performed we need to send an immediate response to the app indicating an
	error has ocurred. We send the a audio status message with the status code 
	set to hfp_fail. We determine which message to send by looking at the action
	the app has requested and the current state of the HFP instance.

RETURNS
	void
*/
void hfpHandleAudioTransferReqError(HFP *hfp, const HFP_INTERNAL_AUDIO_TRANSFER_REQ_T *req);


#endif /* HFP_AUDIO_HANDLER_H_ */
