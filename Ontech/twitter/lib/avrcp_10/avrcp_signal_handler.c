/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	avrcp_signal_handler.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_common.h"
#include "avrcp_signal_handler.h"

#include <panic.h>
#include <string.h>


/*lint -e641 -e572 */


/*
	Returns the next transaction label. This is used to route responses from 
	the remote device to the correct local initiator. Note that zero will 
	never be issued and hence can be used as a special case to indicate no 
	command is pending.
*/
static void getNextTransactionLabel(AVRCP *avrcp)
{
	avrcp->transaction_label = (avrcp->transaction_label + 1) & 0xf;

	/* 
		We treat a label of zero as a special case indicating there is no 
		transaction pending. 
	*/
	if (!avrcp->transaction_label) 
		avrcp->transaction_label = 1;
}


/* Return pointer to the Sink */
static uint8 *avrcpGrabSink(Sink sink, uint16 size)
{
	uint8 *dest = SinkMap(sink);
	uint16 claim_result = SinkClaim(sink, size);

	if (claim_result == 0xffff)
	{
		return NULL;
	}

	return (dest + claim_result);
}


static void avrcpSetAvctpHeader(AVRCP *avrcp, uint8 *ptr, uint16 add_number_packets)
{
	uint16 offset = 0;
	uint8 packet_type = AVCTP0_PACKET_TYPE_SINGLE;

	if (add_number_packets)
	{
		packet_type = AVCTP0_PACKET_TYPE_START;
		ptr[1] = add_number_packets;
		offset = 1;
	}

	ptr[0] = ((avrcp->transaction_label << AVCTP0_TRANSACTION_SHIFT) | packet_type | AVCTP0_CR_COMMAND);
	ptr[1 + offset] = AVCTP1_PROFILE_AVRCP_HIGH;
	ptr[2 + offset] = AVCTP2_PROFILE_AVRCP_LOW;
}


static void avrcpSetPassThroughHeader(uint8 *ptr, const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req)
{
	/* AVRCP header */
	ptr[0] = AVRCP0_CTYPE_CONTROL;
	ptr[1] = ((req->subunit_type << AVRCP1_SUBUNIT_TYPE_SHIFT) & AVRCP1_SUBUNIT_TYPE_MASK) | (req->subunit_id & AVRCP1_SUBUNIT_ID_MASK);
	ptr[2] = AVRCP2_PASSTHROUGH;
	/* Pass-through */
	ptr[3] = req->opid | (req->state ? AVRCP3_PASSTHROUGH_STATE_MASK : 0);
}


static void avrcpSetVendorPacketHeader(uint8 *ptr, const AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *req)
{
	/* AVRCP Vendor header */
	ptr[0] = req->ctype;
	ptr[1] = ((req->subunit_type << AVRCP1_SUBUNIT_TYPE_SHIFT) & AVRCP1_SUBUNIT_TYPE_MASK) |
	(req->subunit_id & AVRCP1_SUBUNIT_ID_MASK);
	ptr[2] = AVRCP2_VENDORDEPENDENT;

	ptr[3] = (req->company_id & 0xFF0000) >> 16;
	ptr[4] = (req->company_id & 0x00FF00) >> 8;
	ptr[5] = (req->company_id & 0x0000FF);
}


static uint32 avrcpConvertUint8ValuesToUint32(const uint8 *ptr)
{
	return ( ((uint32)ptr[0] << 16) | ((uint32)ptr[1] << 8) | (uint32)ptr[2] );
}


/* Send a AVRCP_PASSTHROUGH_IND message to the client task. */
static void avrcpSendPassthroughIndToClient(AVRCP *avrcp, avc_operation_id opid, bool state, avc_subunit_type subunit_type, avc_subunit_id subunit_id, uint16 trans, uint16 pkts, uint16 op_len, const uint8 *op_data)
{
	MAKE_AVRCP_MESSAGE_WITH_LEN(AVRCP_PASSTHROUGH_IND, op_len);
	message->opid = opid;
	message->state = state;
	message->subunit_type = subunit_type;
	message->subunit_id = subunit_id;
	message->size_op_data = op_len;
	
	if (op_len)
		memmove(message->op_data, op_data, op_len);
	else
		message->op_data[0] = 0;	
    
	message->transaction = trans;
	message->no_packets = pkts;
	message->sink = avrcp->sink;
	message->avrcp = avrcp;
				
	MessageSend(avrcp->clientTask, AVRCP_PASSTHROUGH_IND, message);
}


/* Send a AVRCP_VENDORDEPENDENT_IND message to the client task. */
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
		memmove(message->op_data, op_data, op_len);
	else
		message->op_data[0] = 0;	
	
	message->sink = avrcp->sink;
	message->avrcp = avrcp;
	MessageSend(avrcp->clientTask, AVRCP_VENDORDEPENDENT_IND, message);
}


/****************************************************************************
NAME	
	avrcpSendPassthroughCfmToClient

DESCRIPTION
	This function creates a AVRCP_PASSTHROUGH_CFM message and sends it to 
	the client task.
*/
void avrcpSendPassthroughCfmToClient(AVRCP *avrcp, avrcp_status_code status)
{
	avrcpSendCommonCfmMessageToApp(AVRCP_PASSTHROUGH_CFM, status, avrcp->sink, avrcp);
}


