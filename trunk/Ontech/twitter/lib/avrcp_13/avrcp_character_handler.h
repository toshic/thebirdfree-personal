/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_character_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_CHARACTER_HANDLER_H_
#define AVRCP_CHARACTER_HANDLER_H_

#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_send_response.h"


/****************************************************************************
NAME	
	avrcpHandleInformCharSetCommand

DESCRIPTION
	Process the command PDU that has arrrived from the CT.
*/
bool avrcpHandleInformCharSetCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


#endif /* AVRCP_CHARACTER_HANDLER_H_ */
