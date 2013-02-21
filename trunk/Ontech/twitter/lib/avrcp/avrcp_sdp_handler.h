/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    avrcp_sdp_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_SDP_HANDLER_H_
#define AVRCP_SDP_HANDLER_H_


/****************************************************************************
NAME	
	avrcpRegisterServiceRecord

DESCRIPTION
    Attempt to register a service record for the RCP service.
*/
void avrcpRegisterServiceRecord(AVRCP *avrcp);


/****************************************************************************
NAME	
	avrcpHandleSdpRegisterCfm

DESCRIPTION
	This function is called on receipt of an CL_SDP_REGISTER_CFM
	indicating the outcome of registering a service record.
*/
void avrcpHandleSdpRegisterCfm(AVRCP *avrcp, const CL_SDP_REGISTER_CFM_T *cfm);


#endif /* AVRCP_SDP_HANDLER_H_ */
