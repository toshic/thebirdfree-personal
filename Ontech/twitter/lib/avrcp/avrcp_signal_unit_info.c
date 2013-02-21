/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release


FILE NAME
	avrcp_signal_unit_info.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_common.h"
#include "avrcp_private.h"
#include "avrcp_signal_handler.h"
#include "avrcp_signal_unit_info.h"

#include <stdlib.h>
#include <string.h>


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void avrcpSendUnitInfoCfmToClient(AVRCP *avrcp, avrcp_status_code status, uint16 unit_type, uint16 unit, uint32 company)
{
	MAKE_AVRCP_MESSAGE(AVRCP_UNITINFO_CFM);
	message->status = status;
	message->sink = avrcp->sink;
	message->unit_type = (avc_subunit_type) unit_type;
	message->unit = unit;
	message->company_id = company;	
	message->avrcp = avrcp;
	MessageSend(avrcp->clientTask, AVRCP_UNITINFO_CFM, message);
}


/*****************************************************************************/
void avrcpHandleInternalUnitInfoReq(AVRCP *avrcp)
{
	uint16 packet_size = 5 + AVRCP_TOTAL_HEADER_SIZE;
    
    Sink sink = avrcp->sink;

    uint8* ptr = avrcpGrabSink(sink, packet_size);

	if (!ptr)
		avrcpSendUnitInfoCfmToClient(avrcp, avrcp_no_resource, 0, 0, (uint32) 0);
	else
	{
		/* AVCTP header */
		avrcpSetAvctpHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_SINGLE, 0);	

		/* AVRCP header */
		ptr[3] = AVRCP0_CTYPE_STATUS;
		ptr[4] = AVRCP1_UNIT;
		ptr[5] = AVRCP2_UNITINFO;

		/* UnitInfo set to all f's */
		memset(&ptr[6], 0xff, 5);

		/* Send the data */
		(void)SinkFlush(sink,packet_size);

		avrcp->pending = avrcp_unit_info;
		MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_TIMEOUT);
	}	
}
#endif

/*****************************************************************************/
void avrcpHandleInternalUnitInfoRes(AVRCP *avrcp, const AVRCP_INTERNAL_UNITINFO_RES_T *res)
{
	if (!avrcp->identifier)
	{
		/* The source has been processed so drop it here. */
		avrcpSourceProcessed(avrcp);
	}
	else
	{
		if (avrcp->sink)
		{
			if (!res->accept)
				avrcpSendResponse(avrcp, avrcp->identifier, AVRCP_TOTAL_HEADER_SIZE + 5, avctp_response_rejected);
			else
			{
				/* Write back our values and send response */
				avrcp->identifier[6] = 0x07; /* magic number from spec */
				avrcp->identifier[7] = (res->unit_type << AVRCP4_UNITINFO_UNIT_TYPE_SHIFT) | (res->unit & AVRCP4_UNITINFO_UNIT_MASK);
				avrcp->identifier[8] = res->company_id >> 16;
				avrcp->identifier[9] = (res->company_id >> 8) & 0xff;
				avrcp->identifier[10] = res->company_id & 0xff;
				
				avrcpSendResponse(avrcp, avrcp->identifier, AVRCP_TOTAL_HEADER_SIZE + 5, avctp_response_stable);
			}
		}
		
		/* 
			We now free the original command pdu which was stored in the identifier field 
			of the profile lib instance data.
		*/
		free(avrcp->identifier);
		avrcp->identifier = 0;
        avrcpHandleReceivedData(avrcp);
	}
}


