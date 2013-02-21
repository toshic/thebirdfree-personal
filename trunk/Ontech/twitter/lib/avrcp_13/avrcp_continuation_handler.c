/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_continuation_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_metadata_transfer.h"
#include "avrcp_common.h"
#include "avrcp_continuation_handler.h"
#include "avrcp_private.h"
#include "avrcp_signal_handler.h"

#include <source.h>


#ifdef AVRCP_CT_SUPPORT
static uint16 convert_packet_type(avrcp_packet_type type)
{
	/* Convert the enum value to the actual value used to indicate what fragment the packet is. */
	uint16 ret_type = AVCTP0_PACKET_TYPE_SINGLE;

	switch (type)
	{
	case avrcp_packet_type_start:
		ret_type = AVCTP0_PACKET_TYPE_START;
		break;
    case avrcp_packet_type_continue:
		ret_type = AVCTP0_PACKET_TYPE_CONTINUE;
		break;
    case avrcp_packet_type_end:
		ret_type = AVCTP0_PACKET_TYPE_END;
		break;
	case avrcp_packet_type_single:
	default:
		break;
	}
	return ret_type;
}
#endif

/*****************************************************************************/
void avrcpHandleRequestContinuingCommand(AVRCP *avrcp, uint16 transaction, uint16 pdu)
{
	/* Make sure the continuing PDU requested is the same as the one we are storing data for. */
	if (avrcp->continuation_pdu == pdu)
	{
		/* Use the same transaction label for all fragments. */
		avrcp->rsp_transaction_label = transaction;
		/* A message will be sent on this variable being reset, if there if more data to be sent. */
		avrcp->continuation_pdu = 0;
	}
}


/*****************************************************************************/
void avrcpHandleAbortContinuingCommand(AVRCP *avrcp, uint16 transaction, uint16 pdu)
{
	/* Send response back stating if the abort was successful. */
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_ABORT_CONTINUING_RES);

	if (avrcp->continuation_pdu == pdu)
	{
		/* Forget about the data stored. */
		abortContinuation(avrcp);
		message->response = avctp_response_accepted;
	}
	else
		/* The abort continuing PDU requested is not the same as the one we are storing data for, so reject. */
		message->response = avctp_response_rejected;

	avrcp->rsp_transaction_label = transaction;

    MessageSend(&avrcp->task, AVRCP_INTERNAL_ABORT_CONTINUING_RES, message);
}


/*****************************************************************************/
void avrcpHandleInternalAbortContinuingResponse(AVRCP *avrcp, const AVRCP_INTERNAL_ABORT_CONTINUING_RES_T *res)
{
	/* Send a response to the abort continuing request. */
    sendMetadataResponse(avrcp,  res->response, AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID, 0, avrcp_packet_type_single, 0, 0, 0);

    avrcpHandleReceivedData(avrcp);
}


/*****************************************************************************/
void avrcpStoreNextContinuationPacket(AVRCP *avrcp, Source data, uint16 param_length, uint16 pdu_id, uint16 curr_packet, uint16 response, uint8 transaction)
{
	/* Store futher fragments until the CT request them. */
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET);
	message->data = data;
	message->param_length = param_length;
	message->pdu_id = pdu_id;
	message->curr_packet = curr_packet;
	message->response = response;
	message->transaction = transaction;
	
	avrcp->continuation_pdu = pdu_id;
	avrcp->continuation_data = data;

	MessageSendConditionally(&avrcp->task, AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET, message, &avrcp->continuation_pdu);
}


/*****************************************************************************/
void avrcpHandleNextContinuationPacket(AVRCP *avrcp, const AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET_T *ind)
{
	/* The next fragment has been requested, so send it. */
	uint16 data_length = 0;
	uint16 mtu_packet_data_length = 515 - 13;
    uint16 no_mtu_packets = 0;
	avrcp_packet_type packet_type = avrcp_packet_type_continue;

	if (ind->param_length > mtu_packet_data_length)
		no_mtu_packets = (ind->param_length - mtu_packet_data_length)/mtu_packet_data_length;

	if (ind->curr_packet == no_mtu_packets + 2)
	{
		/* This is the last fragment to be sent. */
		data_length = ind->param_length - (mtu_packet_data_length * (no_mtu_packets+1));
		packet_type = avrcp_packet_type_end;
		avrcp->continuation_pdu = ind->pdu_id;
	}
	else
	{
		/* There are more fragments to be sent, store the data for the following fragments. */
		avrcpStoreNextContinuationPacket(avrcp, ind->data, ind->param_length, ind->pdu_id, ind->curr_packet+1, ind->response, ind->transaction);
		data_length = mtu_packet_data_length;
	}

	if (ind->response)
	{
		/* This is a fragmented response. */
		avrcp->rsp_transaction_label = ind->transaction;
		sendMetadataResponse(avrcp,  ind->response, ind->pdu_id, ind->data, packet_type, data_length, 0, 0);
		if (packet_type == avrcp_packet_type_end)
		{
			SourceEmpty(ind->data);
			avrcp->continuation_data = 0;
			avrcpHandleReceivedData(avrcp);
		}
	}
#ifdef AVRCP_CT_SUPPORT
	else
	{
		/* This is a fragmented command. */
		uint16 length = 0;
		avrcp->identifier = avrcpCreateMetadataTransferCmd(AVRCP_SET_APP_VALUE_PDU_ID, ind->param_length, 0, 1, convert_packet_type(packet_type), &length);
		avrcp->identifier[METADATA_HEADER_SIZE] = METADATA_HEADER_SIZE;
		AvrcpVendorDependent(avrcp, subunit_panel, 0x00, AVRCP0_CTYPE_STATUS, AVRCP_BT_COMPANY_ID, data_length + METADATA_HEADER_SIZE, ind->data);
	}
#endif	
}
