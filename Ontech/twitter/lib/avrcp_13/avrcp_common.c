/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_common.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_common.h"

#include <panic.h>


static uint8 next_transaction(uint8 current_transaction)
{
	/*
    	Returns the next transaction label. This is used to route responses 
        from the remote device to the correct local initiator. Note that 
        zero will never be issued and hence can be used as a special case to 
        indicate no command is pending.
    */
	uint8 new_transaction = (current_transaction + 1) & 0xf;

	/* 
		We treat a label of zero as a special case indicating there is no 
		transaction pending. 
	*/
	if (!new_transaction) 
		new_transaction = 1;

	return new_transaction;
}


/*****************************************************************************/
void avrcpSetState(AVRCP *avrcp, avrcpState state)
{
	avrcp->state = state;
}


/****************************************************************************
NAME	
	avrcpSendCommonCfmMessageToApp

DESCRIPTION
	Create a common cfm message (many messages sent to the app
	have the form of the message below and a common function can be used to
	allocate them). Send the message not forgetting to set the correct 
	message id.

RETURNS
	void
*/
void avrcpSendCommonCfmMessageToApp(uint16 message_id, avrcp_status_code status, Sink sink, AVRCP *avrcp)
{
	MAKE_AVRCP_MESSAGE(AVRCP_COMMON_CFM_MESSAGE);
	message->status = status;
	message->sink = sink;
	message->avrcp = avrcp;
	MessageSend(avrcp->clientTask, message_id, message);

    if ((message_id == AVRCP_CONNECT_CFM) && (status != avrcp_success) && avrcp->lazy)
        MessageSend(&avrcp->task, AVRCP_INTERNAL_TASK_DELETE_REQ, 0);
}


/*****************************************************************************/
avrcp_status_code convertResponseToStatus(avrcp_response_type resp)
{
	/* Turn a response into a simplified state that the AVRCP library can return to the application. */
	switch (resp)
	{
	case avctp_response_accepted:
	case avctp_response_in_transition:
	case avctp_response_stable:
    case avctp_response_changed:
		return avrcp_success;
	case avctp_response_not_implemented:
	case avctp_response_bad_profile:
		return avrcp_unsupported;
	case avctp_response_interim:
		return avrcp_interim_success;
	case avctp_response_rejected:
		return avrcp_rejected;
	default:
		return avrcp_fail;
	}	
}


/*****************************************************************************/
void avrcpHandleDeleteTask(AVRCP *avrcp)
{
    /* Discard all messages for the task and free it. */
    (void) MessageFlushTask(&avrcp->task);
    free(avrcp);
}


/*****************************************************************************/
uint8 avrcpGetNextTransactionLabel(AVRCP *avrcp)
{
	/* Increment the transaction label. */
	avrcp->cmd_transaction_label = next_transaction(avrcp->cmd_transaction_label);

	return avrcp->cmd_transaction_label;
}


/*****************************************************************************/
uint8 avrcpFindNextTransactionLabel(AVRCP *avrcp)
{
	/* Find out what the next transaction label is. */
	return next_transaction(avrcp->cmd_transaction_label);
}


/*****************************************************************************/
uint32 avrcpGetCompanyId(const uint8 *ptr, uint16 offset)
{
    uint32 cid = 0;
    uint32 cid_tmp = 0;

    cid_tmp = (uint32) ptr[offset];
    cid = (cid_tmp << 16);
    cid_tmp = (uint32) ptr[offset+1];
    cid |= (cid_tmp << 8);
    cid_tmp = (uint32) ptr[offset+2];
    cid |= cid_tmp;

    return cid;
}


/*****************************************************************************/
uint8 *avrcpGrabSink(Sink sink, uint16 size)
{
	uint8 *dest = SinkMap(sink);
	uint16 claim_result = SinkClaim(sink, size);
    if (claim_result == 0xffff)
    {
		AVRCP_DEBUG(("SinkClaim return Invalid Offset"));
        return NULL;
	}

    return (dest + claim_result);
}


/*****************************************************************************/
void avrcpSetAvctpHeader(AVRCP *avrcp, uint8 *ptr, uint8 packet_type, uint8 total_packets)
{
	/* AVCTP header */
	ptr[0] = (avrcpGetNextTransactionLabel(avrcp) << AVCTP0_TRANSACTION_SHIFT) | packet_type | AVCTP0_CR_COMMAND;
	if (packet_type == AVCTP0_PACKET_TYPE_SINGLE)
	{
		ptr[1] = AVCTP1_PROFILE_AVRCP_HIGH;
		ptr[2] = AVCTP2_PROFILE_AVRCP_REMOTECONTROL;
	}
	else if (packet_type == AVCTP0_PACKET_TYPE_START)
	{
		ptr[1] = total_packets;
		ptr[2] = AVCTP1_PROFILE_AVRCP_HIGH;
		ptr[3] = AVCTP2_PROFILE_AVRCP_REMOTECONTROL;
	}
}


