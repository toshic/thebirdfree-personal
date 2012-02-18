/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Audio-Adaptor-SDK 2009.R1
*/


#ifndef AGHFP_RFC_H_
#define AGHFP_RFC_H_


/****************************************************************************
	Request an RFCOMM channel to be allocated.
*/
void aghfpHandleRfcommAllocateChannel(AGHFP *aghfp);


/****************************************************************************
	Rfcomm channel has been allocated.
*/
void aghfpHandleRfcommRegisterCfm(AGHFP *aghfp, const CL_RFCOMM_REGISTER_CFM_T *cfm);


/****************************************************************************
	Issue a request to the connection lib to create an RFCOMM connection.
*/
void aghfpHandleRfcommConnectRequest(AGHFP *aghfp, const AGHFP_INTERNAL_RFCOMM_CONNECT_REQ_T *req);


/****************************************************************************
	Response to an incoming RFCOMM connect request.
*/
void aghfpHandleRfcommConnectResponse(AGHFP *aghfp, bool response, const bdaddr *addr, uint8 server_channel, const rfcomm_config_params *config);


/****************************************************************************
	Outcome of the RFCOMM connect request or response.
*/
void aghfpHandleRfcommConnectCfm(AGHFP *aghfp, const CL_RFCOMM_CONNECT_CFM_T *cfm);


/****************************************************************************
	Notification of an incoming rfcomm connection, pass this up to the app
	to decide whather to accept this or not.
*/
void aghfpHandleRfcommConnectInd(AGHFP *aghfp, const CL_RFCOMM_CONNECT_IND_T *ind);


/****************************************************************************
	Issue an RFCOMM disconnect to the connection lib.
*/
void aghfpHandleRfcommDisconnectRequest(AGHFP *aghfp);


/****************************************************************************
	Indication that the RFCOMM connection has been disconnected.
*/
void aghfpHandleRfcommDisconnectInd(AGHFP *aghfp, const CL_RFCOMM_DISCONNECT_IND_T *ind);


#endif /* AGHFP_RFC_H_ */
