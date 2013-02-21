/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_group_navigation_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_common.h"
#include "avrcp_group_navigation_handler.h"
#include "avrcp_private.h"
#include "avrcp_send_response.h"
#include "avrcp_signal_handler.h"

#include <source.h>
#include <stdlib.h>


static void sendGroupResponse(AVRCP *avrcp, avrcp_response_type response)
{
	/* Calculate the packet size of the command, assuming it was valid when we originally received it! */
	uint16 packet_size;
	uint16 data_length_slot = 7; /* default for single packet */
	uint16 header_size = AVRCP_TOTAL_HEADER_SIZE + 2; /* default for single packet */
    
	if ((((uint8 *)avrcp->identifier)[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START)
	{
		header_size += 1;
		data_length_slot += 1;
	}
	
	if (((((uint8 *)avrcp->identifier)[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_CONTINUE) ||
		((((uint8 *)avrcp->identifier)[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_END))
	{
		header_size -= 2;
		data_length_slot -= 2;
	}
	
	packet_size = header_size + ((uint8 *)avrcp->identifier)[data_length_slot];
    
	/* Send the response */
	avrcpSendResponse(avrcp, (uint8 *) avrcp->identifier, packet_size, response);
}


static void avrcpHandleInternalGroupResponse(AVRCP *avrcp, uint16 id, avrcp_response_type response)
{
	if (!avrcp->identifier)
		avrcpSendCommonMetadataCfm(avrcp, avrcp_fail, 0, id, 0, 0);
	else
	{
		if (avrcp->sink)
			sendGroupResponse(avrcp, response);
		else
			avrcpSendCommonMetadataCfm(avrcp, avrcp_invalid_sink, 0, id, 0, 0);
		
		free(avrcp->identifier);
		avrcp->identifier = 0;
        avrcpHandleReceivedData(avrcp);
	}
}


/*****************************************************************************/
void avrcpSendGroupIndToClient(AVRCP *avrcp, uint16 vendor_id, uint8 transaction)
{
	if (!(avrcp->local_target_features & AVRCP_GROUP_NAVIGATION))
	{
		if (vendor_id)
			sendPreviousGroupResponse(avrcp, avctp_response_not_implemented);
		else
			sendNextGroupResponse(avrcp, avctp_response_not_implemented);
		return;
	}
	else
	{
		if (vendor_id)
		{
			avrcpBlockReceivedData(avrcp, avrcp_previous_group, 0);
			avrcpSendCommonMetadataInd(avrcp, AVRCP_PREVIOUS_GROUP_IND, transaction);
		}
		else
		{
			avrcpBlockReceivedData(avrcp, avrcp_next_group, 0);
			avrcpSendCommonMetadataInd(avrcp, AVRCP_NEXT_GROUP_IND, transaction);
		}
	}
}


/*****************************************************************************/
void avrcpHandleInternalNextGroupResponse(AVRCP *avrcp, const AVRCP_INTERNAL_NEXT_GROUP_RES_T *res)
{
	avrcpHandleInternalGroupResponse(avrcp, AVRCP_NEXT_GROUP_CFM, res->response);
}


/*****************************************************************************/
void avrcpHandleInternalPreviousGroupResponse(AVRCP *avrcp, const AVRCP_INTERNAL_PREVIOUS_GROUP_RES_T *res)
{
	avrcpHandleInternalGroupResponse(avrcp, AVRCP_PREVIOUS_GROUP_CFM, res->response);
}


#ifdef AVRCP_CT_SUPPORT
void avrcpHandleGroupResponse(AVRCP *avrcp, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);
	uint16 packet_type = ptr[0] & AVCTP0_PACKET_TYPE_MASK;
	uint16 transaction = (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F;
	avrcp_status_code status = avrcp_fail;

	/* Check this is definitely a group response received. */
	if ((packet_type == AVCTP0_PACKET_TYPE_SINGLE) && (packet_size >= 13) && ((ptr[6] & 0x7F) == opid_vendor_unique) && (ptr[7] == 0x05) && (avrcpGetCompanyId(ptr, 8) == AVRCP_BT_COMPANY_ID))
	{
		uint16 id;

		if (ptr[3] == avctp_response_accepted)
			status = avrcp_success;

		if ((ptr[11] << 8) | ptr[12])
			id = AVRCP_PREVIOUS_GROUP_CFM;
		else
			id = AVRCP_NEXT_GROUP_CFM;
	
		avrcpSendCommonMetadataCfm(avrcp, status, transaction, id, source, packet_size);

		avrcp->pending = avrcp_none;
	}

	/* The source has been processed so drop it here. */
	avrcpSourceProcessed(avrcp);
}
#endif
