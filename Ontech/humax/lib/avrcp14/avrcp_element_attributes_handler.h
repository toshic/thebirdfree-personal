/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_element_attributes_handler.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_ELEMENT_ATTRIBUTES_HANDLER_H_
#define AVRCP_ELEMENT_ATTRIBUTES_HANDLER_H_

#include "avrcp_common.h"

/****************************************************************************
NAME    
    avrcpHandleGetElementAttributesCommand

DESCRIPTION
    Handle Get Element attributes command PDU received from CT.
*/
void avrcpHandleGetElementAttributesCommand(AVRCP *avrcp, uint16 meta_packet_type, const uint8* data, uint16 packet_size);



/****************************************************************************
NAME    
    avrcpHandleInternalGetElementAttributesResponse

DESCRIPTION
    Respond to an AVRCP_ELEMENT_ATTRIBUTES_IND message.
*/
void avrcpHandleInternalGetElementAttributesResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES_T *res);


#endif /* AVRCP_ELEMENT_ATTRIBUTES_HANDLER_H_ */
