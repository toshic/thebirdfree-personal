/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release


FILE NAME
	avrcp_signal_vendor.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_common.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_private.h"
#include "avrcp_signal_handler.h"
#include "avrcp_signal_vendor.h"

#include <panic.h>
#include <source.h>
#include <stdlib.h>
#include <string.h>


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
static void avrcpSetVendorPacketHeader(uint8 *ptr, const AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *req)
{
	/* AVRCP header */
	ptr[0] = req->ctype;
	ptr[1] = ((req->subunit_type << AVRCP1_SUBUNIT_TYPE_SHIFT) & AVRCP1_SUBUNIT_TYPE_MASK) |
		 (req->subunit_id & AVRCP1_SUBUNIT_ID_MASK);
	ptr[2] = AVRCP2_VENDORDEPENDENT;
	
	ptr[3] = (req->company_id & 0xFF0000) >> 16;
	ptr[4] = (req->company_id & 0x00FF00) >> 8;
	ptr[5] = (req->company_id & 0x0000FF);
}
#endif

/*****************************************************************************/
static void avrcpSendVendordependentIndToClient(AVRCP *avrcp, uint16 transaction, uint16 pkts, avc_subunit_type	subunit_type, avc_subunit_id subunit_id, uint32 coid, uint8 command_type, uint16 op_len, const uint8 *op_data)
{
	MAKE_AVRCP_MESSAGE_WITH_LEN(AVRCP_VENDORDEPENDENT_IND, op_len);
	message->transaction = transaction;
	message->no_packets = pkts;
	message->subunit_type = subunit_type;
	message->subunit_id = subunit_id;
	message->company_id = coid;
    message->command_type = command_type;
	message->size_op_data = op_len;
	
	if (op_len)
		memcpy(message->op_data, op_data, op_len);
	else
		message->op_data[0] = 0;	
	
	message->sink = avrcp->sink;
	message->avrcp = avrcp;
	MessageSend(avrcp->clientTask, AVRCP_VENDORDEPENDENT_IND, message);
}


