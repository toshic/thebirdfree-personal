/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_avctp.c

DESCRIPTION
    Functions in this file handles AVCTP protocol transactions and 
    AVCTP message handling.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "avrcp_common.h"

/****************************************************************************
*NAME    
*    avrcpAvctpSetCmdHeader    
*
*DESCRIPTION
*    This function is called to set the AVCTP header for START or SINGLE Packet
*    for commands.For Continue or End Packet call the Macro 
*    AVCTP_SET_PKT_HEADER.
*    
*PARAMETERS
*   avrcp         - Task
*   *ptr          - Pointer to the Start of AVCTP Packet in Sink 
*   packet_type   - MUST be AVCTP0_PACKET_TYPE_SINGLE/AVCTP0_PACKET_TYPE_START.
*   total_packets - Total Number of Packets
*
*RETURN
*******************************************************************************/
void avrcpAvctpSetCmdHeader(AVRCP *avrcp, 
                            uint8 *ptr, 
                            uint8 packet_type, 
                            uint8 total_packets)
{
    /* AVCTP header */
    ptr[0] = (avrcpGetNextTransactionLabel(avrcp) << AVCTP0_TRANSACTION_SHIFT)
             | packet_type | AVCTP0_CR_COMMAND;
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



/****************************************************************************
*NAME    
*    avrcpAvctpSendMessage    
*
*DESCRIPTION
*    This function is called to Frame the AVCTP messages (Command or Response)
*   and send it to the peer. Before calling this function, fill the AVRCP 
*   Message  Header in the sink. For sending a response, copy the AVCTP 
*   header received to *ptr before calling the function. Set the Profile ID 
*   in the response. Set the hdr_size=0 (no avrcp header) for
*   bad_profile response.   
*    
*PARAMETERS
*   avrcp            - Task
*   cr_type          - set to 0 for command and 2 for response    
*   ptr              - Pointer to the Start of AVCTP Packet in Sink 
*   hdr_size         - Header size of AVRCP message already in the sink 
*   data_len         - Length of AVRCP message data to send
*   data             - AVRCP Message data to append after AVRCP message head    
*
*RETURN
*   avrcp_status_code
*******************************************************************************/

avrcp_status_code avrcpAvctpSendMessage( AVRCP     *avrcp,
                                         uint8      cr_type,
                                         uint8     *ptr,
                                         uint16     hdr_size,
                                         uint16     data_len,
                                         Source     data)   
{
    uint8  avctp_pkt_type = AVCTP0_PACKET_TYPE_SINGLE;
    uint8  i, start_head;
    uint8  no_complete_packets=0;
    uint16 msg_len=data_len;
    uint16 pkt_size = hdr_size+data_len+AVCTP_SINGLE_PKT_HEADER_SIZE;
    avrcp_status_code result = avrcp_success;
    Sink sink=avrcp->sink;

    /* 
     * Check the packet size is greater than L2CAP MTU Size. Fragmentation is 
     * required for AVCTP packets greater than MTU Size
     */
    if(pkt_size >  avrcp->l2cap_mtu)
    {
        avctp_pkt_type = AVCTP0_PACKET_TYPE_START;
    }

    if(avctp_pkt_type ==  AVCTP0_PACKET_TYPE_START)
    {
        msg_len = avrcp->l2cap_mtu - (hdr_size + AVCTP_START_PKT_HEADER_SIZE);

        /* Calculate the Number of complete_packets.*/
        no_complete_packets = ((data_len - msg_len ) / 
                           (avrcp->l2cap_mtu - AVCTP_CONT_PKT_HEADER_SIZE)) +1;

        /* If there is a reminder of bytes add 1 more */
        if((data_len - msg_len ) % 
            (avrcp->l2cap_mtu - AVCTP_CONT_PKT_HEADER_SIZE))
            no_complete_packets++;
    
        /* Packet size */
        pkt_size = avrcp->l2cap_mtu ;

        /* Fill no_complete_packets */
        ptr[AVCTP_NUM_PKT_OFFSET] = no_complete_packets;
    }

    /* Fill the AVCTP Header */
    if(cr_type == AVCTP0_CR_COMMAND)
    {
        avrcpAvctpSetCmdHeader( avrcp,
                                 &ptr[AVCTP_HEADER_START_OFFSET],
                                  avctp_pkt_type, 
                                  no_complete_packets);
    }
    else
    { 
        /* Frame the header */
        ptr[AVCTP_HEADER_START_OFFSET] =
            (ptr[AVCTP_HEADER_START_OFFSET] & AVCTP_TRANSACTION_MASK) | 
            avctp_pkt_type | AVCTP0_CR_RESPONSE;

        
        /* Set Bad Profile if there is no AVRCP header to follow */
        if(!hdr_size)
        {
            ptr[AVCTP_HEADER_START_OFFSET] |= AVCTP0_IPID;
        } 

    }

    /* Store first octet for future */
    start_head = ptr[AVCTP_HEADER_START_OFFSET];

    /* Before calling this function , Sink Space must be checked for 
     * first message If StreamMove() fails, Target may not get the 
     * entire data and it may drop 
     */
      
    if(msg_len)
    {
        StreamMove(sink, data, msg_len);

        /* Reduce the data length */
        data_len -= msg_len;
    }

    /* Send the data */
    (void)SinkFlush(sink, pkt_size);

    /* Send the Rest of AVCTP Fragments. Start with 2 since we already sent 1 */    
    for (i = 2 ; i <= no_complete_packets; i++)
    {
        if(!(ptr= avrcpGrabSink(sink, AVCTP_CONT_PKT_HEADER_SIZE)))
        {
            result = avrcp_no_resource;
            break;
        }
        
        if(i < no_complete_packets)
        {
            AVCTP_SET_CONT_PKT_HEADER(ptr[AVCTP_HEADER_START_OFFSET],
                                      start_head);
            msg_len = avrcp->l2cap_mtu - AVCTP_CONT_PKT_HEADER_SIZE;
        }
        else
        {
            AVCTP_SET_END_PKT_HEADER(ptr[AVCTP_HEADER_START_OFFSET],start_head);
            msg_len = data_len;
            pkt_size = msg_len + AVCTP_END_PKT_HEADER_SIZE;
        }

        /*  Copy the data to Sink. Sink Space must be validated 
            before calling this function.*/
        StreamMove(sink, data , msg_len); 

           /* Send the data */
        (void)SinkFlush(sink, pkt_size);

        data_len -= msg_len;
    }

    /* If Still Data to send in Source. Drop that much data */
    if (data_len)
    {
        SourceDrop(data,data_len);
    }

    return result;
}

/****************************************************************************
*NAME    
*    avrcpAvctpReceiveMessage    
*
*DESCRIPTION
*    This function is called to process the AVCTP message received. 
*    
*PARAMETERS
*   avrcp                  - Task
*   * ptr                  - Received Data
*   packet_size            - packet_size
*
*RETURN
*  bool - Return TRUE if the message is ready to process, otherwise return 
*         FALSE
*******************************************************************************/
bool avrcpAvctpReceiveMessage(  AVRCP          *avrcp,
                                const uint8*    ptr,
                                uint16          packet_size)
{
    bool    result = FALSE;
    uint16  packet_type = ptr[AVCTP_HEADER_START_OFFSET] & 
                          AVCTP0_PACKET_TYPE_MASK;
    uint16  pid_offset = AVCTP_SINGLE_PKT_PID_OFFSET; 
    uint8*  temp;
    uint8   cr_type= ptr[0] & AVCTP0_CR_MASK; 


    switch(packet_type)
    {
    case AVCTP0_PACKET_TYPE_START:
        if(packet_size < AVRCP_START_PKT_HEADER_SIZE)
        {
           break; 
        }

        /* Check remote device is planning to send a Message greater than 
           AV Size */     
        if((((ptr[AVCTP_NUM_PKT_OFFSET]-1) * avrcp->l2cap_mtu) > 
             AVRCP_AVC_MAX_PKT_SIZE) || (ptr[AVCTP_NUM_PKT_OFFSET] <= 1))
        {
            if(cr_type == AVCTP0_CR_COMMAND)
            {
                avrcp->av_msg = (uint8*)ptr;
                avrcpSendAvcResponse(avrcp, AVRCP_START_PKT_HEADER_SIZE, 
                                    avctp_response_rejected, 0, NULL);
            }
            break;   
        }

        pid_offset =  AVCTP_START_PKT_PID_OFFSET;
        
    case AVCTP0_PACKET_TYPE_SINGLE: /* Fall through */
        if(packet_size < AVRCP_TOTAL_HEADER_SIZE)
        {
            break;
        }

        /* Check if we have received any fragmented pkts before */
        if(avrcp->av_msg_len)
        {
            /* Silently drop it */
            free(avrcp->av_msg);
            avrcp->av_msg_len = 0;
        }

        /* Check the PID Values */
        if (ptr[pid_offset] != AVCTP1_PROFILE_AVRCP_HIGH)
        {
            avrcp->av_msg = (uint8*)ptr;
            avrcpSendAvcResponse(avrcp,0,avctp_response_bad_profile, 0, NULL);
            break;
        }

        if(ptr[pid_offset+1] != AVCTP2_PROFILE_AVRCP_REMOTECONTROL)
        {
            if(((avrcp->device_type == avrcp_target) && 
                (ptr[pid_offset+1] != AVCTP2_PROFILE_AVRCP_CONTROLTARGET)) ||
                ((avrcp->device_type == avrcp_controller) && 
                (ptr[pid_offset+1] != AVCTP2_PROFILE_AVRCP_REMOTECONTROLLER)))
            {
                avrcp->av_msg = (uint8*)ptr;
                avrcpSendAvcResponse(avrcp,0,avctp_response_bad_profile,0,NULL);
                break;
            }
        }

        if(packet_type == AVCTP0_PACKET_TYPE_SINGLE)
        {
            avrcp->av_msg = (uint8*)ptr;
            avrcp->av_msg_len = packet_size;
            result = TRUE;
        }
        else
        {
            /* 
             * Allocate memory and copy the fragmented packet for reassembling 
             * This allocated memory is freed in avrcpSourceProcessed() after 
             * processing the packet. Memory is freed after sending the response
             * for command packets and for response packets it is freed after
             * processing the response. This memory may be send to application
             * if it contains any data interested to the app. In this case, app
             * shall call AvrcpSourceProcessed() to free it. 
             */
            avrcp->av_msg= (uint8*)PanicUnlessMalloc(packet_size);
            avrcp->av_msg_len=packet_size;
            memmove(avrcp->av_msg, ptr, packet_size);
            avrcp->av_msg[AVCTP_NUM_PKT_OFFSET]--;
        }

        break;

    case AVCTP0_PACKET_TYPE_CONTINUE:
    case AVCTP0_PACKET_TYPE_END:  
        if((avrcp->av_msg_len == 0)||(packet_size < AVCTP_CONT_PKT_HEADER_SIZE)
                       || (avrcp->av_msg[AVCTP_NUM_PKT_OFFSET] == 0) )
        {
            /* Bad Packet. Just ignore */
            break;
        }

      /* Check the transaction ID */
       if((ptr[AVCTP_HEADER_START_OFFSET] & AVCTP_TRANSACTION_MASK) !=
           (avrcp->av_msg[AVCTP_HEADER_START_OFFSET] & AVCTP_TRANSACTION_MASK)) 
            
       {
            /* Send the Response and Drop the packet */
            avrcpSendAvcResponse(avrcp, AVRCP_START_PKT_HEADER_SIZE, 
                                    avctp_response_rejected, 0, NULL);
            break;
       }


        /* Reassemble the Continuation Packet */
        temp = (uint8*)realloc(avrcp->av_msg, avrcp->av_msg_len+packet_size-1);
        if(!temp)
        {
            /* We failed to allocate memory . Stop receiving  further packets 
               and give this to app for processing */
            avrcp->av_msg[AVCTP_NUM_PKT_OFFSET] = 0;

            /* Panic in Debug Mode */
            AVRCP_DEBUG(("Failed to realloc memory for the received fragment"));
        }
        else
        {
            /* Ignore the 1 byte Header */  
            packet_size--;
            memmove(temp+avrcp->av_msg_len, &ptr[1],packet_size);

            avrcp->av_msg=temp;
            avrcp->av_msg_len+= packet_size;

            /* Reduce the number of packets */
            avrcp->av_msg[AVCTP_NUM_PKT_OFFSET]--;

        }
        
        
       if((packet_type == AVCTP0_PACKET_TYPE_END) || 
          (avrcp->av_msg[AVCTP_NUM_PKT_OFFSET] == 0))
       {
            /* Last packet Received */
            result = TRUE;
       }

       break;
    }

    return result;
}





