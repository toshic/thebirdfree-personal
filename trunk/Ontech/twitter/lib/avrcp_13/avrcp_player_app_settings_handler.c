/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_player_app_settings_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_metadata_transfer.h"
#include "avrcp_common.h"
#include "avrcp_player_app_settings_handler.h"
#include "avrcp_private.h"
#include "avrcp_send_response.h"
#include "avrcp_signal_handler.h"

#include <source.h>
#include <stdlib.h>


/*****************************************************************************/
/***************************** LIST APP ATTRIBUTES ***************************/
/*****************************************************************************/

/*****************************************************************************/
void avrcpHandleListAppAttributesCommand(AVRCP *avrcp, uint16 transaction)
{
	if (!avrcpMetadataAppSettingsEnabled(avrcp))
	{
		sendListAttrResponse(avrcp, avctp_response_not_implemented, 0, 0);
	}
	else
	{
		/* We know this PDU cannot be fragmented. Pass the request directly to the client. */	
		avrcpSendCommonMetadataInd(avrcp, AVRCP_LIST_APP_ATTRIBUTE_IND, transaction);
	}
}


/*****************************************************************************/
bool avrcpHandleListAppAttributesResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	avrcp_response_type response;
	uint16 total_packets;
	uint16 data_offset;
	uint8 data[1];

	const uint8 *ptr = SourceMap(source);

	if ((avrcp->pending != avrcp_list_app_attributes) && (avrcp->continuation_pdu != AVRCP_LIST_APP_ATTRIBUTES_PDU_ID))
		return FALSE;

	if(!getResponseStatus(ctp_packet_type, ptr, &response))
		return FALSE;

	if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 1, ptr, &total_packets, &data_offset, &data[0]))
		return FALSE;

	avrcpSendCommonFragmentedMetadataCfm(avrcp, convertResponseToStatus(response), FALSE, AVRCP_LIST_APP_ATTRIBUTE_CFM, (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, total_packets, ctp_packet_type, meta_packet_type, data[0], packet_size-data_offset, data_offset, source);
	
	return TRUE;
}


/*****************************************************************************/
void avrcpHandleInternalListAppAttributesResponse(AVRCP *avrcp, AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES_T *res)
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
		mandatory_data[0] = res->size_attributes_list;
	}
	else
	{
		mandatory_data = insertRejectCode(&res->response, &size_mandatory_data, &param_length);
	}

	prepareMetadataStatusResponse(avrcp, res->response, AVRCP_LIST_APP_ATTRIBUTES_PDU_ID, param_length, res->attributes_list, size_mandatory_data, mandatory_data);
}


/*****************************************************************************/
/***************************** LIST APP VALUES *******************************/
/*****************************************************************************/

/*****************************************************************************/
void avrcpHandleListAppValuesCommand(AVRCP *avrcp, uint16 transaction, uint16 attribute_id)
{
	if (!avrcpMetadataAppSettingsEnabled(avrcp))
	{
		sendListValuesResponse(avrcp, avctp_response_not_implemented, 0, 0);
	}
	else
	{
		/* We know this PDU cannot be fragmented. Pass the request directly to the client. */
		MAKE_AVRCP_MESSAGE(AVRCP_LIST_APP_VALUE_IND);
		message->avrcp = avrcp;
		message->transaction = transaction;
		message->attribute_id = attribute_id;
		MessageSend(avrcp->clientTask, AVRCP_LIST_APP_VALUE_IND, message);
	}
}


/*****************************************************************************/
bool avrcpHandleListAppValuesResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	avrcp_response_type response;
	uint16 total_packets;
	uint16 data_offset;
	uint8 data[1];

	if ((avrcp->pending != avrcp_list_app_values) && (avrcp->continuation_pdu != AVRCP_LIST_APP_VALUE_PDU_ID))
		return FALSE;

	if(!getResponseStatus(ctp_packet_type, ptr, &response))
		return FALSE;

	if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 1, ptr,  &total_packets, &data_offset, &data[0]))
		return FALSE;

	avrcpSendCommonFragmentedMetadataCfm(avrcp, convertResponseToStatus(response), TRUE, AVRCP_LIST_APP_VALUE_CFM, (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, total_packets, ctp_packet_type, meta_packet_type, data[0], packet_size-data_offset, data_offset, source);

	return TRUE;
}


/*****************************************************************************/
void avrcpHandleInternalListAppValuesResponse(AVRCP *avrcp, AVRCP_INTERNAL_LIST_APP_VALUE_RES_T *res)
{
	uint16 size_mandatory_data = 0;
	uint8 *mandatory_data = 0;
	uint16 param_length = 0;

	standardiseCtypeStatus(&res->response);

	if (res->response == avctp_response_stable)
	{
		size_mandatory_data = 1;
		param_length = size_mandatory_data + res->size_values_list;

		/* Insert the mandatory data */
		mandatory_data = (uint8 *) malloc(size_mandatory_data);
		mandatory_data[0] = res->size_values_list;
	}
	else
	{
		mandatory_data = insertRejectCode(&res->response, &size_mandatory_data, &param_length);
	}

	prepareMetadataStatusResponse(avrcp, res->response, AVRCP_LIST_APP_VALUE_PDU_ID, param_length, res->values_list, size_mandatory_data, mandatory_data);
}


