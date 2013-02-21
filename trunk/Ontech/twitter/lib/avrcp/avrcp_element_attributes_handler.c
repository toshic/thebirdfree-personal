/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_element_attributes_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_common.h"
#include "avrcp_continuation_handler.h"
#include "avrcp_element_attributes_handler.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_private.h"

#include <source.h>
#include <stdlib.h>


/*****************************************************************************/
bool avrcpHandleGetElementAttributesCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	uint16 total_packets;
	uint16 data_offset;
	uint8 data[9];
	uint32 identifier_high = 0, identifier_low = 0;


	if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 9, ptr, &total_packets, &data_offset, &data[0]))
		return FALSE;

	identifier_high = convertUint8ValuesToUint32(&data[0]);
	identifier_low = convertUint8ValuesToUint32(&data[4]);

	{
		MAKE_AVRCP_MESSAGE(AVRCP_GET_ELEMENT_ATTRIBUTES_IND);

		message->avrcp = avrcp;
		message->transaction = (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F;
		message->no_packets = total_packets;
		message->ctp_packet_type = ctp_packet_type;
		message->metadata_packet_type = meta_packet_type;
		message->identifier_high = identifier_high;
		message->identifier_low = identifier_low;
		message->number_of_attributes = data[8];
		message->data_offset = data_offset;
		message->size_attributes = packet_size - data_offset;
		message->attributes = source;

		MessageSend(avrcp->clientTask, AVRCP_GET_ELEMENT_ATTRIBUTES_IND, message);
	}

	return TRUE;
}


/*****************************************************************************/
bool avrcpHandleGetElementAttributesResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	avrcp_response_type response;
	uint16 total_packets;
	uint16 data_offset;
	uint8 data[1];

	/* Only process the response if it was expected. */
	if ((avrcp->pending != avrcp_get_element_attributes) && (avrcp->continuation_pdu != AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID))
		return FALSE;

	if(!getResponseStatus(ctp_packet_type, ptr, &response))
		return FALSE;

	if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 1, ptr, &total_packets, &data_offset, &data[0]))
		return FALSE;

	avrcpSendCommonFragmentedMetadataCfm(avrcp, convertResponseToStatus(response), FALSE, AVRCP_GET_ELEMENT_ATTRIBUTES_CFM, (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, total_packets, ctp_packet_type, meta_packet_type, data[0], packet_size-data_offset, data_offset, source);

	return TRUE;
}


/*****************************************************************************/
void avrcpHandleInternalGetElementAttributesResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES_T *res)
{
	uint16 size_mandatory_data = 0;
	uint8 *mandatory_data = 0;
	uint16 param_length = 0;

	standardiseCtypeStatus(&res->response);

	if (res->response == avctp_response_stable)
	{
		size_mandatory_data = 1;
		param_length = size_mandatory_data + res->size_attributes_list;
		/* Insert the mandatory data */
		mandatory_data = (uint8 *) malloc(size_mandatory_data);
		mandatory_data[0] = res->number_of_attributes;
	}
	else
	{
		mandatory_data = insertRejectCode(&res->response, &size_mandatory_data, &param_length);
	}

	prepareMetadataStatusResponse(avrcp, res->response, AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID, param_length, res->attributes_list, size_mandatory_data, mandatory_data);
}
