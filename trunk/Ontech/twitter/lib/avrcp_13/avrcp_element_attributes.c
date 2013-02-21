/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_element_attributes.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_common.h"
#include "avrcp_element_attributes_handler.h"
#include "avrcp_metadata_command_req.h"
#include "avrcp_private.h"


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void AvrcpGetElementAttributes(AVRCP *avrcp, uint32 identifier_high, uint32 identifier_low, uint16 size_attributes, Source attributes)
{
	uint8 extra_params[10];
	avrcp_status_code status;

	extra_params[0] = METADATA_HEADER_SIZE + 9;

	convertUint32ToUint8Values(&extra_params[1], identifier_high);
	convertUint32ToUint8Values(&extra_params[5], identifier_low);

	extra_params[9] = size_attributes/4;

	status = avrcpMetadataStatusCommand(avrcp, AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID, avrcp_get_element_attributes, size_attributes, 0, 10, extra_params, attributes);

	if (status != avrcp_success)
	{
		avrcpSendCommonFragmentedMetadataCfm(avrcp, status, FALSE, AVRCP_GET_ELEMENT_ATTRIBUTES_CFM, 0, 1, 0, 0, 0, 0, 0, 0);
	}
}
#endif

/*****************************************************************************/
void AvrcpGetElementAttributesResponse(AVRCP *avrcp, avrcp_response_type response, uint16 number_of_attributes, uint16 size_attributes, Source attributes)
{
	/* Only allow a response to be sent if the corresponding command arrived. */
	if (avrcp->block_received_data == avrcp_get_element_attributes)
    {
		sendGetElementsResponse(avrcp, response, number_of_attributes, size_attributes, attributes);	
	}
	else
		PRINT(("AvrcpGetElementAttributesResponse: CT not waiting for response\n"));
}
