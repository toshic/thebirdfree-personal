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
#include "avrcp_metadata_transfer.h"
#include "avrcp_play_status_handler.h"
#include "avrcp_private.h"
#include "avrcp_signal_handler.h"

#include <stdlib.h>


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void avrcpSendGetPlayStatusCfm(AVRCP *avrcp, avrcp_status_code status, uint32 song_length, uint32 song_elapsed, avrcp_play_status play_status, uint8 transaction, Source source, uint16 packet_size)
{
    MAKE_AVRCP_MESSAGE(AVRCP_GET_PLAY_STATUS_CFM);

    message->avrcp = avrcp;
	message->transaction = transaction;
    message->status = status;
	message->song_length = song_length;
	message->song_elapsed = song_elapsed;
	message->play_status = play_status;

	if ((status != avrcp_success) && (packet_size >= 14))
		message->status = getRejectCode(avrcp, packet_size - 13, status, source);

	MessageSend(avrcp->clientTask, AVRCP_GET_PLAY_STATUS_CFM, message);
}


/*****************************************************************************/
bool avrcpHandleGetPlayStatusResponse(AVRCP *avrcp, uint16 transaction, const uint8 *ptr, Source source, uint16 packet_size)
{
	uint32 song_length = 0, song_elapsed = 0;

	/* Only process the response if it is expected. */
	if ((avrcp->pending == avrcp_get_play_status) || (avrcp->continuation_pdu == AVRCP_GET_PLAY_STATUS_PDU_ID))
	{
		song_length = convertUint8ValuesToUint32(&ptr[13]);
		song_elapsed = convertUint8ValuesToUint32(&ptr[17]);
		avrcpSendGetPlayStatusCfm(avrcp, convertResponseToStatus(ptr[3]), song_length, song_elapsed, ptr[21], transaction, source, packet_size);
	}
	else
		return FALSE;

	return TRUE;
}
#endif

/*****************************************************************************/
void avrcpHandleInternalGetPlayStatusResponse(AVRCP *avrcp, AVRCP_INTERNAL_GET_PLAY_STATUS_RES_T *res)
{
	uint16 size_mandatory_data = 0;
	uint8 *mandatory_data = 0;
	uint16 param_length = 0;

	standardiseCtypeStatus(&res->response);

	if (res->response == avctp_response_stable)
	{
		size_mandatory_data = 9;
		param_length = size_mandatory_data;
		/* Insert the mandatory data */
		mandatory_data = (uint8 *) malloc(size_mandatory_data);
		convertUint32ToUint8Values(&mandatory_data[0], res->song_length);
		convertUint32ToUint8Values(&mandatory_data[4], res->song_elapsed);
		mandatory_data[8] = res->play_status;
	}
	else
	{
		mandatory_data = insertRejectCode(&res->response, &size_mandatory_data, &param_length);
	}

    sendMetadataResponse(avrcp,  res->response, AVRCP_GET_PLAY_STATUS_PDU_ID, 0, avrcp_packet_type_single, param_length, size_mandatory_data, mandatory_data);

    avrcpHandleReceivedData(avrcp);
}