/****************************************************************************
NAME	
	avrcpSendVendordependentCfmToClient

DESCRIPTION
	This function creates a AVRCP_VENDORDEPENDENT_CFM message and sends it to 
	the client task.
*/
void avrcpSendVendordependentCfmToClient(AVRCP *avrcp, avrcp_status_code status, uint8 response)
{
	MAKE_AVRCP_MESSAGE(AVRCP_VENDORDEPENDENT_CFM);
	message->status = status;
	message->sink = avrcp->sink;
	message->response = response;
	message->avrcp = avrcp;
	MessageSend(avrcp->clientTask, AVRCP_VENDORDEPENDENT_CFM, message);
}


/****************************************************************************
NAME	
	avrcpSendUnitInfoCfmToClient

DESCRIPTION
	This function creates a AVRCP_UNITINFO_CFM message and sends it to 
	the client task.
*/
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


/****************************************************************************
NAME	
	avrcpSendSubunitInfoCfmToClient

DESCRIPTION
	This function creates a AVRCP_SUBUNITINFO_CFM message and sends it to 
	the client task.
*/
void avrcpSendSubunitInfoCfmToClient(AVRCP *avrcp, avrcp_status_code status, uint8 page, const uint8 *page_data)
{
	MAKE_AVRCP_MESSAGE(AVRCP_SUBUNITINFO_CFM);
	message->status = status;
	message->page = page;

	if (page_data)
		memmove(message->page_data, page_data, PAGE_DATA_LENGTH);
	else
		memset(message->page_data, 0, PAGE_DATA_LENGTH);

	message->sink = avrcp->sink;
	message->avrcp = avrcp;
	MessageSend(avrcp->clientTask, AVRCP_SUBUNITINFO_CFM, message);
}


/****************************************************************************
NAME	
	avrcpHandleInternalPassThroughReq

DESCRIPTION
	This function internally handles a pass through message request
*/
void avrcpHandleInternalPassThroughReq(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req)
{
	Sink sink = avrcp->sink;
	
    uint16 stream_move;
	uint16 packet_size = req->operation_data_length + 8;
    uint16 max_mtu = avrcp->l2cap_mtu;
    uint8* ptr;
    
	if (avrcp->watchdog)
	{
		/* Watchdog set, we're busy */
		avrcpSendPassthroughCfmToClient(avrcp, avrcp_busy);
	}
    else if (packet_size > 515)
        /* An AV/C command frame has 512 bytes max (plus 3 bytes for AVCTP header)*/
        avrcpSendPassthroughCfmToClient(avrcp, avrcp_fail);
	else
	{
		getNextTransactionLabel(avrcp);
	
		if (packet_size > max_mtu)
		{
			if (req->opid == opid_vendor_unique)
				avrcpHandleFragmentedPassThroughReq(avrcp, req, max_mtu);
			else
				avrcpSendPassthroughCfmToClient(avrcp, avrcp_unsupported);
		}
		else
		{
			ptr = avrcpGrabSink(sink, 8);
         
            if (ptr == NULL)
            {
		        avrcpSendPassthroughCfmToClient(avrcp, avrcp_no_resource);
                if (req->operation_data)
                    SourceEmpty(req->operation_data);
                return;
	        }
                       
            /* AVCTP header */
			avrcpSetAvctpHeader(avrcp, &ptr[0], 0);
			
			/* AVRCP header */
			avrcpSetPassThroughHeader(&ptr[3], req);
			
		    ptr[7] = req->operation_data_length;
            
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
			avrcp->pending = avrcp_passthrough;
			avrcp->watchdog = 1;

			/* Set a watchdog timeout */
			MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_TIMEOUT);
		}
	}
    SourceEmpty(req->operation_data);
}


