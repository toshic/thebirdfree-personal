/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release


FILE NAME
	avrcp_signal_passthrough.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_common.h"
#include "avrcp_group_navigation_handler.h"
#include "avrcp_private.h"
#include "avrcp_signal_handler.h"
#include "avrcp_signal_passthrough.h"

#include <source.h>
#include <stdlib.h>
#include <string.h>


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
static void avrcpSetPassthroughPacketHeader(uint8 *ptr, const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req)
{
	/* AVRCP header */
	ptr[0] = AVRCP0_CTYPE_CONTROL;
	ptr[1] = ((req->subunit_type << AVRCP1_SUBUNIT_TYPE_SHIFT) & AVRCP1_SUBUNIT_TYPE_MASK) | (req->subunit_id & AVRCP1_SUBUNIT_ID_MASK);
	ptr[2] = AVRCP2_PASSTHROUGH;
	ptr[3] = req->opid | (req->state ? AVRCP3_PASSTHROUGH_STATE_MASK : 0);
	ptr[4] = req->operation_data_length;
}


/*****************************************************************************/
void avrcpSendPassthroughCfmToClient(AVRCP *avrcp, avrcp_status_code status)
{
	avrcpSendCommonCfmMessageToApp(AVRCP_PASSTHROUGH_CFM, status, avrcp->sink, avrcp);
}


/*****************************************************************************/
void avrcpHandleInternalPassThroughReq(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req)
{
	Sink sink = avrcp->sink;

	avrcp_status_code result = avrcp_success;
	
    uint16 stream_move;
	uint16 packet_size = req->operation_data_length + 8;
    uint16 max_mtu = avrcp->l2cap_mtu;
    uint8 *dest = SinkMap(sink);
    uint8* ptr;
    
	if (dest == NULL)
	{
		/* No space in the buffer */
		result = avrcp_no_resource;
		goto IntPassthroughReqError;	
	}
    else if (packet_size > 515)
	{
        /* An AV/C command frame has 512 bytes max (plus 3 bytes for AVCTP header)*/
        result = avrcp_fail;
		goto IntPassthroughReqError;
	}
	else
	{
		if (packet_size > max_mtu)
		{
			if (req->opid == opid_vendor_unique)
			{
				avrcpHandleFragmentedPassThroughReq(avrcp, req, dest, max_mtu);
			}
			else
			{
				result = avrcp_unsupported;
				goto IntPassthroughReqError;				
			}
		}
		else
		{
			ptr = avrcpGrabSink(sink, 8);
			if (!ptr)
            {
                result = avrcp_no_resource;
				goto IntPassthroughReqError;
	        }
       
            /* AVCTP header */
			avrcpSetAvctpHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_SINGLE, 0);			

			/* AVRCP header */
			avrcpSetPassthroughPacketHeader(&ptr[3], req);           
            
            if (ptr[7])
            {
                stream_move = StreamMove(sink, req->operation_data, ptr[7]);
                if (stream_move != ptr[7])
                {
                    /* Just send the data that we managed to move into the sink */
                    ptr[7] = stream_move;
                    packet_size = ptr[7] + 8;
                }
            }
           
			/* Flush the data */
			(void)SinkFlush(sink,packet_size);
          
			/* Set the pending flag and watchdog */
			if (!avrcp->pending)
				avrcp->pending = avrcp_passthrough;

			/* Set a watchdog timeout */
			MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_TIMEOUT);
		}
	}
   
IntPassthroughReqError:

	SourceEmpty(req->operation_data);
	
	if (result != avrcp_success)
		avrcpSendPassthroughCfmToClient(avrcp, result);
}