/*****************************************************************************/
/****************************** GET APP VALUES *******************************/
/*****************************************************************************/

/*****************************************************************************/
bool avrcpHandleGetAppValuesCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	uint16 total_packets;
	uint16 data_offset;
	uint8 data[1];

	if (!avrcpMetadataAppSettingsEnabled(avrcp))
	{
		sendGetValuesResponse(avrcp, avctp_response_not_implemented, 0, 0);

		avrcpSourceProcessed(avrcp);
	}
	else
	{
		if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 1, ptr, &total_packets, &data_offset, &data[0]))
			return FALSE;

		avrcpSendCommonFragmentedMetadataInd(avrcp, AVRCP_GET_APP_VALUE_IND, (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, total_packets, ctp_packet_type, meta_packet_type, data[0], packet_size - data_offset, data_offset, source);
	}

	return TRUE;
}


/*****************************************************************************/
bool avrcpHandleGetAppValuesResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	avrcp_response_type response;
	uint16 total_packets;
	uint16 data_offset;
	uint8 data[1];

	if ((avrcp->pending != avrcp_get_app_values) && (avrcp->continuation_pdu != AVRCP_GET_APP_VALUE_PDU_ID))
		return FALSE;

	if(!getResponseStatus(ctp_packet_type, ptr, &response))
		return FALSE;

	if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 1, ptr, &total_packets, &data_offset, &data[0]))
		return FALSE;

	avrcpSendCommonFragmentedMetadataCfm(avrcp, convertResponseToStatus(response), FALSE, AVRCP_GET_APP_VALUE_CFM, (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, total_packets, ctp_packet_type, meta_packet_type, data[0], packet_size-data_offset, data_offset, source);

	return TRUE;
}


/*****************************************************************************/
void avrcpHandleInternalGetAppValueResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_APP_VALUE_RES_T *res)
{
	uint16 size_mandatory_data = 0;
	uint8 *mandatory_data = 0;
	uint16 param_length = 0;

	standardiseCtypeStatus(&res->response);

	if (res->response == avctp_response_stable)
	{
		size_mandatory_data = 1;
		param_length = size_mandatory_data + res->size_values_list;
		/* Insert the mandatory data */
		mandatory_data = (uint8 *) malloc(size_mandatory_data);
		mandatory_data[0] = res->size_values_list/2;
	}
	else
	{
		mandatory_data = insertRejectCode(&res->response, &size_mandatory_data, &param_length);
	}

	prepareMetadataStatusResponse(avrcp, res->response, AVRCP_GET_APP_VALUE_PDU_ID, param_length, res->values_list, size_mandatory_data, mandatory_data);
}


/*****************************************************************************/
/****************************** SET APP VALUES *******************************/
/*****************************************************************************/

/*****************************************************************************/
bool avrcpHandleSetAppValuesCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	uint16 total_packets=0;
	uint16 data_offset=0;
	uint8 data[1];

	if (!avrcpMetadataAppSettingsEnabled(avrcp))
	{
		sendSetValuesResponse(avrcp, avctp_response_not_implemented);
	
		avrcpSourceProcessed(avrcp);
	}
	else
	{
		if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 1, ptr, &total_packets, &data_offset, &data[0]))
			return FALSE;

		avrcpSendCommonFragmentedMetadataInd(avrcp, AVRCP_SET_APP_VALUE_IND, (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, total_packets, ctp_packet_type, meta_packet_type, data[0], packet_size - data_offset, data_offset, source);
	}

	return TRUE;
}


/*****************************************************************************/
bool avrcpHandleSetAppValuesResponse(AVRCP *avrcp, avrcp_status_code status, uint16 transaction, Source source, uint16 packet_size)
{
	if ((avrcp->pending == avrcp_set_app_values) || (avrcp->continuation_pdu == AVRCP_SET_APP_VALUE_PDU_ID))
	{
		avrcpSendCommonMetadataCfm(avrcp, status, transaction, AVRCP_SET_APP_VALUE_CFM, source, packet_size);
		return TRUE;
	}

	return FALSE;
}


/*****************************************************************************/
void avrcpHandleInternalSetAppValueResponse(AVRCP *avrcp, AVRCP_INTERNAL_SET_APP_VALUE_RES_T *res)
{
	uint16 size_mandatory_data = 0;
	uint8 *mandatory_data = 0;
	uint16 param_length = 0;

	standardiseCtypeControl(&res->response);

	if (res->response != avctp_response_accepted)
	{
		mandatory_data = insertRejectCode(&res->response, &size_mandatory_data, &param_length);
	}

	sendMetadataResponse(avrcp,  res->response, AVRCP_SET_APP_VALUE_PDU_ID, 0, avrcp_packet_type_single, param_length, size_mandatory_data, mandatory_data);

    avrcpHandleReceivedData(avrcp);
}