/****************************************************************************
NAME	
	avrcpHandleFragmentedPassThroughReq

DESCRIPTION
	This function handles a pass through message request, with fragmentation
*/
void avrcpHandleFragmentedPassThroughReq(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req, uint16 max_mtu)
{
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
    
	if (ptr == NULL)
	{
		/* We don't have any space in the buffer */
		avrcpSendPassthroughCfmToClient(avrcp, avrcp_no_resource);
	}
	else
	{      
		/* Write start packet */
		avrcpSetAvctpHeader(avrcp, &ptr[0], no_complete_packets + 2);
		
		/* AVRCP header */
		avrcpSetPassThroughHeader(&ptr[3], req);
		
		/* Copy the operation data */
		ptr[8] = max_mtu - 9;
        
        stream_move = StreamMove(sink, req->operation_data, ptr[8]);
        if (stream_move != ptr[8])
        {
            /* Data source can't be moved into Sink as expected so return failure.
               As we've claim memory in the Sink we must flush the data. Let the target
               work out that it hasn't received all the packets. 
            */
            avrcpSendPassthroughCfmToClient(avrcp, avrcp_fail);
            packet_size = 9 + stream_move;
            (void)SinkFlush(sink, packet_size);
            SourceEmpty(req->operation_data);
            return;            
        }
        
        start_data_length = ptr[8];
        packet_size = max_mtu;
		
		/* Send the data */
		(void)SinkFlush(sink, packet_size);
		
		for (i = 0; i < no_complete_packets; i++)
		{            
			ptr = avrcpGrabSink(sink, 6);
               
            if (ptr == NULL)
            {
		        avrcpSendPassthroughCfmToClient(avrcp, avrcp_no_resource);
                SourceEmpty(req->operation_data);
                return;
            }
                        
			/* Write continue packets */
			ptr[0] = (avrcp->transaction_label << AVCTP0_TRANSACTION_SHIFT) | AVCTP0_PACKET_TYPE_CONTINUE | AVCTP0_CR_COMMAND;
			
			/* AVRCP header */
			avrcpSetPassThroughHeader(&ptr[3], req);
			
			/* Copy data */
			/* This data copy can probably be taken out into a separate fun */
			ptr[5] = max_mtu - 6;
            
            stream_move = StreamMove(sink, req->operation_data, ptr[5]);    
            if (stream_move != ptr[5])
            {
                avrcpSendPassthroughCfmToClient(avrcp, avrcp_fail);
                packet_size = 6 + stream_move;
                (void)SinkFlush(sink, packet_size);
                SourceEmpty(req->operation_data);
                return;
            }
            
            continue_data_length = ptr[5];
            packet_size = max_mtu;
            
			/* Send the data */
			(void)SinkFlush(sink, packet_size);
		}
              
		ptr = avrcpGrabSink(sink, 6);
               
        if (ptr == NULL)
        {
		    avrcpSendPassthroughCfmToClient(avrcp, avrcp_no_resource);
            SourceEmpty(req->operation_data);
            return;
        }
                   
		/* Write end packet */
		ptr[0] = (avrcp->transaction_label << AVCTP0_TRANSACTION_SHIFT) | AVCTP0_PACKET_TYPE_END | AVCTP0_CR_COMMAND;

		avrcpSetPassThroughHeader(&ptr[1], req);
		
		/* Copy data. i is the data length remaining for the end packet */
		i = req->operation_data_length - ((max_mtu - 9) + (max_mtu - 6) * no_complete_packets);
		ptr[5] = i;
          
        stream_move = StreamMove(sink, req->operation_data, i);   
        if (stream_move != i)
        {
            avrcpSendPassthroughCfmToClient(avrcp, avrcp_fail);
            packet_size = 6 + stream_move;
            (void)SinkFlush(sink, packet_size);
            SourceEmpty(req->operation_data);
            return;
        }
     
        continue_data_length = i;
        packet_size = 6 + i;
		
		/* Send the data */
		(void)SinkFlush(sink, packet_size);
     
		/* Set the state flags and watchdog timeout */
		avrcp->pending = avrcp_passthrough;
		avrcp->watchdog = 1;
		MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_FRAGMENTED_TIMEOUT);
	}
    SourceEmpty(req->operation_data);
}


/****************************************************************************
NAME	
	avrcpHandleReceivedData

DESCRIPTION
	This function is called to process data received over the L2cap connection
*/
void avrcpHandleReceivedData(AVRCP *avrcp)
{
    uint16 packet_size;
	Source source = StreamSourceFromSink(avrcp->sink);

	while (((packet_size = SourceBoundary(source)) != 0) && !avrcp->block_received_data)
	{
		const uint8 *ptr = SourceMap(source);
		
		if (packet_size > 6)
		{
			if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_SINGLE)
			{
				if ((ptr[0] & AVCTP0_CR_MASK) == AVCTP0_CR_COMMAND)
				{
					/* which profile? */
					if ((ptr[1] == AVCTP1_PROFILE_AVRCP_HIGH) &&
						(ptr[2] == AVCTP2_PROFILE_AVRCP_LOW))
					{
						if (ptr[5] == AVRCP2_VENDORDEPENDENT)
							avrcpHandleVendorMessage(avrcp, ptr, packet_size);
						else
							avrcpProcessCommand(avrcp, ptr, packet_size);
					}
					else
						avrcpSendResponse(avrcp, ptr, AVCTP_HEADER_SIZE_SINGLE_PKT_OCTETS, avctp_response_bad_profile);
				}
				else
				{
					/* which profile? */
					if ((ptr[1] == AVCTP1_PROFILE_AVRCP_HIGH) &&
						(ptr[2] == AVCTP2_PROFILE_AVRCP_LOW))
						avrcpProcessResponse(avrcp, ptr);
					/* else unknown profile - ignore */
				}
			}
			else
			{
				uint16 ptrType = 5; /* default */

				if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START)
					ptrType = 6;
				
				if (((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_CONTINUE) ||
					((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_END))
					ptrType = 3;
				
				if (ptr[ptrType] != AVRCP2_VENDORDEPENDENT)
					avrcpHandleReceivedFragmentedMessage(avrcp, ptr, packet_size);
				else
					avrcpHandleVendorMessage(avrcp, ptr, packet_size);
			}
		}		
		/*
			Packet is too small to be valid - ignore! We can't use AVCTPSendResponse 
			as the packet is too small to send back anything reasonable.
		*/
		
		/* Discard the packet. */
		SourceDrop(source, packet_size);
	}
}