/*****************************************************************************/
void avrcpHandleFragmentedPassThroughReq(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req, uint8 *dest, uint16 max_mtu)
{
	avrcp_status_code result = avrcp_success;

	uint16 no_complete_packets;
	uint16 i = 0;
	uint16 start_data_length = max_mtu - 9;
	uint16 continue_data_length = max_mtu - 6;
    uint16 stream_move;
    uint16 packet_size;
    uint8 *ptr;
	Sink sink = avrcp->sink;

	no_complete_packets = (req->operation_data_length - start_data_length) / continue_data_length;  
	/* If no remainder then reduce no_complete by one as we will always assume one end packet */
	if (!((req->operation_data_length - start_data_length) % continue_data_length))	
		no_complete_packets--;
    
   
    ptr = avrcpGrabSink(sink, 9);
	if (!ptr)
    {
        result = avrcp_no_resource;
		goto IntPassthroughFragReqError;
	}

    /* AVCTP header */
	avrcpSetAvctpHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_START, no_complete_packets + 2);			

	/* AVRCP header */
	avrcpSetPassthroughPacketHeader(&ptr[4], req);   
	
	/* Copy the operation data */
	ptr[8] = max_mtu - 9;
    
    stream_move = StreamMove(sink, req->operation_data, ptr[8]);
    if (stream_move != ptr[8])
    {
        /* Data source can't be moved into Sink as expected so return failure.
           As we've claim memory in the Sink we must flush the data. Let the target
           work out that it hasn't received all the packets. 
        */
		result = avrcp_fail;              
        (void)SinkFlush(sink, 9 + stream_move);
        goto IntPassthroughFragReqError;
        return;            
    }
    
    start_data_length = ptr[8];
    packet_size = max_mtu;
	
	/* Send the data */
	(void)SinkFlush(sink, packet_size);
	
	for (i = 0; i < no_complete_packets; i++)
	{
		ptr = avrcpGrabSink(sink, 6);
		if (!ptr)
		{
			result = avrcp_no_resource;
			goto IntPassthroughFragReqError;
		}

		/* AVCTP header */
		avrcpSetAvctpHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_CONTINUE, 0);			

		/* AVRCP header */
		avrcpSetPassthroughPacketHeader(&ptr[1], req);  
           		
		/* Copy data */
		/* This data copy can probably be taken out into a separate fun */
		ptr[5] = max_mtu - 6;
        
        stream_move = StreamMove(sink, req->operation_data, ptr[5]);    
        if (stream_move != ptr[5])
        {
            result = avrcp_fail;                  
            (void)SinkFlush(sink, 6 + stream_move);
            goto IntPassthroughFragReqError;
        }
        
        continue_data_length = ptr[5];
        packet_size = max_mtu;
        
		/* Send the data */
		(void)SinkFlush(sink, packet_size);
	}
    
	ptr = avrcpGrabSink(sink, 6);
	if (!ptr)
	{
		result = avrcp_no_resource;
		goto IntPassthroughFragReqError;
	}

	/* AVCTP header */
	avrcpSetAvctpHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_END, 0);			

	/* AVRCP header */
	avrcpSetPassthroughPacketHeader(&ptr[1], req); 

	/* Copy data. i is the data length remaining for the end packet */
	i = req->operation_data_length - ((max_mtu - 9) + (max_mtu - 6) * no_complete_packets);
	ptr[5] = i;
      
    stream_move = StreamMove(sink, req->operation_data, i);   
    if (stream_move != i)
    {
        result = avrcp_fail;                      
        (void)SinkFlush(sink, 6 + stream_move);
        goto IntPassthroughFragReqError;     
    }
 
    continue_data_length = i;
    packet_size = 6 + i;
	
	/* Send the data */
	(void)SinkFlush(sink, packet_size);
 
	/* Set the state flags and watchdog timeout */
	avrcp->pending = avrcp_passthrough;
	MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_FRAGMENTED_TIMEOUT);

IntPassthroughFragReqError:

	SourceEmpty(req->operation_data);
	
	if (result != avrcp_success)
		avrcpSendPassthroughCfmToClient(avrcp, result);    
}
#endif

/*****************************************************************************/
void avrcpHandleInternalPassThroughRes(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_RES_T *res)
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
			avrcpSendResponse(avrcp, (uint8 *) avrcp->identifier, packet_size, res->response);
		}
	
		/* 
			We now free the original command pdu which was hidden in the identifier 
			field of AVRCP_PASSTHROUGH_IND. Note that this is NOT portable!
		*/
		free(avrcp->identifier);
		avrcp->identifier = 0;
        avrcpHandleReceivedData(avrcp);
	}
}