/*****************************************************************************/
/*************************** GET APP ATTRIBUTE TEXT **************************/
/*****************************************************************************/

/*****************************************************************************/
bool avrcpHandleGetAppAttributeTextCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	uint16 total_packets=0;
	uint16 data_offset=0;
	uint8 data[1];

	if (!avrcpMetadataAppSettingsEnabled(avrcp))
	{
		sendGetAttributeTextResponse(avrcp, avctp_response_not_implemented, 0, 0, 0);

		avrcpSourceProcessed(avrcp);
	}
	else
	{
		if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 1, ptr, &total_packets, &data_offset, &data[0]))
			return FALSE;

		avrcpSendCommonFragmentedMetadataInd(avrcp, AVRCP_GET_APP_ATTRIBUTE_TEXT_IND, (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, total_packets, ctp_packet_type, meta_packet_type, data[0], packet_size - data_offset, data_offset, source);
	}

	return TRUE;
}


/*****************************************************************************/
bool avrcpHandleGetAppAttributeTextResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	avrcp_response_type response;
	uint16 total_packets=0;
	uint16 data_offset=0;
	uint8 data[1];

	if ((avrcp->pending != avrcp_get_app_attribute_text) && (avrcp->continuation_pdu != AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID))
		return FALSE;

	if(!getResponseStatus(ctp_packet_type, ptr, &response))
		return FALSE;

	if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 1, ptr, &total_packets, &data_offset, &data[0]))
		return FALSE;

	avrcpSendCommonFragmentedMetadataCfm(avrcp, convertResponseToStatus(response), FALSE, AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM, (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, total_packets, ctp_packet_type, meta_packet_type, data[0], packet_size-data_offset, data_offset, source);

	return TRUE;

}


/*****************************************************************************/
void avrcpHandleInternalGetAppAttributeTextResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES_T *res)
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

	prepareMetadataStatusResponse(avrcp, res->response, AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID, param_length, res->attributes_list, size_mandatory_data, mandatory_data);
}


/*****************************************************************************/
/*************************** GET APP VALUE TEXT **************************/
/*****************************************************************************/

/*****************************************************************************/
bool avrcpHandleGetAppValueTextCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	uint16 total_packets=0;
	uint16 data_offset=0;
	uint8 data[2];

	if (!avrcpMetadataAppSettingsEnabled(avrcp))
	{
		sendGetValueTextResponse(avrcp, avctp_response_not_implemented, 0, 0, 0);
		
		avrcpSourceProcessed(avrcp);
	}
	else
	{
		if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 2, ptr, &total_packets, &data_offset, &data[0]))
			return FALSE;
	}

	{
		MAKE_AVRCP_MESSAGE(AVRCP_GET_APP_VALUE_TEXT_IND);

		message->avrcp = avrcp;
		message->transaction = (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F;
		message->no_packets = total_packets;
		message->ctp_packet_type = ctp_packet_type;
		message->metadata_packet_type = meta_packet_type;
		message->attribute_id = data[0];
		message->number_of_attributes = data[1];
		message->data_offset = data_offset;
		message->size_attributes = packet_size - data_offset;
		message->attributes = source;

		MessageSend(avrcp->clientTask, AVRCP_GET_APP_VALUE_TEXT_IND, message);
	}	

	return TRUE;
}


/*****************************************************************************/
bool avrcpHandleGetAppValueTextResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	avrcp_response_type response;
	uint16 total_packets=0;
	uint16 data_offset=0;
	uint8 data[1];

	if ((avrcp->pending != avrcp_get_app_value_text) && (avrcp->continuation_pdu != AVRCP_GET_APP_VALUE_TEXT_PDU_ID))
		return FALSE;

	if(!getResponseStatus(ctp_packet_type, ptr, &response))
		return FALSE;

	if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 1, ptr, &total_packets, &data_offset, &data[0]))
		return FALSE;

	avrcpSendCommonFragmentedMetadataCfm(avrcp, convertResponseToStatus(response), FALSE, AVRCP_GET_APP_VALUE_TEXT_CFM, (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, total_packets, ctp_packet_type, meta_packet_type, data[0], packet_size-data_offset, data_offset, source);

	return TRUE;
}


/*****************************************************************************/
void avrcpHandleInternalGetAppValueTextResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES_T *res)
{
    uint16 size_mandatory_data = 0;
	uint8 *mandatory_data = 0;
	uint16 param_length = 0;

	standardiseCtypeStatus(&res->response);

	if (res->response == avctp_response_stable)
	{
		size_mandatory_data = 1;
		param_length = size_mandatory_data + res->size_values_list;
		/* Insert the mandatory data */
		mandatory_data = (uint8 *) malloc(size_mandatory_data);
		mandatory_data[0] = res->number_of_values;
	}
	else
	{
		mandatory_data = insertRejectCode(&res->response, &size_mandatory_data, &param_length);
	}

	prepareMetadataStatusResponse(avrcp, res->response, AVRCP_GET_APP_VALUE_TEXT_PDU_ID, param_length, res->values_list, size_mandatory_data, mandatory_data);
}
