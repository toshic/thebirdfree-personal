/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_get_app_attr_text.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_common.h"
#include "avrcp_metadata_command_req.h"
#include "avrcp_send_response.h"
#include "avrcp_metadata_control_command_req.h"



#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void AvrcpListAppSettingAttribute(AVRCP *avrcp)
{
	avrcp_status_code status = avrcpMetadataStatusCommand(avrcp, AVRCP_LIST_APP_ATTRIBUTES_PDU_ID, avrcp_list_app_attributes, 0, 0, 0, 0, 0);

	if (status != avrcp_success)
	{
		avrcpSendCommonFragmentedMetadataCfm(avrcp, status, FALSE, AVRCP_LIST_APP_ATTRIBUTE_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
	}
}


/*****************************************************************************/
void AvrcpListAppSettingValue(AVRCP *avrcp, uint16 attribute_id)
{
	uint8 status_params[] = {0};
	avrcp_status_code status;

	status_params[0] = attribute_id;

	status = avrcpMetadataStatusCommand(avrcp, AVRCP_LIST_APP_VALUE_PDU_ID, avrcp_list_app_values, sizeof(status_params), status_params, 0, 0, 0);

	if (status != avrcp_success)
	{
		avrcpSendCommonFragmentedMetadataCfm(avrcp, status, FALSE, AVRCP_LIST_APP_VALUE_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
	}
}


/*****************************************************************************/
void AvrcpGetCurrentAppSettingValue(AVRCP *avrcp, uint16 size_attributes, Source attributes)
{
	uint8 extra_params[2];
	avrcp_status_code status;

	extra_params[0] = METADATA_HEADER_SIZE+1;
	extra_params[1] = size_attributes;

	status = avrcpMetadataStatusCommand(avrcp, AVRCP_GET_APP_VALUE_PDU_ID, avrcp_get_app_values, size_attributes, 0, 2, extra_params, attributes);

	if (status != avrcp_success)
	{
		avrcpSendCommonFragmentedMetadataCfm(avrcp, status, FALSE, AVRCP_GET_APP_VALUE_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
	}
}

/*****************************************************************************/
void AvrcpSetAppSettingValue(AVRCP *avrcp, uint16 size_attributes, Source attributes)
{
	avrcp_status_code status;

	/* Size of Attributes should be multiple of 2 contains teh ID & Value combination */
	if(size_attributes % 2)
	{
		avrcpSendCommonMetadataCfm(avrcp, avrcp_rejected_invalid_param, 0, AVRCP_SET_APP_VALUE_CFM, 0, 0);
	}
	status = avrcpMetadataControlCommand(avrcp, AVRCP_SET_APP_VALUE_PDU_ID, avrcp_set_app_values, size_attributes, attributes);

	if (status != avrcp_success)
	{
		avrcpSendCommonMetadataCfm(avrcp, status, 0, AVRCP_SET_APP_VALUE_CFM, 0, 0);
	}
}

void AvrcpGetAppSettingAttributeText(AVRCP *avrcp, uint16 size_attributes, Source attributes)
{
	uint8 extra_params[2];
	avrcp_status_code status;

	extra_params[0] = METADATA_HEADER_SIZE+1;
	extra_params[1] = size_attributes;

	status = avrcpMetadataStatusCommand(avrcp, AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID, avrcp_get_app_attribute_text, size_attributes, 0, 2, extra_params, attributes);

	if (status != avrcp_success)
	{
		avrcpSendCommonFragmentedMetadataCfm(avrcp, status, FALSE, AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
	}
}

/*****************************************************************************/
void AvrcpGetAppSettingValueText(AVRCP *avrcp, uint16 attribute_id, uint16 size_values, Source values)
{
	uint8 extra_params[3];
	avrcp_status_code status;

	extra_params[0] = METADATA_HEADER_SIZE+2;
	extra_params[1] = attribute_id;
	extra_params[2] = size_values;

	status = avrcpMetadataStatusCommand(avrcp, AVRCP_GET_APP_VALUE_TEXT_PDU_ID, avrcp_get_app_value_text, size_values, 0, 3, extra_params, values);

	if (status != avrcp_success)
	{
		avrcpSendCommonFragmentedMetadataCfm(avrcp, status, FALSE, AVRCP_GET_APP_VALUE_TEXT_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
	}
}

#endif

/*****************************************************************************/
void AvrcpListAppSettingAttributeResponse(AVRCP *avrcp, avrcp_response_type response, uint16 size_attributes, Source attributes)
{
	/* Send the response only if the command arrived to start with. */
	if (avrcp->block_received_data == avrcp_list_app_attributes)
    {
		sendListAttrResponse(avrcp, response, size_attributes, attributes);
	}
	else
		PRINT(("AvrcpListAppSettingAttributeResponse: CT not waiting for response\n"));
}



/*****************************************************************************/
void AvrcpListAppSettingValueResponse(AVRCP *avrcp, avrcp_response_type response, uint16 size_values, Source values)
{
	/* Send the response only if the command arrived to start with. */
	if (avrcp->block_received_data == avrcp_list_app_values)
    {
		sendListValuesResponse(avrcp, response, size_values, values);
	}
	else
		PRINT(("AvrcpListAppSettingValueResponse: CT not waiting for response\n"));
}

/*****************************************************************************/
void AvrcpGetCurrentAppSettingValueResponse(AVRCP *avrcp, avrcp_response_type response, uint16 size_values, Source values)
{
	/* Send the response only if the command arrived to start with. */
	if (avrcp->block_received_data == avrcp_get_app_values)
    {
		sendGetValuesResponse(avrcp, response, size_values, values);
	}
	else
		PRINT(("AvrcpGetCurrentAppSettingValueResponse: CT not waiting for response\n"));
}

/*****************************************************************************/
void AvrcpSetAppSettingValueResponse(AVRCP *avrcp, avrcp_response_type response)
{
	/* Send the response only if the command arrived to start with. */
	if (avrcp->block_received_data == avrcp_set_app_values)
    {
		sendSetValuesResponse(avrcp, response);
	}
	else
		PRINT(("AvrcpSetAppSettingValueResponse: CT not waiting for response\n"));
}



/*****************************************************************************/
void AvrcpGetAppSettingAttributeTextResponse(AVRCP *avrcp, avrcp_response_type response, uint16 number_of_attributes, uint16 size_attributes, Source attributes)
{
	/* Send the response only if the command arrived to start with. */
	if (avrcp->block_received_data == avrcp_get_app_attribute_text)
    {
		sendGetAttributeTextResponse(avrcp, response, number_of_attributes, size_attributes, attributes);
	}
	else
		PRINT(("AvrcpGetAppSettingAttributeTextResponse: CT not waiting for response\n"));
}



/*****************************************************************************/
void AvrcpGetAppSettingValueTextResponse(AVRCP *avrcp, avrcp_response_type response, uint16 number_of_values, uint16 size_values, Source values)
{
	/* Send the response only if the command arrived to start with. */
	if (avrcp->block_received_data == avrcp_get_app_attribute_text)
    {
		sendGetValueTextResponse(avrcp, response, number_of_values, size_values, values);
	}
	else
		PRINT(("AvrcpGetAppSettingValueResponse: CT not waiting for response\n"));
}
