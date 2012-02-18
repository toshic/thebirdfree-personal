/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_send_packet_handler.c        

DESCRIPTION

NOTES

*/



/****************************************************************************
	Header files
*/

#include "a2dp_send_packet_handler.h"

#include <sink.h>
#include <string.h>
#include <stdlib.h>


/****************************************************************************/
uint8 *a2dpGrabSink(Sink sink, uint16 size)
{
	uint8 *dest = SinkMap(sink);
	uint16 claim_result = SinkClaim(sink, size);
    if (claim_result == 0xffff)
    {
        return NULL;
	}

    return (dest + claim_result);
}


/****************************************************************************/
bool a2dpSendPacket(Sink sink, uint16 mtu, uint16 length)
{
    if (mtu >= length)
    {
        /* packet fits in MTU, send it */
        return SinkFlush(sink, length);
    }
    else
    {
        /*
            Fragment the packet.
        */
        uint8 *ptr;
        uint8 header;
        uint16 i;

        /* Calculate the number of fragments needed. */
        uint16 fragments = (length+mtu-2) / (mtu-1);

        /* Claim more space to add the header per fragment */
        (void) PanicZero(SinkClaim(sink,fragments));

        /*
            Start Packet
        */
        ptr = SinkMap(sink);
        
		/* change initial header to start */
        header = (ptr[0] & 0xf3);
        ptr[0] |= header | (avdtp_packet_type_start << 2);
        
		/* slide the data down the sink to make room for the header. */
        memmove(&ptr[2],&ptr[1],length-1);
        
		/* fill in the number of fragments field */
        ptr[1] = fragments;
        
		/* flush the first packet */
        (void) SinkFlush(sink, mtu);
        
		/* adjust remaining length */
        length -= mtu-1;

        /*
            Continuation Packets
        */
        for(i=0;i<fragments-2;i++)
        {
            ptr = SinkMap(sink);
            /* slide the data down the sink to make room for the header. */
            memmove(&ptr[1],&ptr[0],length);

            /* fill in header */
            ptr[0] = header | (avdtp_packet_type_continue << 2);

            /* flush the packet packet */
            (void) SinkFlush(sink, mtu);

            /* adjust remaining length */
            length -= mtu-1;
        }

        /*
            End Packet
        */
        ptr = SinkMap(sink);

        /* slide the data down the sink to make room for the header. */
        memmove(&ptr[1],&ptr[0],length);

        /* fill in header */
        ptr[0] = header | (avdtp_packet_type_end << 2);

        /* flush the final packet */
        (void) SinkFlush(sink,length+1);
    }

    return FALSE;
}