/****************************************************************************
NAME	
	avrcpProcessCommand

DESCRIPTION
	This function is called to process an AVRCP Command packet
*/
void avrcpProcessCommand(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{
	if ((ptr[3] == AVRCP0_CTYPE_CONTROL) && (ptr[5] == AVRCP2_PASSTHROUGH))
	{
		/* Check packet has a valid header and payload if any */
		if ((packet_size < (AVRCP_TOTAL_HEADER_SIZE + 2)) ||
			(packet_size < (AVRCP_TOTAL_HEADER_SIZE + ptr[7])))
		{
			/* Reject the response - invalid header or payload */
			avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
		}
		else
		{
			/* 
				Any reply must contain the original message. We 'secretly' pass this 
				to the application as an identifier that it must pass back for the response. 
			*/
			avrcp->identifier = malloc(packet_size);

			if (avrcp->identifier)
			{
				/* Copy the message into the buffer */
                avrcp->block_received_data = 1;
				memmove(avrcp->identifier, ptr, packet_size);
				
				/* Send the message to the client */
				avrcpSendPassthroughIndToClient(avrcp, 
					(avc_operation_id) (ptr[6] & AVRCP3_PASSTHROUGH_OP_MASK), 
					(ptr[6] & AVRCP3_PASSTHROUGH_STATE_MASK ? 1 : 0), 
					(avc_subunit_type) ((ptr[4] & AVRCP1_SUBUNIT_TYPE_MASK) >> AVRCP1_SUBUNIT_TYPE_SHIFT), 
					ptr[4] & AVRCP1_SUBUNIT_ID_MASK, 0, 0, avrcp->identifier[7], avrcp->identifier[7] ? &avrcp->identifier[8] : 0);
			}
			else
			{
				/* Failed to allocate memory to pass message to client */
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
			}
		}
	}
	else if ((ptr[3] == AVRCP0_CTYPE_STATUS) && (ptr[5] == AVRCP2_UNITINFO))
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

				/* Copy the message into the buffer and store with the profile instance data */
                avrcp->block_received_data = 1;
				memmove(avrcp->identifier, ptr, packet_size);
			}
			else
			{
				/* Failed to allocate memory to pass message to client */
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
			}
		}
	}
	else if ((ptr[3] == AVRCP0_CTYPE_STATUS) && (ptr[5] == AVRCP2_SUBUNITINFO))
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

				/* Copy the message into the buffer */
                avrcp->block_received_data = 1;
				memmove(avrcp->identifier, ptr, packet_size);
			}
			else
			{
				/* Failed to allocate memory to pass message to client */
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
			}
		}
	}
	else
	{
		/* Reject this its an unknown command */
		avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_not_implemented);
	}
}


