/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_caps_handler.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_caps_handler.h"
#include "avrcp_common.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_private.h"
#include "avrcp_signal_handler.h"

#include <source.h>
#include <stdlib.h>


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void avrcpSendGetCapsCfm(AVRCP *avrcp, avrcp_status_code status, bool response, uint16 transaction, uint16 total_packets, uint16 ctp_packet_type, uint16 metadata_packet_type, avrcp_capability_id caps_id, uint16 number_of_caps, uint16 data_length, uint16 data_offset, Source source)
{
	/* Send successful confirmation message up to app. */
    MAKE_AVRCP_MESSAGE(AVRCP_GET_CAPS_CFM);

    message->avrcp = avrcp;    
    message->status = status;
    message->transaction = transaction;
    message->no_packets = total_packets;
    message->ctp_packet_type = ctp_packet_type;
    message->metadata_packet_type = metadata_packet_type;
    message->caps = caps_id;
    message->number_of_caps = number_of_caps;
    
    if ((status == avrcp_success) && data_length)
    {
		message->data_offset = data_offset;
        message->size_caps_list = data_length;
        message->caps_list = source;
    }
    else
    {
		message->data_offset = 0;
        message->size_caps_list = 0;
        message->caps_list = 0;

		if (response)
			message->status = getRejectCode(avrcp, data_length, status, source);
    }

	MessageSend(avrcp->clientTask, AVRCP_GET_CAPS_CFM, message);
}
#endif

/*****************************************************************************/
void avrcpHandleInternalGetCapsResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_CAPS_RES_T *res)
{
	uint16 size_mandatory_caps = 3; /* mandatory caps size */
	uint16 size_mandatory_data = 0;
	uint8 *mandatory_data = 0;
	uint16 param_length = 0;

	standardiseCtypeStatus(&res->response);

	if (res->response == avctp_response_stable)
	{
		if (res->caps_id == avrcp_capability_event_supported)
			size_mandatory_caps = 2;

		/* cap id (1) + cap len (1) + mandatory caps */
		size_mandatory_data = 2 + size_mandatory_caps;

		/* param_length = cap id (1) + cap len (1) + mandatory caps + supplied caps (size_caps_list) */
		param_length = size_mandatory_data + res->size_caps_list;
		
		/* Insert the mandatory ids */
		mandatory_data = (uint8 *) malloc(size_mandatory_data);
		mandatory_data[0] = res->caps_id;

		/* Insert the mandatory company ID data if this is the capability being returned. */
		if (res->caps_id == avrcp_capability_company_id)
		{
			mandatory_data[1] = (res->size_caps_list/3) + 1;
			mandatory_data[2] = (AVRCP_BT_COMPANY_ID >> 16) & 0xff;
			mandatory_data[3] = (AVRCP_BT_COMPANY_ID >> 8) & 0xff;
			mandatory_data[4] = AVRCP_BT_COMPANY_ID & 0xff;
		}
		/* Insert the mandatory events supported data if this is the capability being returned. */
		else if (res->caps_id == avrcp_capability_event_supported)
		{
			mandatory_data[1] = size_mandatory_caps + res->size_caps_list;
			mandatory_data[2] = avrcp_event_playback_status_changed;
			mandatory_data[3] = avrcp_event_track_changed;
		}
		else
		{
			/* Unsupported capability so don't send a response (Panic if debug version of the lib). */
			AVRCP_DEBUG(("avrcpHandleInternalGetCapsResponse detects invalid caps_id\n"));
			return;
		}
	}
	else
	{
		mandatory_data = insertRejectCode(&res->response, &size_mandatory_data, &param_length);
	}

	prepareMetadataStatusResponse(avrcp, res->response, AVRCP_GET_CAPS_PDU_ID, param_length, res->caps_list, size_mandatory_data, mandatory_data);
}


/*****************************************************************************/
void avrcpHandleGetCapsCommand(AVRCP *avrcp, uint16 transaction, avrcp_capability_id caps)
{
	/* Send an indication up to the app that this PDU command has arrived. */
    MAKE_AVRCP_MESSAGE(AVRCP_GET_CAPS_IND);
    message->avrcp = avrcp;
	message->transaction = transaction;
    message->caps = caps;
    MessageSend(avrcp->clientTask, AVRCP_GET_CAPS_IND, message);
}


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
bool avrcpHandleGetCapsResponse(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);

	avrcp_response_type response;
	uint16 total_packets;
	uint16 data_offset;
	uint8 data[2];

	/*	
		Process the response PDU. Just ignore this response if it was not a pending event (ie. check the command
		was sent in the first place).
	*/
	if ((avrcp->pending != avrcp_get_caps) && (avrcp->continuation_pdu != AVRCP_GET_CAPS_PDU_ID))
		return FALSE;

	if(!getResponseStatus(ctp_packet_type, ptr, &response))
		return FALSE;

	if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 2, ptr, &total_packets, &data_offset, &data[0]))
		return FALSE;

	avrcpSendGetCapsCfm(avrcp, convertResponseToStatus(response) , TRUE, (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, total_packets, ctp_packet_type, meta_packet_type, data[0], data[1], packet_size-data_offset, data_offset, source);

	return TRUE;
}
#endif