void avrcpHandlePassthroughCommand(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{
    uint16 packet_type = ptr[0] & AVCTP0_PACKET_TYPE_MASK;
	uint8 transaction = (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F;
    
    if (avrcp->last_ctp_fragment == avrcp_frag_end)
		avrcp->last_ctp_fragment = avrcp_frag_none;
	
    avrcp->identifier = malloc(packet_size);
			
	if (avrcp->identifier)
	{	
		uint16 passthrough_mask = 0;
		uint16 subunit_mask = 0;
		uint16 iden_offset = 0;
		uint16 pkts = 1;

        avrcpBlockReceivedData(avrcp, avrcp_passthrough, 0);

		/* Copy the message into the buffer */
		memcpy(avrcp->identifier, ptr, packet_size);
        
        if (packet_type == AVCTP0_PACKET_TYPE_SINGLE)
        {
			avrcp->last_ctp_fragment = avrcp_frag_none;
            /* Check packet has a valid header and payload if any */
            if ((packet_size < (AVRCP_TOTAL_HEADER_SIZE + 2)) ||
			(packet_size < (AVRCP_TOTAL_HEADER_SIZE + ptr[7])))
	        {
		        /* Reject the response - invalid header or payload */
                free(avrcp->identifier);
				avrcp->identifier = 0;
		        avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
                return;
	        }
			/* Check if this is a Metadata Group command. */
			if ((packet_size >= 13) && ((ptr[6] & 0x7F) == opid_vendor_unique) && (ptr[7] == 0x05) && (avrcpGetCompanyId(ptr, 8) == AVRCP_BT_COMPANY_ID))
			{
                if(avrcp->srcUsed > 13)
                {
                    avrcp->srcUsed = 13; /* Size of Group */
                }
				avrcpSendGroupIndToClient(avrcp, (ptr[11] << 8) | ptr[12], transaction);
			}
			else
			{
				/* Send the Passthrough message to the client */
				passthrough_mask = 6; subunit_mask = 4; iden_offset = 7;
			}
		} 
        else if (packet_type == AVCTP0_PACKET_TYPE_START)
        {
            avrcp->last_ctp_fragment = avrcp_frag_start;

			passthrough_mask = 7; subunit_mask = 5; iden_offset = 8; pkts = ptr[1];      
        } 
        else if ((packet_type == AVCTP0_PACKET_TYPE_CONTINUE) ||
				(packet_type == AVCTP0_PACKET_TYPE_END))
        {
            if ((avrcp->last_ctp_fragment != avrcp_frag_start) &&
				(avrcp->last_ctp_fragment != avrcp_frag_continue))
			{
                free(avrcp->identifier);
				avrcp->identifier = 0;
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
                return;
			}
            
			passthrough_mask = 4; subunit_mask = 2; iden_offset = 5; pkts = 0;
			
			if (packet_type == AVCTP0_PACKET_TYPE_CONTINUE)
				avrcp->last_ctp_fragment = avrcp_frag_continue;
			else
				avrcp->last_ctp_fragment = avrcp_frag_end;
        } 

		if (passthrough_mask != 0)
		{
            uint16 pdu_size = avrcp->identifier[iden_offset] + iden_offset +1;
			MAKE_AVRCP_MESSAGE_WITH_LEN(AVRCP_PASSTHROUGH_IND, avrcp->identifier[iden_offset]);

			message->opid = (avc_operation_id) (ptr[passthrough_mask] & AVRCP3_PASSTHROUGH_OP_MASK);
			message->state = ptr[passthrough_mask] & AVRCP3_PASSTHROUGH_STATE_MASK ? 1 : 0;
			message->subunit_type = (avc_subunit_type) ((ptr[subunit_mask] & AVRCP1_SUBUNIT_TYPE_MASK) >> AVRCP1_SUBUNIT_TYPE_SHIFT);
			message->subunit_id = ptr[subunit_mask] & AVRCP1_SUBUNIT_ID_MASK;
			message->size_op_data = avrcp->identifier[iden_offset];

            /* Find the packet boundry */
            if(avrcp->srcUsed > pdu_size) 
            {
                avrcp->srcUsed = pdu_size;
            }
			
			if (avrcp->identifier[iden_offset])
				memcpy(message->op_data, avrcp->identifier[iden_offset] ? &avrcp->identifier[iden_offset+1] : 0, message->size_op_data);
			else
				message->op_data[0] = 0;	
    
			message->transaction = transaction;
			message->no_packets = pkts;
			message->sink = avrcp->sink;
			message->avrcp = avrcp;
						
			MessageSend(avrcp->clientTask, AVRCP_PASSTHROUGH_IND, message);
		}
    }
	else
	{
	    avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
	}    
}


#ifdef AVRCP_CT_SUPPORT
void avrcpHandlePassthroughResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{
	/* Response, find correct slots for info depending on packet type */
    uint16 packet_type = ptr[0] & AVCTP0_PACKET_TYPE_MASK;
	uint16 ptrResponse = 3; /* default for single packet */
    uint16 pdu_size; 

	if (packet_type == AVCTP0_PACKET_TYPE_START)
		ptrResponse = 4;
	else if ((packet_type == AVCTP0_PACKET_TYPE_CONTINUE) || (packet_type == AVCTP0_PACKET_TYPE_END))
		ptrResponse = 1;
	
    pdu_size =  ptrResponse + 5 + ptr[ptrResponse + 4];
    if(pdu_size < avrcp->srcUsed)
    {
        avrcp->srcUsed = pdu_size;
    }

	if (ptr[ptrResponse] == avctp_response_accepted)
		avrcpSendPassthroughCfmToClient(avrcp, avrcp_success);
	else if (ptr[ptrResponse] == avctp_response_not_implemented)
		avrcpSendPassthroughCfmToClient(avrcp, avrcp_unsupported);
	else
		/* default to a fail handles avctp_response_rejected etc */
		avrcpSendPassthroughCfmToClient(avrcp, avrcp_fail);
	
	(void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);

	if ((packet_type == AVCTP0_PACKET_TYPE_SINGLE) || (packet_type == AVCTP0_PACKET_TYPE_END))
		/* No longer waiting */
		avrcp->pending = avrcp_none;
	else
		MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_TIMEOUT);

	/* The source has been processed so drop it here. */
	avrcpSourceProcessed(avrcp);
}
#endif