/****************************************************************************
NAME	
	avrcpHandleReceivedFragmentedMessage

DESCRIPTION
	
*/
void avrcpHandleReceivedFragmentedMessage(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{
	if (avrcp->fragmented == avrcp_frag_end)
		avrcp->fragmented = avrcp_frag_none;
	
	if ((ptr[0] & AVCTP0_CR_MASK) == AVCTP0_CR_COMMAND)
	{
		if (((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START))
		{
			/* Check packet has a valid header and payload if any */
			if (avrcp->fragmented != avrcp_frag_none)
			{
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
			}

			/* 
				Any reply must contain the original message. We 'secretly' pass this 
				to the application as an identifier that it must pass back for the response. 
			*/
			avrcp->identifier = malloc(packet_size);
			
			if (avrcp->identifier)
			{	
				/* Copy the message into the buffer */
                avrcp->block_received_data = 1;
				memmove(avrcp->identifier, ptr, packet_size);

				avrcpSendPassthroughIndToClient(avrcp, 
					(avc_operation_id) (ptr[7] & AVRCP3_PASSTHROUGH_OP_MASK), 
					(ptr[7] & AVRCP3_PASSTHROUGH_STATE_MASK ? 1 : 0), 
					(avc_subunit_type) ((ptr[5] & AVRCP1_SUBUNIT_TYPE_MASK) >> AVRCP1_SUBUNIT_TYPE_SHIFT), 
					ptr[5] & AVRCP1_SUBUNIT_ID_MASK, 
					(ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, ptr[1], 
					avrcp->identifier[8], avrcp->identifier[8] ? &avrcp->identifier[9] : 0);
				
				if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START)
					avrcp->fragmented = avrcp_frag_start;
			}
			else
			{
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
			}
		}
		else if (((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_CONTINUE) ||
				((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_END))
		{
			/* Check packet has a valid header and payload if any */
			if ((avrcp->fragmented != avrcp_frag_start) &&
				(avrcp->fragmented != avrcp_frag_continue))
			{
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
			}

			/* 
				Any reply must contain the original message. We 'secretly' pass this to 
				the application as an identifier that it must pass back for the response. 
			*/
			avrcp->identifier = malloc(packet_size);
			
			if (avrcp->identifier)
			{			
				/* Copy the message into the buffer */
                avrcp->block_received_data = 1;
				memmove(avrcp->identifier, ptr, packet_size);

				/* Send the message to the client */
				avrcpSendPassthroughIndToClient(avrcp, 
					(avc_operation_id) (ptr[4] & AVRCP3_PASSTHROUGH_OP_MASK), 
					(ptr[4] & AVRCP3_PASSTHROUGH_STATE_MASK ? 1 : 0), 
					(avc_subunit_type) ((ptr[2] & AVRCP1_SUBUNIT_TYPE_MASK) >> AVRCP1_SUBUNIT_TYPE_SHIFT), 
					ptr[2] & AVRCP1_SUBUNIT_ID_MASK, 
					(ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, 0, 
					avrcp->identifier[5], avrcp->identifier[5] ? &avrcp->identifier[6] : 0);
				
				if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_CONTINUE)
					avrcp->fragmented = avrcp_frag_continue;
				else
					avrcp->fragmented = avrcp_frag_end;
			}
			else
			{
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
			}
		}
	}
	else
	{
		/* Response, find correct slots for info depending on packet type */
		uint16 ptrType = 5, ptrResponse = 3; /* default */
	
		if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START)
		{
			ptrType = 6;
			ptrResponse = 4;
		}
	
		if (((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_CONTINUE) ||
			((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_END))
		{
			ptrType = 3;
			ptrResponse = 1;
		}
		
		/* Check that we were expecting this response and if so process it. */
		if ((avrcp->pending == avrcp_passthrough) && (ptr[ptrType] == AVRCP2_PASSTHROUGH))
		{
			if (ptr[ptrResponse] == avctp_response_accepted)
				avrcpSendPassthroughCfmToClient(avrcp, avrcp_success);
			else if (ptr[ptrResponse] == avctp_response_not_implemented)
				avrcpSendPassthroughCfmToClient(avrcp, avrcp_unsupported);
			else
				/* default to a fail handles avctp_response_rejected etc */
				avrcpSendPassthroughCfmToClient(avrcp, avrcp_fail);
			
			(void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);

			if (((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_END))
			{
				/* No longer waiting */
				avrcp->pending = avrcp_none;
				avrcp->watchdog = 0;		
			}
			else
				MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_FRAGMENTED_TIMEOUT);
		}
	}
}


/****************************************************************************
NAME	
	avrcpSendResponse
	
DESCRIPTION
	This function is used to send a response back to the CT device.
	Currently doesn't handle a packet size > max_mtu as the received 
	packet should always be <= max_mtu, and split into a start, continue 
	and end packet if neccessary.
*/
void avrcpSendResponse(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size, avrcp_response_type response)
{
    Sink sink = avrcp->sink;
	uint8 *res = avrcpGrabSink(sink, packet_size);
    
    if (res == NULL)
    {
		/* Nothing to do */
    }
    else
    {
        /* Copy the original message */
        memmove(res, ptr, packet_size);

        /* bad profile error so set the invalid PID bit, and don't send any response message */
        if (response == avctp_response_bad_profile)
        {
            res[0] |= AVCTP0_IPID;
        }
        else
        {
            /* Overwrite command type with response */
            if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START)
                res[4] = response;
            else if (((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_CONTINUE) ||
                    ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_END))
                res[1] = response;
            else
                res[3] = response;
        }

        /* flip CR bit */
        res[0] ^= AVCTP0_CR_MASK;

        /* Send response */ 
        (void) SinkFlush(sink, packet_size);
    }

    avrcp->block_received_data = 0;
}


/****************************************************************************
NAME	
	avrcpProcessResponse

DESCRIPTION
	This function is called to process an AVRCP Response packet
*/
void avrcpProcessResponse(AVRCP *avrcp, const uint8 *ptr)
{
	/* Check that we were expecting this response and if so process it. */
	if ((avrcp->pending == avrcp_passthrough) && (ptr[5] == AVRCP2_PASSTHROUGH))
	{
		if (ptr[3] == avctp_response_accepted)
			avrcpSendPassthroughCfmToClient(avrcp, avrcp_success);
		else if (ptr[3] == avctp_response_not_implemented)
			avrcpSendPassthroughCfmToClient(avrcp, avrcp_unsupported);
		else
			/* Default to a fail handles avctp_response_rejected etc */
			avrcpSendPassthroughCfmToClient(avrcp, avrcp_fail);

		/* No longer waiting */
		avrcp->pending = avrcp_none;
		avrcp->watchdog = 0;
		(void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
	}
	else if ((avrcp->pending == avrcp_unit_info) && (ptr[5] == AVRCP2_UNITINFO))
	{
		if (ptr[3] == avctp_response_stable)
			avrcpSendUnitInfoCfmToClient(avrcp, avrcp_success, 
				ptr[7] >> AVRCP4_UNITINFO_UNIT_TYPE_SHIFT, 
				ptr[7] & AVRCP4_UNITINFO_UNIT_MASK,
				avrcpConvertUint8ValuesToUint32(&ptr[8]));				
		else
			avrcpSendUnitInfoCfmToClient(avrcp, avrcp_fail, 0, 0, (uint32) 0);

		/* No longer waiting */
		avrcp->pending = avrcp_none;
		avrcp->watchdog = 0;
		(void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
	}
	else if ((avrcp->pending == avrcp_subunit_info) && (ptr[5] == AVRCP2_SUBUNITINFO))
	{
		if (ptr[3] == avctp_response_stable)
			avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_success, 
				ptr[6] >> AVRCP3_SUBUNITINFO_PAGE_SHIFT, &ptr[7]);
		else
			avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_fail, 0, 0);

		/* No longer waiting */
		avrcp->pending = avrcp_none;
		avrcp->watchdog = 0;
		(void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);	
	}
	else if (avrcp->pending == avrcp_vendor)
	{
		/* Response, find correct slots for info depending on packet type */
		uint16 ptrType = 5, ptrResponse = 3; /* default */

		if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START)
		{
			ptrType = 6;
			ptrResponse = 4;
		}
		
		if (((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_CONTINUE) ||
			((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_END))
		{
			ptrType = 3;
			ptrResponse = 1;
		}
		
		/* Check that we were expecting this response and if so process it. */
		if (ptr[ptrType] == AVRCP2_VENDORDEPENDENT)
		{
			avrcpSendVendordependentCfmToClient(avrcp, avrcp_success, ptr[ptrResponse]);

			/* No longer waiting */
			avrcp->pending = avrcp_none;
			avrcp->watchdog = 0;
			(void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
		}
	}
	else
	{
		/* Unhandled response: Ignore */
	}
}


/****************************************************************************
NAME	
	avrcpHandleVendorMessage

DESCRIPTION
	This function process a vendor-dependent message that is received over the L2cap connection
*/
void avrcpHandleVendorMessage(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{
	if (avrcp->fragmented == avrcp_frag_end)
		avrcp->fragmented = avrcp_frag_none;
	
	if ((ptr[0] & AVCTP0_CR_MASK) == AVCTP0_CR_COMMAND)
	{
		if (((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START) ||
				((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_SINGLE))
		{	
			uint32 cid = 0;
			uint8 addition = 0;
	
			/* Check packet has a valid header and payload if any */
			if (avrcp->fragmented != avrcp_frag_none)
			{
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
			}
			
			if(((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START))
				addition = 1;
			
			/* 
				Any reply must contain the original message. We 'secretly' pass this to 
				the application as an identifier that it must pass back for the response. 
			*/
			avrcp->identifier = malloc(packet_size);
			
			if (avrcp->identifier)
			{
				uint16 oplen = packet_size - (9 + addition);

				/* Copy the data into a buffer */
                avrcp->block_received_data = 1;
				memmove(avrcp->identifier, ptr, packet_size);

				/* Pull out the company id */
				cid = avrcpConvertUint8ValuesToUint32(&ptr[6+addition]);
                
                /* temporarily replace type of message (AVRCP2_VENDORDEPENDENT) with packet length (used for response) */
                avrcp->identifier[5 + addition] = packet_size;

				/* Send the ind message to the client */
				avrcpSendVendordependentIndToClient(avrcp, 
					(ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, 
					(uint16) (addition ? ptr[1] : 0), 
					(avc_subunit_type) ((ptr[4 + addition] & AVRCP1_SUBUNIT_TYPE_MASK) >> AVRCP1_SUBUNIT_TYPE_SHIFT), 
					ptr[4 + addition] & AVRCP1_SUBUNIT_ID_MASK, 
					cid, ptr[3 + addition], oplen, oplen ? &ptr[9 + addition] : 0);
							
				if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START)
					avrcp->fragmented = avrcp_frag_start;
			}
			else
			{
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
			}
		}
		else
		if (((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_CONTINUE) ||
			((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_END))
		{	
			uint32 cid = 0;

			/* Check packet has a valid header and payload if any */
			if ((avrcp->fragmented != avrcp_frag_start) &&
				(avrcp->fragmented != avrcp_frag_continue))
			{
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
			}
			
			/* 
				Any reply must contain the original message. Store this in the profile 
				instance data.
			*/
			avrcp->identifier = malloc(packet_size);
			
			if (avrcp->identifier)
			{
				uint16 oplen;

				/* Get the company id */
				cid = avrcpConvertUint8ValuesToUint32(&ptr[4]);

				oplen = packet_size - 7;

				/* Copy the message into the buffer */
                avrcp->block_received_data = 1;
				memmove(avrcp->identifier, ptr, packet_size);
                
                /* temporarily replace type of message (AVRCP2_VENDORDEPENDENT) with packet length (used for response) */
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
					avrcp->fragmented = avrcp_frag_continue;
				else
					avrcp->fragmented = avrcp_frag_end;
			}
			else
			{
				avrcpSendResponse(avrcp, ptr, packet_size, avctp_response_rejected);
			}
		}
	}
	else
	{
		/* Response, find correct slots for info depending on packet type */
		uint16 ptrType = 5, ptrResponse = 3; /* default */

		if ((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_START)
		{
			ptrType = 6;
			ptrResponse = 4;
		}
		
		if (((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_CONTINUE) ||
			((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_END))
		{
			ptrType = 3;
			ptrResponse = 1;
		}
		
		/* Check that we were expecting this response and if so process it. */
		if ((avrcp->pending == avrcp_vendor) && (ptr[ptrType] == AVRCP2_VENDORDEPENDENT))
		{			
			avrcpSendVendordependentCfmToClient(avrcp, avrcp_success, ptr[ptrResponse]);
			
			(void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);

			if (((ptr[0] & AVCTP0_PACKET_TYPE_MASK) == AVCTP0_PACKET_TYPE_END))
			{
				/* No longer waiting */
				avrcp->pending = avrcp_none;
				avrcp->watchdog = 0;
			}
			else
				MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_FRAGMENTED_TIMEOUT);
		}
	}
}


/****************************************************************************
NAME	
	avrcpHandleInternalPassThroughRes

DESCRIPTION
	This function internally handles a pass through message result
*/
void avrcpHandleInternalPassThroughRes(AVRCP *avrcp, const AVRCP_INTERNAL_PASSTHROUGH_RES_T *res)
{
	if (!avrcp->identifier)
	{
		avrcpSendPassthroughCfmToClient(avrcp, avrcp_fail);
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
		else
		{
			avrcpSendPassthroughCfmToClient(avrcp, avrcp_invalid_sink);
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


/****************************************************************************
NAME	
	avrcpHandleInternalWatchdogTimeout

DESCRIPTION
	Called if the watchdog times out by the CT not receiving a response within
	the alloted time
*/
void avrcpHandleInternalWatchdogTimeout(AVRCP *avrcp)
{
	if (avrcp->sink)
	{
		if (avrcp->pending == avrcp_passthrough)
			avrcpSendPassthroughCfmToClient(avrcp, avrcp_timeout);
		else if (avrcp->pending == avrcp_unit_info)
			avrcpSendUnitInfoCfmToClient(avrcp, avrcp_timeout, 0, 0, (uint32) 0);
		else if (avrcp->pending == avrcp_subunit_info)
			avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_timeout, 0, 0);
		else if (avrcp->pending == avrcp_vendor)
			avrcpSendVendordependentCfmToClient(avrcp, avrcp_timeout, 0);

		avrcp->watchdog = 0;
	}
}


/****************************************************************************
NAME	
	avrcpHandleInternalUnitInfoReq

DESCRIPTION
	This function internally handles unit info message request
*/
void avrcpHandleInternalUnitInfoReq(AVRCP *avrcp)
{
	uint16 packet_size = 5 + AVRCP_TOTAL_HEADER_SIZE;
    
    Sink sink = avrcp->sink;
    uint8* ptr = avrcpGrabSink(sink, packet_size);

	if (avrcp->watchdog != 0)
		avrcpSendUnitInfoCfmToClient(avrcp, avrcp_busy, 0, 0, (uint32) 0);
	else if (ptr == NULL)
		avrcpSendUnitInfoCfmToClient(avrcp, avrcp_no_resource, 0, 0, (uint32) 0);
	else
	{
		/* AVCTP header */
		getNextTransactionLabel(avrcp);
		avrcpSetAvctpHeader(avrcp, &ptr[0], 0);

		/* AVRCP header */
		ptr[3] = AVRCP0_CTYPE_STATUS;
		ptr[4] = AVRCP1_UNIT;
		ptr[5] = AVRCP2_UNITINFO;

		/* UnitInfo set to all f's */
		memset(&ptr[6], 0xff, 5);

		/* Send the data */
		(void)SinkFlush(sink,packet_size);

		avrcp->pending = avrcp_unit_info;
		avrcp->watchdog = 1;
		MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_TIMEOUT);
	}	
}


/****************************************************************************
NAME	
	avrcpHandleInternalUnitInfoRes

DESCRIPTION
	This function internally handles unit info message result
*/
void avrcpHandleInternalUnitInfoRes(AVRCP *avrcp, const AVRCP_INTERNAL_UNITINFO_RES_T *res)
{
	if (!avrcp->identifier)
	{
		avrcpSendUnitInfoCfmToClient(avrcp, avrcp_fail, 0, 0, (uint32) 0);
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
		else
		{
			avrcpSendUnitInfoCfmToClient(avrcp, avrcp_invalid_sink, 0, 0, (uint32) 0);
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


/****************************************************************************
NAME	
	avrcpHandleInternalSubUnitInfoReq

DESCRIPTION
	This function internally handles subunit info message request
*/
void avrcpHandleInternalSubUnitInfoReq(AVRCP *avrcp, const AVRCP_INTERNAL_SUBUNITINFO_REQ_T *req)
{
	uint16 packet_size = 5 + AVRCP_TOTAL_HEADER_SIZE;
    
    Sink sink = avrcp->sink;
    uint8* ptr = avrcpGrabSink(sink, packet_size);

	if (avrcp->watchdog)
		avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_busy, 0, 0);
	else if (ptr == NULL)
		avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_no_resource, 0, 0);
	else
	{
		/* AVCTP header */
		getNextTransactionLabel(avrcp);
		avrcpSetAvctpHeader(avrcp, &ptr[0], 0);

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
		avrcp->watchdog = 1;
		MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_TIMEOUT);
	}	
}


/****************************************************************************
NAME	
	avrcpHandleInternalSubUnitInfoRes

DESCRIPTION
	This function internally handles subunit info message result
*/
void avrcpHandleInternalSubUnitInfoRes(AVRCP *avrcp, const AVRCP_INTERNAL_SUBUNITINFO_RES_T *res)
{
	if (!avrcp->identifier)
	{
		avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_fail, 0, 0);
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
				memmove(&avrcp->identifier[7], &res->page_data[0], PAGE_DATA_LENGTH);
				
				avrcpSendResponse(avrcp, avrcp->identifier, AVRCP_TOTAL_HEADER_SIZE + 5, avctp_response_stable);
			}
		}
        else
		{
			avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_fail, 0, 0);
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


/****************************************************************************
NAME	
	avrcpHandleInternalVendorDependentReq

DESCRIPTION
	This function internally handles vendor-dependent message request
*/
void avrcpHandleInternalVendorDependentReq(AVRCP *avrcp, const AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *req)
{
	uint16 packet_size = 3 + AVRCP_TOTAL_HEADER_SIZE + req->data_length;
    
    Sink sink = avrcp->sink;
    uint8* ptr;
	uint16 max_mtu = avrcp->l2cap_mtu;
	uint16 no_complete_packets = 1;
    uint16 start_data_length = 0;
    uint16 continue_data_length = 0;
	uint16 i = 0;
    uint16 stream_move;
    uint16 data_length = 0;
	avrcp_status_code result = avrcp_success;
    
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

	if (avrcp->watchdog)
	{
		result = avrcp_busy;
		goto IntVendorDepReqError;
	}	
	else
	{
		getNextTransactionLabel(avrcp);
 
		if (packet_size <= max_mtu)
		{
			ptr = avrcpGrabSink(sink, 9);
         
            if (ptr == NULL)
            {
				result = avrcp_no_resource;
				goto IntVendorDepReqError;		                        
	        }
           
            /* AVCTP header */
			avrcpSetAvctpHeader(avrcp, &ptr[0], 0);

			/* AVRCP header */
			avrcpSetVendorPacketHeader(&ptr[3], req);
            
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
		}
		else
		{
			ptr = avrcpGrabSink(sink, 10);
            
            if (ptr == NULL)
            {
		        result = avrcp_no_resource;
				goto IntVendorDepReqError;                
	        }
            
			/* Write start packet with info on the number of packets to expect */
			avrcpSetAvctpHeader(avrcp, &ptr[0], no_complete_packets + 2);

			/* AVRCP header */
			avrcpSetVendorPacketHeader(&ptr[4], req);
	
			/* Copy data */
            stream_move = StreamMove(sink, req->data, start_data_length);
            if (stream_move != start_data_length)
            {
                /* Data source can't be moved into Sink as expected so return failure */                
                packet_size = 10 + stream_move;
                (void)SinkFlush(sink, packet_size);
                result = avrcp_fail;
				goto IntVendorDepReqError;
                return;
            }
						
			(void) SinkFlush(sink,max_mtu);
         
			for (i = 0; i < no_complete_packets; i++)
			{                
				ptr = avrcpGrabSink(sink, 7);

                if (ptr == NULL)
                {
		            result = avrcp_no_resource;
					goto IntVendorDepReqError;
	            }                
                
				/* Write continue packets */
				ptr[0] = (avrcp->transaction_label << AVCTP0_TRANSACTION_SHIFT) | AVCTP0_PACKET_TYPE_CONTINUE | AVCTP0_CR_COMMAND;

				/* AVRCP header */
				avrcpSetVendorPacketHeader(&ptr[1], req);

                stream_move = StreamMove(sink, req->data, continue_data_length);  
                if (stream_move != continue_data_length)
                {
                    /* Data source can't be moved into Sink as expected so return failure */                    
                    packet_size = 7 + stream_move;
                    (void)SinkFlush(sink, packet_size);
                    result = avrcp_fail;
					goto IntVendorDepReqError;
                }
				
				(void)SinkFlush(sink, max_mtu);
              
			}

            i = req->data_length - ((max_mtu - 10) + (max_mtu - 7) * no_complete_packets);
                       
			ptr = avrcpGrabSink(sink, 7);
                
            if (ptr == NULL)
            {
		        result = avrcp_no_resource;
				goto IntVendorDepReqError;
                return;
            }
                      
			/* Write end packet */
			ptr[0] = (avrcp->transaction_label << AVCTP0_TRANSACTION_SHIFT) | AVCTP0_PACKET_TYPE_END | AVCTP0_CR_COMMAND;

			/* AVRCP header */
			avrcpSetVendorPacketHeader(&ptr[1], req);

			/* Copy data. i is the data length remaining for the end packet */
            stream_move = StreamMove(sink, req->data, i);
            if (stream_move != i)
            {
                /* Data source can't be moved into Sink as expected so return failure */                
                packet_size = 7 + stream_move;
                (void)SinkFlush(sink, packet_size);
                result = avrcp_fail;
				goto IntVendorDepReqError;
            }
		
			/* Send the data */
			(void) SinkFlush(sink, i+7);
		}

		avrcp->pending = avrcp_vendor;
		avrcp->watchdog = 1;
		MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 0, AVCTP_WATCHDOG_FRAGMENTED_TIMEOUT);
	}

IntVendorDepReqError:
	SourceEmpty(req->data);

	if (result != avrcp_success)
	{
		avrcpSendVendordependentCfmToClient(avrcp, result, 0);
	}
}


/****************************************************************************
NAME	
	avrcpHandleInternalVendorDependentRes

DESCRIPTION
	This function internally handles vendor-dependent message result
*/
void avrcpHandleInternalVendorDependentRes(AVRCP *avrcp, const AVRCP_INTERNAL_VENDORDEPENDENT_RES_T *res)
{
	uint16 packet_size;
    uint16 size_pos;
	
	if (avrcp->identifier)
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

/*lint +e641 +e572 */
