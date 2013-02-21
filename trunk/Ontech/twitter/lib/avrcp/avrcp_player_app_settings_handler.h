/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_player_app_settings_handler.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_PLAYER_APP_SETTINGS_HANDLER_H_
#define AVRCP_PLAYER_APP_SETTINGS_HANDLER_H_


#include "avrcp_private.h"
#include "avrcp_send_response.h"


/****************************************************************************
NAME	
	avrcpHandleListAppAttributesCommand

DESCRIPTION
	Handle List App Attributes command PDU received from CT.
*/
void avrcpHandleListAppAttributesCommand(AVRCP *avrcp, uint16 transaction);


/****************************************************************************
NAME	
	avrcpHandleListAppAttributesResponse

DESCRIPTION
	Handle List App Attributes response PDU received from TG.
*/
bool avrcpHandleListAppAttributesResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleInternalListAppAttributesResponse

DESCRIPTION
	Internal handler for the AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES message.
*/
void avrcpHandleInternalListAppAttributesResponse(AVRCP *avrcp, AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleListAppValuesCommand

DESCRIPTION
	Handle List App Values command PDU received from CT.
*/
void avrcpHandleListAppValuesCommand(AVRCP *avrcp, uint16 transaction, uint16 attribute_id);


/****************************************************************************
NAME	
	avrcpHandleListAppValuesResponse

DESCRIPTION
	Handle List App Values response PDU received from TG.
*/
bool avrcpHandleListAppValuesResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleInternalListAppValuesResponse

DESCRIPTION
	Internal handler for the AVRCP_INTERNAL_LIST_APP_VALUE_RES message.
*/
void avrcpHandleInternalListAppValuesResponse(AVRCP *avrcp, AVRCP_INTERNAL_LIST_APP_VALUE_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleGetAppValuesCommand

DESCRIPTION
	Handle Get App Values command PDU received from CT.
*/
bool avrcpHandleGetAppValuesCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleGetAppValuesResponse

DESCRIPTION
	Handle Get App Values response PDU received from TG.
*/
bool avrcpHandleGetAppValuesResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleInternalGetAppValueResponse

DESCRIPTION
	Internal handler for the AVRCP_INTERNAL_GET_APP_VALUE_RES message.
*/
void avrcpHandleInternalGetAppValueResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_APP_VALUE_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleSetAppValuesCommand

DESCRIPTION
	Handle Set App Values command PDU received from CT.
*/
bool avrcpHandleSetAppValuesCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleSetAppValuesResponse

DESCRIPTION
	Handle Set App Values response PDU received from TG.
*/
bool avrcpHandleSetAppValuesResponse(AVRCP *avrcp, avrcp_status_code status, uint16 transaction, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleInternalSetAppValueResponse

DESCRIPTION
	Internal handler for the AVRCP_INTERNAL_SET_APP_VALUE_RES message.
*/
void avrcpHandleInternalSetAppValueResponse(AVRCP *avrcp, AVRCP_INTERNAL_SET_APP_VALUE_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleGetAppAttributeTextCommand

DESCRIPTION
	Handle Get App Attribute Text command PDU received from CT.
*/
bool avrcpHandleGetAppAttributeTextCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleGetAppAttributeTextResponse

DESCRIPTION
	Handle Get App Attribute Text response PDU received from TG.
*/
bool avrcpHandleGetAppAttributeTextResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleInternalGetAppAttributeTextResponse

DESCRIPTION
	Internal handler for the AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES message.
*/
void avrcpHandleInternalGetAppAttributeTextResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES_T *res);


/****************************************************************************
NAME	
	avrcpHandleGetAppValueTextCommand

DESCRIPTION
	Handle Get App Value Text command PDU received from CT.
*/
bool avrcpHandleGetAppValueTextCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleGetAppValueTextResponse

DESCRIPTION
	Handle Get App Value Text response PDU received from TG.
*/
bool avrcpHandleGetAppValueTextResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size);


/****************************************************************************
NAME	
	avrcpHandleInternalGetAppValueTextResponse

DESCRIPTION
	Internal handler for the AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES message.
*/
void avrcpHandleInternalGetAppValueTextResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES_T *res);


#endif /* AVRCP_PLAYER_APP_SETTINGS_HANDLER_H_ */