/*****************************************************************************/
void avrcpSourceProcessed(AVRCP *avrcp)
{
    Source source = StreamSourceFromSink(avrcp->sink);

	if (avrcp->srcUsed)
	{
		SourceDrop(source, avrcp->srcUsed);
		avrcp->srcUsed = 0;
	}

    if(SourceBoundary(source))
    {
        /* There is data in the L2CAP sink to be processed.
        Message Internally to trigger*/
       MessageSend(&avrcp->task, AVRCP_INTERNAL_MESSAGE_MORE_DATA, 0);
    }
}


/*****************************************************************************/
bool avrcpMetadataEnabled(AVRCP *avrcp)
{
	return (avrcp->local_extensions & AVRCP_EXTENSION_METADATA);
}


/*****************************************************************************/
bool avrcpMetadataAppSettingsEnabled(AVRCP *avrcp)
{
	return (avrcp->local_target_features & AVRCP_PLAYER_APPLICATION_SETTINGS);
}


/*****************************************************************************/
avrcp_status_code getRejectCode(AVRCP *avrcp, uint16 data_length, avrcp_status_code status, Source source)
{
	avrcp_status_code return_code = status;

	if (data_length)
	{
		if ((status == avrcp_rejected) && (data_length >= 1))
		{
			const uint8 *ptr = SourceMap(source);

			if (ptr[0] == 0)
				return_code = avrcp_rejected_invalid_pdu;
			else if (ptr[0] == 1)
				return_code = avrcp_rejected_invalid_param;
			else if (ptr[0] == 2)
				return_code = avrcp_rejected_invalid_content;
			else if (ptr[0] == 3)
				return_code = avrcp_rejected_internal_error;
		}
		avrcpSourceProcessed(avrcp);
	}

	return return_code;
}


/*****************************************************************************/
uint8 *insertRejectCode(avrcp_response_type *response, uint16 *size_mandatory_data, uint16 *param_length)
{
	uint8 *data = 0;

	*size_mandatory_data = 1;
	*param_length = *size_mandatory_data;
	data = (uint8 *) malloc(*size_mandatory_data);
	/*  avrcp_response_rejected_internal_error for all */
	data[0] = 3;

	if ((*response >= avrcp_response_rejected_invalid_pdu) && (*response <= avrcp_response_rejected_internal_error))
	{
		if (*response == avrcp_response_rejected_invalid_pdu)
			data[0] = 0;
		else if (*response == avrcp_response_rejected_invalid_param)
			data[0] = 1;
		else if (*response == avrcp_response_rejected_invalid_content)
			data[0] = 2;
	}
	*response = avctp_response_rejected;

	return data;
}


/*****************************************************************************/
void standardiseCtypeStatus(avrcp_response_type *response)
{
	switch (*response)
	{
	case avctp_response_accepted:	
	case avctp_response_changed:
	case avctp_response_interim:
		*response = avctp_response_stable;
		break;
	case avctp_response_in_transition:
	case avctp_response_bad_profile:
		*response = avrcp_response_rejected_internal_error;
		break;
	default:
		break;
	}
}


/*****************************************************************************/
void standardiseCtypeControl(avrcp_response_type *response)
{
	switch (*response)
	{
	case avctp_response_stable:
	case avctp_response_changed:
	case avctp_response_interim:
		*response = avctp_response_accepted;
		break;
	case avctp_response_in_transition:
	case avctp_response_bad_profile:
		*response = avrcp_response_rejected_internal_error;
		break;
	default:
		break;
	}
}


/*****************************************************************************/
void standardiseCtypeNotify(avrcp_response_type *response)
{
	switch (*response)
	{
	case avctp_response_accepted:	
	case avctp_response_stable:
		*response = avctp_response_changed;
		break;
	case avctp_response_in_transition:
	case avctp_response_bad_profile:
		*response = avrcp_response_rejected_internal_error;
		break;
	default:
		break;
	}
}


/*****************************************************************************/
uint32 convertUint8ValuesToUint32(const uint8 *ptr)
{
	return ((((uint32)ptr[0] << 24) & 0xFF000000) | (((uint32)ptr[1] << 16) & 0x00FF0000) | (((uint32)ptr[2] << 8) & 0x0000FF00) | ((uint32)ptr[3] & 0x000000FF));
}


/*****************************************************************************/
void convertUint32ToUint8Values(uint8 *ptr, uint32 value)
{
	ptr[0] = (value >> 24) & 0xFF;
	ptr[1] = (value >> 16) & 0xFF;
	ptr[2] = (value >> 8) & 0xFF;
	ptr[3] = value & 0xFF;
}

