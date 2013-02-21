/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_send_response.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_private.h"
#include "avrcp_send_response.h"

#include <string.h>



/*****************************************************************************/
void sendPassThroughResponse(AVRCP *avrcp, avrcp_response_type response)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_PASSTHROUGH_RES);

#ifdef AVRCP_DEBUG_LIB	
	if ((response < avctp_response_not_implemented) || (response > avctp_response_bad_profile))
	{
		AVRCP_DEBUG(("Out of range response  0x%x\n", response));
	}
#endif

	message->response = response;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_PASSTHROUGH_RES, message);
}


/*****************************************************************************/
void sendUnitInfoResponse(AVRCP *avrcp, bool accept, avc_subunit_type unit_type, uint8 unit, uint32 company_id)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_UNITINFO_RES);

#ifdef AVRCP_DEBUG_LIB	
	if (unit_type > 0x1F)
	{
		AVRCP_DEBUG(("Out of range subunit type  0x%x\n", unit_type));
	}
    if (unit > 0x07)
	{
		AVRCP_DEBUG(("Out of range unit  0x%x\n", unit));
	}
    if (company_id > 0xFFFFFF)
	{
		AVRCP_DEBUG(("Out of range company id  0x%lx\n", company_id));
	}
#endif

	message->accept = accept;
	message->unit_type = unit_type;
	message->unit = unit;
	message->company_id = company_id;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_UNITINFO_RES, message);
}


/*****************************************************************************/
void sendSubunitInfoResponse(AVRCP *avrcp, bool accept, const uint8 *page_data)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SUBUNITINFO_RES);
	message->accept= accept;

	if (accept)
		memcpy(message->page_data, page_data, PAGE_DATA_LENGTH);
	else
		memset(message->page_data, 0, PAGE_DATA_LENGTH);

	MessageSend(&avrcp->task, AVRCP_INTERNAL_SUBUNITINFO_RES, message);
}


/*****************************************************************************/
void sendVendorDependentResponse(AVRCP *avrcp, avrcp_response_type response)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_VENDORDEPENDENT_RES);

#ifdef AVRCP_DEBUG_LIB	
	if ((response < avctp_response_not_implemented) || (response > avctp_response_bad_profile))
	{
		AVRCP_DEBUG(("Out of range response  0x%x\n", response));
	}
#endif

    message->response= response;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_VENDORDEPENDENT_RES, message);
}


/*****************************************************************************/
void avrcpSendInformBatteryResponse(AVRCP *avrcp, avrcp_response_type response)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_INFORM_BATTERY_STATUS_RES);   
	message->response = response;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_INFORM_BATTERY_STATUS_RES, message);
}


/*****************************************************************************/
void avrcpSendGetCapsResponse(AVRCP *avrcp, avrcp_response_type response, avrcp_capability_id caps, uint16 size_caps_list, Source caps_list)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_CAPS_RES);
	message->caps_id = caps;        
    message->response = response;        
    message->size_caps_list = size_caps_list;
    message->caps_list = caps_list;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_GET_CAPS_RES, message);
}


/*****************************************************************************/
void avrcpSendInformCharSetResponse(AVRCP *avrcp, avrcp_response_type response)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_INFORM_CHAR_SET_RES);   
	message->response = response;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_INFORM_CHAR_SET_RES, message);
}


/*****************************************************************************/
void sendGetElementsResponse(AVRCP *avrcp, avrcp_response_type response, uint16 number_of_attributes, uint16 size_attributes, Source attributes)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES);
	message->response = response;
	message->number_of_attributes = number_of_attributes;
	message->size_attributes_list = size_attributes;
    message->attributes_list = attributes;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES, message);
}


/*****************************************************************************/
void sendNextGroupResponse(AVRCP *avrcp, avrcp_response_type response)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_NEXT_GROUP_RES);   
	message->response = response;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_NEXT_GROUP_RES, message);
}


/*****************************************************************************/
void sendPreviousGroupResponse(AVRCP *avrcp, avrcp_response_type response)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_PREVIOUS_GROUP_RES);   
	message->response = response;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_PREVIOUS_GROUP_RES, message);
}


/*****************************************************************************/
void sendPlayStatusResponse(AVRCP *avrcp, avrcp_response_type response, uint32 song_length, uint32 song_elapsed, avrcp_play_status play_status)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_PLAY_STATUS_RES);
	message->response = response;
	message->song_length = song_length;
	message->song_elapsed = song_elapsed;
    message->play_status = play_status;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_GET_PLAY_STATUS_RES, message);
}


/*****************************************************************************/
void sendListAttrResponse(AVRCP *avrcp, avrcp_response_type response, uint16 size_attributes, Source attributes)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES);
	message->response = response;
	message->size_attributes_list = size_attributes;
    message->attributes_list = attributes;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES, message);
}


/*****************************************************************************/
void sendListValuesResponse(AVRCP *avrcp, avrcp_response_type response, uint16 size_values, Source values)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_LIST_APP_VALUE_RES);
	message->response = response;
	message->size_values_list = size_values;
    message->values_list = values;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_LIST_APP_VALUE_RES, message);
}


/*****************************************************************************/
void sendGetValuesResponse(AVRCP *avrcp, avrcp_response_type response, uint16 size_values, Source values)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_APP_VALUE_RES);
	message->response = response;
	message->size_values_list = size_values;
    message->values_list = values;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_GET_APP_VALUE_RES, message);
}


/*****************************************************************************/
void sendSetValuesResponse(AVRCP *avrcp, avrcp_response_type response)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SET_APP_VALUE_RES);
	message->response = response;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_SET_APP_VALUE_RES, message);
}


/*****************************************************************************/
void sendGetAttributeTextResponse(AVRCP *avrcp, avrcp_response_type response, uint16 number_of_attributes, uint16 size_attributes, Source attributes)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES);
	message->response = response;
	message->number_of_attributes = number_of_attributes;
	message->size_attributes_list = size_attributes;
	message->attributes_list = attributes;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES, message);
}


/*****************************************************************************/
void sendGetValueTextResponse(AVRCP *avrcp, avrcp_response_type response, uint16 number_of_values, uint16 size_values, Source values)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES);
	message->response = response;
	message->number_of_values = number_of_values;
	message->size_values_list = size_values;
	message->values_list = values;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES, message);
}
