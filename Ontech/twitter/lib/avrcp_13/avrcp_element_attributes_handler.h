/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_element_attributes_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_ELEMENT_ATTRIBUTES_HANDLER_H_
#define AVRCP_ELEMENT_ATTRIBUTES_HANDLER_H_


#include "avrcp_private.h"
#include "avrcp_send_response.h"


/****************************************************************************
NAME	
	avrcpHandleGetElementAttributesCommand

DESCRIPTION
	Handle Get Element attributes command PDU received from CT.
*/
bool avrcpHandleGetElementAttributesCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleGetElementAttributesResponse

DESCRIPTION
	Handle Get Element attributes response PDU received from TG.
*/
bool avrcpHandleGetElementAttributesResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleInternalGetElementAttributesResponse

DESCRIPTION
	Respond to an AVRCP_ELEMENT_ATTRIBUTES_IND message.
*/
void avrcpHandleInternalGetElementAttributesResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES_T *res);


#endif /* AVRCP_ELEMENT_ATTRIBUTES_HANDLER_H_ */