bool getResponseStatus(uint16 ctp_packet_type, const uint8 *ptr, avrcp_response_type *response)
{
	uint16 offset;

	switch(ctp_packet_type)
	{
	case AVCTP0_PACKET_TYPE_SINGLE:
		offset = 3;
		break;
	case AVCTP0_PACKET_TYPE_START:
		offset = 4;
		break;
	case AVCTP0_PACKET_TYPE_CONTINUE:
	case AVCTP0_PACKET_TYPE_END: 
		*response = avctp_response_in_transition;
		return TRUE;
		break;
	default:
		return FALSE;
	}

	*response = ptr[offset];
	return TRUE;
}


/*****************************************************************************/
bool getMessageDataFromPacket(uint16 ctp_packet_type, uint16 meta_packet_type, uint16 size_data, const uint8 *ptr, uint16 *total_packets, uint16 *data_offset, uint8 *data)
{
	uint16 i;
	uint16 data_start = 0;

	memset(&data[0], 0, size_data);
	*total_packets = 0;

	if (ctp_packet_type == AVCTP0_PACKET_TYPE_SINGLE)
	{
		*total_packets = 1;
		data_start = 13;
	}
	else if (ctp_packet_type == AVCTP0_PACKET_TYPE_START)
	{
		*total_packets = ptr[1];
		data_start = 14;
	}
	else if ((ctp_packet_type == AVCTP0_PACKET_TYPE_CONTINUE) || (ctp_packet_type == AVCTP0_PACKET_TYPE_END))
	{
		*data_offset = 11;
	}
	else
		return FALSE;

	if ((ctp_packet_type == AVCTP0_PACKET_TYPE_SINGLE) || (ctp_packet_type == AVCTP0_PACKET_TYPE_START))
	{
		if ((meta_packet_type == AVCTP0_PACKET_TYPE_SINGLE) || (meta_packet_type == AVCTP0_PACKET_TYPE_START))
		{
			*data_offset = data_start + size_data;
			for (i=0; i<size_data; i++)
				data[i] = ptr[data_start+i];			
		}
		else
		{
			*data_offset = data_start;
		}
	}

	return TRUE;
}


/*****************************************************************************/
void avrcpSendCommonMetadataCfm(AVRCP *avrcp, avrcp_status_code status, uint16 transaction, uint16 id, Source source, uint16 packet_size)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_METADATA_CFM_MESSAGE);

    message->avrcp = avrcp;
    message->status = status;
	message->transaction = transaction;

	if ((status != avrcp_success) && (packet_size >= 14))
		message->status = getRejectCode(avrcp, packet_size - 13, status, source);

	MessageSend(avrcp->clientTask, id, message);
}


/*****************************************************************************/
void avrcpSendCommonFragmentedMetadataCfm(AVRCP *avrcp, avrcp_status_code status, bool response, uint16 id, uint16 transaction, uint16 total_packets, uint16 ctp_packet_type, uint16 metadata_packet_type, uint16 number_of_data_items, uint16 data_length, uint16 data_offset, Source source)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_FRAGMENTED_METADATA_CFM);

    message->avrcp = avrcp;
    message->status = status;

	message->transaction = transaction;
    message->no_packets = total_packets;
    message->ctp_packet_type = ctp_packet_type;
    message->metadata_packet_type = metadata_packet_type;
    message->number_of_data_items = number_of_data_items;

    if ((status == avrcp_success) && data_length)
    {
		message->data_offset = data_offset;
        message->size_data = data_length;
        message->data = source;
    }
    else
    {
		message->data_offset = 0;
        message->size_data = 0;
        message->data = 0;

		if (response)
			message->status = getRejectCode(avrcp, data_length, status, source);
    }

	MessageSend(avrcp->clientTask, id, message);
}


/*****************************************************************************/
void avrcpSendCommonMetadataInd(AVRCP *avrcp, uint16 id, uint16 transaction)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_METADATA_IND_MESSAGE);

    message->avrcp = avrcp;
	message->transaction = transaction;

	MessageSend(avrcp->clientTask, id, message);
}


/*****************************************************************************/
void avrcpSendCommonFragmentedMetadataInd(AVRCP *avrcp, uint16 id, uint16 transaction, uint16 total_packets, uint16 ctp_packet_type, uint16 metadata_packet_type, uint16 number_of_data_items, uint16 data_length, uint16 data_offset, Source source)
{
    MAKE_AVRCP_MESSAGE(AVRCP_COMMON_FRAGMENTED_METADATA_IND);

    message->avrcp = avrcp;
	message->transaction = transaction;
    message->no_packets = total_packets;
    message->ctp_packet_type = ctp_packet_type;
    message->metadata_packet_type = metadata_packet_type;
    message->number_of_data_items = number_of_data_items;
	message->data_offset = data_offset;
    message->size_data = data_length;
    message->data = source;

	MessageSend(avrcp->clientTask, id, message);
}