/*****************************************************************************/
void avrcpSendVendordependentCfmToClient(AVRCP *avrcp, avrcp_status_code status, uint8 response)
{
	MAKE_AVRCP_MESSAGE(AVRCP_VENDORDEPENDENT_CFM);
	message->status = status;
	message->sink = avrcp->sink;
	message->response = response;
	message->avrcp = avrcp;
	MessageSend(avrcp->clientTask, AVRCP_VENDORDEPENDENT_CFM, message);
}


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void avrcpHandleInternalVendorDependentReq(AVRCP *avrcp, const AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *req)
{
	uint16 packet_size = 3 + AVRCP_TOTAL_HEADER_SIZE + req->data_length;
    
	avrcp_status_code result = avrcp_success;
	
    Sink sink = avrcp->sink;
    uint8 *dest = SinkMap(sink);
    uint8* ptr;
	uint16 max_mtu = avrcp->l2cap_mtu;
	uint16 no_complete_packets = 1;
    uint16 start_data_length = 0;
    uint16 continue_data_length = 0;
	uint16 i = 0, j = 0;
    uint16 stream_move;
    uint16 data_length = 0;
	uint16 extra_data = 0;
    
    if (packet_size > 515)
    {
        /* An AV/C command frame has 512 bytes max (plus 3 bytes for AVCTP header) */
        result = avrcp_fail;
		goto IntVendorDepReqError;
    }
	
	if (packet_size > max_mtu)
	{
		start_data_length = max_mtu - 10;
		continue_data_length = max_mtu - 7;
		no_complete_packets = (req->data_length - start_data_length) / continue_data_length;  
		/* If no remainder then reduce no_complete by one as we will always assume one end packet */
		if (!((req->data_length - start_data_length) % continue_data_length))	
			no_complete_packets--;
	}    

	if (dest == NULL)
    {
		result = avrcp_no_resource;
		goto IntVendorDepReqError;
    }
	else
	{
		if (avrcp->identifier)
			extra_data = avrcp->identifier[METADATA_HEADER_SIZE];

		if (packet_size <= max_mtu)
		{
			bool empty_source = FALSE;

			ptr = avrcpGrabSink(sink, 9 + extra_data);
			if (!ptr)
            {
                result = avrcp_no_resource;
				goto IntVendorDepReqError;
	        }
       
            /* AVCTP header */
			avrcpSetAvctpHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_SINGLE, 0);			

			/* AVRCP header */
			avrcpSetVendorPacketHeader(&ptr[3], req);
            
			/* 
				If the identifier is in use then it means the metadata header must be added in before
				the Source data.
			*/
			if (avrcp->identifier)
			{
				data_length = req->data_length - extra_data;
				j = 0;
				for (i = 0; i < extra_data+1; i++)
				{
					if (i != METADATA_HEADER_SIZE)
						ptr[9 + j++] = avrcp->identifier[i];
				}
				
				/* Only empty the source if it is the last fragment. */
				if ((avrcp->identifier[1] == AVCTP0_PACKET_TYPE_END) && (req->data == avrcp->continuation_data))
					empty_source = TRUE;

				free(avrcp->identifier);
				avrcp->identifier = 0;
			}
			else
				data_length = req->data_length;

            if (data_length)
            {
                stream_move = StreamMove(sink, req->data, data_length);
                if (stream_move != data_length)
                {
                    /* Just send the data that we managed to move into the sink */
                    data_length = stream_move;
                    packet_size = data_length + 9;
                }
            }

			(void)SinkFlush(sink,packet_size);

			if (empty_source)
			{
				SourceEmpty(req->data);
				avrcp->continuation_data = 0;
			}
		}
		else
		{       
			ptr = avrcpGrabSink(sink, 10 + extra_data);
			if (!ptr)
            {
                result = avrcp_no_resource;
				goto IntVendorDepReqError;
	        }

			/* AVCTP header */
			avrcpSetAvctpHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_START, no_complete_packets + 2);			

			/* AVRCP header */
			avrcpSetVendorPacketHeader(&ptr[4], req);

			if (avrcp->identifier)
			{
				start_data_length -= extra_data;
				j = 0;
				for (i = 0; i < extra_data+1; i++)
				{
					if (i != METADATA_HEADER_SIZE)
						ptr[10 + j++] = avrcp->identifier[i];
				}
				free(avrcp->identifier);
				avrcp->identifier = 0;
			}

			/* Copy data */
            stream_move = StreamMove(sink, req->data, start_data_length);
            if (stream_move != start_data_length)
            {
                /* Data source can't be moved into Sink as expected so return failure */
                result = avrcp_fail;            
                (void)SinkFlush(sink, 10 + stream_move);
				goto IntVendorDepReqError;             
            }
						
			(void) SinkFlush(sink,max_mtu);
         
			for (i = 0; i < no_complete_packets; i++)
			{
                ptr = avrcpGrabSink(sink, 7);
				if (!ptr)
				{
					result = avrcp_no_resource;
					goto IntVendorDepReqError;
				}             

				/* AVCTP header */
				avrcpSetAvctpHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_CONTINUE, 0);			

				/* AVRCP header */
				avrcpSetVendorPacketHeader(&ptr[1], req);

                stream_move = StreamMove(sink, req->data, continue_data_length);  
                if (stream_move != continue_data_length)
                {
                    /* Data source can't be moved into Sink as expected so return failure */
                    result = avrcp_fail;                            				                    
                    (void)SinkFlush(sink, 7 + stream_move);                   
                    goto IntVendorDepReqError;  
                }
				
				(void)SinkFlush(sink, max_mtu);
              
			}

            i = req->data_length - ((max_mtu - 10) + (max_mtu - 7) * no_complete_packets);                    

			ptr = avrcpGrabSink(sink, 7);
			if (!ptr)
			{
				result = avrcp_no_resource;
				goto IntVendorDepReqError;
			}             

			/* AVCTP header */
			avrcpSetAvctpHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_END, 0);			

			/* AVRCP header */
			avrcpSetVendorPacketHeader(&ptr[1], req);

			/* Copy data. i is the data length remaining for the end packet */
            stream_move = StreamMove(sink, req->data, i);
            if (stream_move != i)
            {
                /* Data source can't be moved into Sink as expected so return failure */
                result = avrcp_fail;                   
                (void)SinkFlush(sink, 7 + stream_move);
                goto IntVendorDepReqError;  
                return;
            }
		
			/* Send the data */
			(void) SinkFlush(sink, i+7);
		}

        if (req->company_id != AVRCP_BT_COMPANY_ID)
		    avrcp->pending = avrcp_vendor;

		if ((avrcp->pending == avrcp_request_continuation) || avrcp->continuation_pdu)
  			avrcp->pending = avrcp_none;
		else
		{
			if (packet_size <= max_mtu)
				MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_TIMEOUT);
			else
				MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_FRAGMENTED_TIMEOUT);
		}
	}
	return;
	
IntVendorDepReqError:
	
	SourceEmpty(req->data);
	
	if (result != avrcp_success)
	{
		if (avrcp->pending >= avrcp_get_caps)
        	avrcpSendMetadataFailCfmToClient(avrcp, result);
        else
            avrcpSendVendordependentCfmToClient(avrcp, result, 0);
	}
}
#endif

/*****************************************************************************/
void avrcpHandleInternalVendorDependentRes(AVRCP *avrcp, const AVRCP_INTERNAL_VENDORDEPENDENT_RES_T *res)
{
	uint16 packet_size;
    uint16 size_pos;
	
	if (!avrcp->identifier)
	{
		/* The source has been processed so drop it here. */
		avrcpSourceProcessed(avrcp);
	}
	else
	{
		if (avrcp->sink)
		{
			/* Retrieve the packet size of the command, assuming it was valid when we originally received it! */
			if ((((uint8 *)avrcp->identifier)[0] & 0x0C) == AVCTP0_PACKET_TYPE_START)
                size_pos = 6;
		    else if ((((uint8 *)avrcp->identifier)[0] & 0x0C) == AVCTP0_PACKET_TYPE_SINGLE)	
                size_pos = 5;
            else
                size_pos = 3;
            
            /* Replace our temp stored packet length with the correct value for the response message */
			packet_size = avrcp->identifier[size_pos];
            avrcp->identifier[size_pos] = AVRCP2_VENDORDEPENDENT;
			
			avrcpSendResponse(avrcp, (uint8 *)avrcp->identifier, packet_size, res->response);
		}
		
		/* We now free the original command pdu which was stored in the profile instance data */
		free(avrcp->identifier);
		avrcp->identifier = 0;
        avrcpHandleReceivedData(avrcp);
	}
}