void avrcpHandleUnitInfoCommand(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{
	/* Check packet has a valid header and payload if any */
	if (packet_size < (AVRCP_TOTAL_HEADER_SIZE + 5))
	{
		avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
		return;
	}
	else
	{
		/* Any reply must contain the original message so store it with the instance data */
		avrcp->identifier = malloc(packet_size);

		if (avrcp->identifier)
		{
			MAKE_AVRCP_MESSAGE(AVRCP_UNITINFO_IND);
			message->sink = avrcp->sink;
			message->avrcp = avrcp;
			MessageSend(avrcp->clientTask, AVRCP_UNITINFO_IND, message);

			avrcpBlockReceivedData(avrcp, avrcp_unit_info, 0);

			/* Copy the message into the buffer and store with the profile instance data */
			memcpy(avrcp->identifier, ptr, packet_size);
		}
		else
		{
			/* Failed to allocate memory to pass message to client */
			avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
		}
	}
}


#ifdef AVRCP_CT_SUPPORT
void avrcpHandleUnitInfoResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{ 
	if (ptr[3] == avctp_response_stable)
		avrcpSendUnitInfoCfmToClient(avrcp, avrcp_success, 
			ptr[7] >> AVRCP4_UNITINFO_UNIT_TYPE_SHIFT, 
			ptr[7] & AVRCP4_UNITINFO_UNIT_MASK, 
			(((uint32)ptr[8] << 16) & 0x00FF0000) | (((uint32)ptr[9] << 8) & 0x0000FF00) | ((uint32)ptr[10] & 0x000000FF));
	else
		avrcpSendUnitInfoCfmToClient(avrcp, avrcp_fail, 0, 0, (uint32) 0);

	/* No longer waiting */
	avrcp->pending = avrcp_none;
	(void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);

	/* The source has been processed so drop it here. */
	avrcpSourceProcessed(avrcp);
}

/*****************************************************************************/
void avrcpSendSubunitInfoCfmToClient(AVRCP *avrcp, avrcp_status_code status, uint8 page, const uint8 *page_data)
{
	MAKE_AVRCP_MESSAGE(AVRCP_SUBUNITINFO_CFM);
	message->status = status;
	message->page = page;

	if (page_data)
		memcpy(message->page_data, page_data, PAGE_DATA_LENGTH);
	else
		memset(message->page_data, 0, PAGE_DATA_LENGTH);

	message->sink = avrcp->sink;
	message->avrcp = avrcp;
	MessageSend(avrcp->clientTask, AVRCP_SUBUNITINFO_CFM, message);
}


/*****************************************************************************/
void avrcpHandleInternalSubUnitInfoReq(AVRCP *avrcp, const AVRCP_INTERNAL_SUBUNITINFO_REQ_T *req)
{
	uint16 packet_size = 5 + AVRCP_TOTAL_HEADER_SIZE;
    
    Sink sink = avrcp->sink;
    uint8* ptr = avrcpGrabSink(sink, packet_size);

	if (!ptr)
		avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_no_resource, 0, 0);
	else
	{
		/* AVCTP header */
		avrcpSetAvctpHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_SINGLE, 0);	

		/* AVRCP header */
		ptr[3] = AVRCP0_CTYPE_STATUS;
		ptr[4] = AVRCP1_UNIT;
		ptr[5] = AVRCP2_SUBUNITINFO;

		/* Fill in operand[0] - page and extension code*/
		ptr[6] = ((req->page << AVRCP3_SUBUNITINFO_PAGE_SHIFT) & 0x70) | AVRCP3_SUBUNITINFO_EXTEND_MASK;
        
        /* Set Page data to 0xFF */
        ptr[7] = 0xFF;
        ptr[8] = 0xFF;
        ptr[9] = 0xFF;
        ptr[10] = 0xFF;

		/* Send the data */
		(void) SinkFlush(sink,packet_size);

		avrcp->pending = avrcp_subunit_info;
		MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_TIMEOUT);
	}	
}
#endif

/*****************************************************************************/
void avrcpHandleInternalSubUnitInfoRes(AVRCP *avrcp, const AVRCP_INTERNAL_SUBUNITINFO_RES_T *res)
{
	if (!avrcp->identifier)
	{
		/* The source has been processed so drop it here. */
		avrcpSourceProcessed(avrcp);
	}
	else
	{
		if (avrcp->sink)
		{
			if (!res->accept)
				/* return NOT_IMPLEMENTED for empty pages */
				avrcpSendResponse(avrcp, avrcp->identifier, AVRCP_TOTAL_HEADER_SIZE + 5, avctp_response_not_implemented);
			else
			{
				/* Copy back the page data */
				memcpy(&avrcp->identifier[7], &res->page_data[0], PAGE_DATA_LENGTH);
				
				avrcpSendResponse(avrcp, avrcp->identifier, AVRCP_TOTAL_HEADER_SIZE + 5, avctp_response_stable);
			}
		}
       	/* 
			We now free the original command pdu which was hidden in the identifier field 
			of AVCT_UNITINFO_IND. Note that this is NOT portable!
		*/
		free(avrcp->identifier);
		avrcp->identifier = 0;
        avrcpHandleReceivedData(avrcp);
	}
}


void avrcpHandleSubUnitInfoCommand(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{
	/* Check packet has a valid header and payload if any */
	if (packet_size < (AVRCP_TOTAL_HEADER_SIZE + 5))
	{
		avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
	}
	else
	{
		/* 
			Any reply must contain the original message. Store the msg 
			while we wait for the response from the client.
		*/
		avrcp->identifier = malloc(packet_size);
		
		if (avrcp->identifier)
		{
			MAKE_AVRCP_MESSAGE(AVRCP_SUBUNITINFO_IND);
			message->page = ptr[6] >> AVRCP3_SUBUNITINFO_PAGE_SHIFT;
			message->sink = avrcp->sink;
			message->avrcp = avrcp;
			MessageSend(avrcp->clientTask, AVRCP_SUBUNITINFO_IND, message);

			avrcpBlockReceivedData(avrcp, avrcp_subunit_info, 0);

			/* Copy the message into the buffer */
			memcpy(avrcp->identifier, ptr, packet_size);
		}
		else
		{
			/* Failed to allocate memory to pass message to client */
			avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
		}
	}
}


#ifdef AVRCP_CT_SUPPORT
void avrcpHandleSubUnitInfoResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{
	if (ptr[3] == avctp_response_stable)
		avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_success, 
			ptr[6] >> AVRCP3_SUBUNITINFO_PAGE_SHIFT, &ptr[7]);
	else
		avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_fail, 0, 0);

	/* No longer waiting */
	avrcp->pending = avrcp_none;
	(void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);	

	/* The source has been processed so drop it here. */
	avrcpSourceProcessed(avrcp);
}
#endif

