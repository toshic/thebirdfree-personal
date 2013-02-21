/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_signal_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_SIGNAL_PASSTHROUGH_H_
#define AVRCP_SIGNAL_PASSTHROUGH_H_

#include "avrcp.h"


#ifdef AVRCP_CT_SUPPORT
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
	avrcpHandleInternalPassThroughReq

DESCRIPTION
	This function internally handles a pass through message request.
*/
void avrcpHandleInternalPassThroughReq(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req);


/****************************************************************************
NAME	
	avrcpHandleFragmentedPassThroughReq

DESCRIPTION
	This function handles a pass through message request, with fragmentation.
*/
void avrcpHandleFragmentedPassThroughReq(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req, uint8 *dest, uint16 max_mtu);
#endif

/****************************************************************************
NAME	
	avrcpHandleInternalPassThroughRes

DESCRIPTION
	This function internally handles a pass through message result.
*/
void avrcpHandleInternalPassThroughRes(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandlePassthroughCommand

DESCRIPTION
	This function internally handles a pass through command
    received from a remote device.
*/
void avrcpHandlePassthroughCommand(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size);


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME	
	avrcpHandlePassthroughResponse

DESCRIPTION
	This function internally handles a packet pass through response
    received from a remote device.
*/
void avrcpHandlePassthroughResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size);
#endif

#endif /* AVRCP_SIGNAL_PASSTHROUGH_H_ */
