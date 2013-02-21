/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    avrcp_signal_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_SIGNAL_HANDLER_H_
#define AVRCP_SIGNAL_HANDLER_H_


/****************************************************************************
NAME	
	avrcpSendPassthroughCfmToClient

DESCRIPTION
	This function creates a AVRCP_PASSTHROUGH_CFM message and sends it to 
	the client task.
*/
void avrcpSendPassthroughCfmToClient(AVRCP *avrcp, avrcp_status_code status);


/****************************************************************************
NAME	
	avrcpSendVendordependentCfmToClient

DESCRIPTION
	This function creates a AVRCP_VENDORDEPENDENT_CFM message and sends it to 
	the client task.
*/
void avrcpSendVendordependentCfmToClient(AVRCP *avrcp, avrcp_status_code status, uint8 response);


/****************************************************************************
NAME	
	avrcpSendUnitInfoCfmToClient

DESCRIPTION
	This function creates a AVRCP_UNITINFO_CFM message and sends it to 
	the client task.
*/
void avrcpSendUnitInfoCfmToClient(AVRCP *avrcp, avrcp_status_code status, uint16 unit_type, uint16 unit, uint32 company);


/****************************************************************************
NAME	
	avrcpSendSubunitInfoCfmToClient

DESCRIPTION
	This function creates a AVRCP_SUBUNITINFO_CFM message and sends it to 
	the client task.
*/
void avrcpSendSubunitInfoCfmToClient(AVRCP *avrcp, avrcp_status_code status, uint8 page, const uint8 *page_data);


/****************************************************************************
NAME	
	avrcpHandleInternalPassThroughReq

DESCRIPTION
	This function internally handles a pass through message request
*/
void avrcpHandleInternalPassThroughReq(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req);


/****************************************************************************
NAME	
	avrcpHandleFragmentedPassThroughReq

DESCRIPTION
	This function handles a pass through message request, with fragmentation
*/
void avrcpHandleFragmentedPassThroughReq(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req, uint16 max_mtu);


/****************************************************************************
NAME	
	avrcpHandleReceivedData

DESCRIPTION
	This function is called to process data received over the L2cap connection
*/
void avrcpHandleReceivedData(AVRCP *avrcp);


/****************************************************************************
NAME	
	avrcpProcessCommand

DESCRIPTION
	This function is called to process an AVRCP Command packet
*/
void avrcpProcessCommand(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleReceivedFragmentedMessage

DESCRIPTION
	
*/
void avrcpHandleReceivedFragmentedMessage(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpSendResponse
	
DESCRIPTION
	This function is used to send a response back to the CT device
*/
void avrcpSendResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size, avrcp_response_type response);


/****************************************************************************
NAME	
	avrcpProcessResponse

DESCRIPTION
	This function is called to process an AVRCP Response packet
*/
void avrcpProcessResponse(AVRCP *avrcp, const uint8 *ptr);


/****************************************************************************
NAME	
	avrcpHandleVendorMessage

DESCRIPTION
	This function process a vendor-dependent message that is received over the L2cap connection
*/
void avrcpHandleVendorMessage(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleInternalPassThroughRes

DESCRIPTION
	This function internally handles a pass through message result
*/
void avrcpHandleInternalPassThroughRes(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleInternalWatchdogTimeout

DESCRIPTION
	Called if the watchdog times out by the CT not receiving a response within
	the alloted time
*/
void avrcpHandleInternalWatchdogTimeout(AVRCP *avrcp);


/****************************************************************************
NAME	
	avrcpHandleInternalUnitInfoReq

DESCRIPTION
	This function internally handles unit info message request
*/
void avrcpHandleInternalUnitInfoReq(AVRCP *avrcp);


/****************************************************************************
NAME	
	avrcpHandleInternalUnitInfoRes

DESCRIPTION
	This function internally handles unit info message result
*/
void avrcpHandleInternalUnitInfoRes(AVRCP *avrcp, const AVRCP_INTERNAL_UNITINFO_RES_T *req);


/****************************************************************************
NAME	
	avrcpHandleInternalSubUnitInfoReq

DESCRIPTION
	This function internally handles subunit info message request
*/
void avrcpHandleInternalSubUnitInfoReq(AVRCP *avrcp, const AVRCP_INTERNAL_SUBUNITINFO_REQ_T *req);


/****************************************************************************
NAME	
	avrcpHandleInternalSubUnitInfoRes

DESCRIPTION
	This function internally handles subunit info message result
*/
void avrcpHandleInternalSubUnitInfoRes(AVRCP *avrcp, const AVRCP_INTERNAL_SUBUNITINFO_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleInternalVendorDependentReq

DESCRIPTION
	This function internally handles vendor-dependent message request
*/
void avrcpHandleInternalVendorDependentReq(AVRCP *avrcp, const AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *req);


/****************************************************************************
NAME	
	avrcpHandleInternalVendorDependentRes

DESCRIPTION
	This function internally handles vendor-dependent message result
*/
void avrcpHandleInternalVendorDependentRes(AVRCP *avrcp, const AVRCP_INTERNAL_VENDORDEPENDENT_RES_T *res);


#endif /* AVRCP_SIGNAL_HANDLER_H_ */
