/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_signal_passthrough.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_SIGNAL_PASSTHROUGH_H_
#define AVRCP_SIGNAL_PASSTHROUGH_H_

#include "avrcp_common.h"

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


/****************************************************************************
NAME    
    avrcpHandlePassthroughResponse

DESCRIPTION
    This function internally handles a packet pass through response
    received from a remote device.
*/
void avrcpHandlePassthroughResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size);


/****************************************************************************
NAME    
    avrcpSendGroupIndToClient

DESCRIPTION
    Send indication of the command up to the app.
*/
void avrcpSendGroupIndToClient(AVRCP *avrcp, uint16 vendor_id, uint8 transaction);


/****************************************************************************
NAME    
    avrcpHandleInternalGroupResponse

DESCRIPTION
    Process the response to be sent to CT.
*/
void avrcpHandleInternalGroupResponse(AVRCP *avrcp, 
                        const AVRCP_INTERNAL_GROUP_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleGroupResponse

DESCRIPTION
    Handle the response received from the TG.
*/
void avrcpHandleGroupResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size);


#endif /* AVRCP_SIGNAL_PASSTHROUGH_H_ */