void avrcpHandleVendorCommand(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{
	uint16 packet_type = ptr[0] & AVCTP0_PACKET_TYPE_MASK;

    if (avrcp->last_ctp_fragment == avrcp_frag_end)
		avrcp->last_ctp_fragment = avrcp_frag_none;
    
    {
		if ((packet_type == AVCTP0_PACKET_TYPE_START) ||
				(packet_type == AVCTP0_PACKET_TYPE_SINGLE))
		{	
			uint32 cid = 0;
			uint8 addition = 0;
	
			if (packet_type == AVCTP0_PACKET_TYPE_START)
			{
				avrcp->last_ctp_fragment = avrcp_frag_start;
				addition = 1;
			}
		
			/* 
				Any reply must contain the original message. We 'secretly' pass this to 
				the application as an identifier that it must pass back for the response. 
			*/
			avrcp->identifier = malloc(packet_size);
	
            if (avrcp->identifier)
            {
                uint16 oplen = packet_size - (9 + addition);
                
				avrcpBlockReceivedData(avrcp, avrcp_vendor, 0);

                /* Copy the data into a buffer */
                memcpy(avrcp->identifier, ptr, packet_size);
                
				cid = avrcpGetCompanyId(ptr, 6+ addition);
                /* temporarily replace type of message (AVRCP2_VENDORDEPENDENT) with packet length (used for response) */
                avrcp->identifier[5 + addition] = packet_size;
                
                /* Send the ind message to the client */
                avrcpSendVendordependentIndToClient(avrcp, 
                    (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, 
                    (uint16) (addition ? ptr[1] : 1), 
                    (avc_subunit_type) ((ptr[4 + addition] & AVRCP1_SUBUNIT_TYPE_MASK) >> AVRCP1_SUBUNIT_TYPE_SHIFT), 
                    ptr[4 + addition] & AVRCP1_SUBUNIT_ID_MASK, 
                    cid, ptr[3 + addition], oplen, oplen ? &ptr[9 + addition] : 0);
            }
            else
            {
                avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
            }
		}
		else 
        {
            if ((packet_type == AVCTP0_PACKET_TYPE_CONTINUE) ||
                (packet_type == AVCTP0_PACKET_TYPE_END))
            {
                /* Check packet has a valid header and payload if any */
                if ((avrcp->last_ctp_fragment != avrcp_frag_start) &&
                    (avrcp->last_ctp_fragment != avrcp_frag_continue))
                {
                    avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
                }
                else
                {
                    /* Get the company id */
                    uint32 cid = avrcpGetCompanyId(ptr, 4);

                    /* Reply must contain the original message. Store this in the profile instance data. */
                    avrcp->identifier = malloc(packet_size);
                
                    if (avrcp->identifier)
                    {
                        uint16 oplen = packet_size - 7;

						avrcpBlockReceivedData(avrcp, avrcp_vendor, 0);

                        /* Copy the message into the buffer */
                        memcpy(avrcp->identifier, ptr, packet_size);
                        
                        /* Temporarily replace type of message (AVRCP2_VENDORDEPENDENT) with packet length (used for response) */
                        avrcp->identifier[3] = packet_size;
                        
                        /* Send the ind message to the client task */
                        avrcpSendVendordependentIndToClient(avrcp, 
                            (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, 
                            0, 
                            (avc_subunit_type) ((ptr[2] & AVRCP1_SUBUNIT_TYPE_MASK) >> AVRCP1_SUBUNIT_TYPE_SHIFT), 
                            ptr[2] & AVRCP1_SUBUNIT_ID_MASK, cid,
                            ptr[1],
                            oplen, oplen ? &ptr[7] : 0);

                        if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_CONTINUE)
                            avrcp->last_ctp_fragment = avrcp_frag_continue;
                        else
                            avrcp->last_ctp_fragment = avrcp_frag_end;
                    }
                    else
                    {
                        avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
                    }
                }
            }
        }
	}
}


#ifdef AVRCP_CT_SUPPORT
void avrcpHandleVendorResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{
	/* Response, find correct slots for info depending on packet type */
    uint16 packet_type = ptr[0] & AVCTP0_PACKET_TYPE_MASK;
	uint16 ptrResponse = 3; /* default for single packet */

	if (packet_type == AVCTP0_PACKET_TYPE_START)
		ptrResponse = 4;
	else if ((packet_type == AVCTP0_PACKET_TYPE_CONTINUE) || (packet_type == AVCTP0_PACKET_TYPE_END))
		ptrResponse = 1;
	
	avrcpSendVendordependentCfmToClient(avrcp, avrcp_success, ptr[ptrResponse]);
	
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
