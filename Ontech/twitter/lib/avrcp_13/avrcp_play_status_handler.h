/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_play_status_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_PLAY_STATUS_HANDLER_H_
#define AVRCP_PLAY_STATUS_HANDLER_H_


#include "avrcp_private.h"
#include "avrcp_send_response.h"

#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME	
	avrcpSendGetPlayStatusCfm

DESCRIPTION
	Send an AVRCP_GET_PLAY_STATUS_CFM message to the client.
*/
void avrcpSendGetPlayStatusCfm(AVRCP *avrcp, avrcp_status_code status, uint32 song_length, uint32 song_elapsed, avrcp_play_status play_status, uint8 transaction, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleGetPlayStatusResponse

DESCRIPTION
	Handle Get Play Status response received from the TG.
*/
bool avrcpHandleGetPlayStatusResponse(AVRCP *avrcp, uint16 transaction, const uint8 *ptr, Source source, uint16 packet_size);
#endif

/****************************************************************************
NAME	
	avrcpHandleInternalGetPlayStatusResponse

DESCRIPTION
	Internal handler for the AVRCP_INTERNAL_GET_PLAY_STATUS_RES message.
*/
void avrcpHandleInternalGetPlayStatusResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_PLAY_STATUS_RES_T *res);


#endif /* AVRCP_PLAY_STATUS_HANDLER_H_ */
