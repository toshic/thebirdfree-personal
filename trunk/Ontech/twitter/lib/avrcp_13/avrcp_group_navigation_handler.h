/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_group_navigation_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_GROUP_NAVIGATION_HANDLER_H_
#define AVRCP_GROUP_NAVIGATION_HANDLER_H_


#include "avrcp_private.h"
#include "avrcp_send_response.h"


/****************************************************************************
NAME	
	avrcpSendGroupIndToClient

DESCRIPTION
	Send indication of the command up to the app.
*/
void avrcpSendGroupIndToClient(AVRCP *avrcp, uint16 vendor_id, uint8 transaction);


/****************************************************************************
NAME	
	avrcpHandleInternalNextGroupResponse

DESCRIPTION
	Process the response to be sent to CT.
*/
void avrcpHandleInternalNextGroupResponse(AVRCP *avrcp, const AVRCP_INTERNAL_NEXT_GROUP_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleInternalPreviousGroupResponse

DESCRIPTION
	Process the response to be sent to CT.
*/
void avrcpHandleInternalPreviousGroupResponse(AVRCP *avrcp, const AVRCP_INTERNAL_PREVIOUS_GROUP_RES_T *res);


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME	
	avrcpHandleGroupResponse

DESCRIPTION
	Handle the response received from the TG.
*/
void avrcpHandleGroupResponse(AVRCP *avrcp, Source source, uint16 packet_size);
#endif

#endif /* AVRCP_GROUP_NAVIGATION_HANDLER_H_ */
