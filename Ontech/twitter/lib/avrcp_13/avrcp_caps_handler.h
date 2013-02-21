/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_caps_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_CAPS_HANDLER_H_
#define AVRCP_CAPS_HANDLER_H_

#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_send_response.h"


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME	
	avrcpSendGetCapsCfm

DESCRIPTION
	Send a successful AVRCP_GET_CAPS_CFM message to the client task.
*/
void avrcpSendGetCapsCfm(AVRCP *avrcp, avrcp_status_code status, bool response, uint16 transaction, uint16 total_packets, uint16 packet_type, uint16 metadata_packet_type, avrcp_capability_id caps_id, uint16 number_of_caps, uint16 data_length, uint16 data_offset, Source source);
#endif

/****************************************************************************
NAME	
	avrcpHandleInternalGetCapsResponse

DESCRIPTION
	Internal handler for the AVRCP_INTERNAL_GET_CAPS_RES message.
*/
void avrcpHandleInternalGetCapsResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_CAPS_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleGetCapsCommand

DESCRIPTION
	Handle a Get Caps command PDU arriving from the CT.
*/
void avrcpHandleGetCapsCommand(AVRCP *avrcp, uint16 transaction, avrcp_capability_id caps);


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME	
	avrcpHandleGetCapsResponse

DESCRIPTION
	Handle a Get Caps response PDU arriving from the TG.
*/
bool avrcpHandleGetCapsResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);
#endif

#endif /* AVRCP_CAPS_HANDLER_H_ */
