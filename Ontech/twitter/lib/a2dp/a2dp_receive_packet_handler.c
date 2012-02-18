/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_receive_packet_handler.c        

DESCRIPTION

NOTES

*/



/****************************************************************************
	Header files
*/

#include "a2dp_receive_command_handler.h"
#include "a2dp_receive_response_handler.h"
#include "a2dp_receive_packet_handler.h"
#include "a2dp_send_packet_handler.h"

#include <stdlib.h>
#include <sink.h>
#include <source.h>
#include <stream.h>
#include <string.h>


/****************************************************************************
NAME	
	handleBadHeader

DESCRIPTION
	This function is called when a complete signal packet is received which
	contains an invalid header.
  
RETURNS
	void
*/
static void handleBadHeader(const A2DP* a2dp, const uint8 *ptr, uint16 packet_size)
{
	uint8 pdu_size = 0;    
	Sink sink = a2dp->signal_conn.sink;
	uint8 *resp = a2dpGrabSink(sink, 3);

    if (!sink || !SinkIsValid(sink) || (resp == NULL))
        return;

	/* reject header */
	resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
	resp[1] = (packet_size > 1 ? ptr[1]:0);
	resp[2] = (uint8) avdtp_bad_header_format;
	pdu_size = 3;

    (void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, pdu_size));
}


/****************************************************************************
NAME	
	processSignalPacket

DESCRIPTION
	This function is called to process a complete signal packet receiving on
    the signalling channel
	
RETURNS
	void
*/
static bool processSignalPacket(A2DP* a2dp, const uint8* ptr, uint16 packet_size)
{
	/* decode header fields */
	switch (ptr[0] & 3)
	{
		case avdtp_message_type_command:
    		/* process the command */
    		if (!handleReceiveCommand(a2dp, ptr, packet_size))
				/* it was inappropriate to process the command, so go away and come back later */
       			return FALSE;
			break;
			
		case avdtp_message_type_accept:
		case avdtp_message_type_reject:
			handleReceiveResponse(a2dp, ptr, packet_size);
			break;

		default:
			handleBadHeader(a2dp, ptr, packet_size);
			break;
	}
	
	return TRUE;
}


/****************************************************************************
NAME	
	processSignalFragment

DESCRIPTION
	This function is called to process signal fragments arriving on the signalling
	channel.  It performs reassembly if needed and passed the signal to
    processSignalPacket()
	
RETURNS
	void
*/
static void processSignalFragment(A2DP* a2dp)
{
	uint16 packet_size;
	Source source = StreamSourceFromSink(a2dp->signal_conn.sink);

    if (!source || !SourceIsValid(source))
		return;

	while ((packet_size = SourceBoundary(source)) != 0)
	{
		const uint8 *ptr = SourceMap(source);
        uint16 type = (ptr[0]>>2) & 3;

        switch (type)
        {
            case avdtp_packet_type_single:
            {
                if (a2dp->signal_conn.reassembled_packet != NULL)
                {
                    /* 
                        We've received a single packet in the
                        middle of reassembling a fragment.
                        Discard the previous fragments.
                    */
                    free(a2dp->signal_conn.reassembled_packet);
                    a2dp->signal_conn.reassembled_packet = NULL;
                }

                /* complete packet, process it now */
                if (!processSignalPacket(a2dp, ptr, packet_size))
					/* exit early so we don't drop the packet */
					return;
            }
            break;

            case avdtp_packet_type_start:
            {
                if (a2dp->signal_conn.reassembled_packet != NULL)
                {
                    /*
                        We've received a start message part
                        way through reassembling another message.
                        Discard the old and start again.
                    */
                    free(a2dp->signal_conn.reassembled_packet);
                }

                /*
                    Allocate some memory to store the data until
                    the next fragment arrives.
                */
                a2dp->signal_conn.reassembled_packet = (uint8 *)malloc(packet_size-1);
                if (a2dp->signal_conn.reassembled_packet != NULL)
                {
                    /* copy the packet into RAM but discard the
                       Number of Fragments field as we don't trust the
                       remote device to set it correctly. */
                    a2dp->signal_conn.reassembled_packet[0] = ptr[0];
                    memmove(a2dp->signal_conn.reassembled_packet+1,ptr+2,packet_size-2);
                    a2dp->signal_conn.reassembled_packet_length = packet_size-1;
                }
            }
            break;

            case avdtp_packet_type_continue:
            case avdtp_packet_type_end:
            {
                /* ignore continuation unless we've seen the start */
                if (a2dp->signal_conn.reassembled_packet == NULL)
                    break;

                /* check the transaction label is the same as the start */
                if ((a2dp->signal_conn.reassembled_packet[0] & 0xf0) != (ptr[0] & 0xf0))
                    break;

                /* grow the reassembled packet to add the new fragment */
                a2dp->signal_conn.reassembled_packet = (uint8*)realloc(a2dp->signal_conn.reassembled_packet, a2dp->signal_conn.reassembled_packet_length + packet_size - 1);
                if (a2dp->signal_conn.reassembled_packet == NULL)
                    break;

                /* copy the continuation fragment (less the header) */
                memmove(a2dp->signal_conn.reassembled_packet+a2dp->signal_conn.reassembled_packet_length,ptr+1,packet_size-1);
                a2dp->signal_conn.reassembled_packet_length+=packet_size-1;

                if (type == avdtp_packet_type_end)
                {
                    /* free the message */
                    free(a2dp->signal_conn.reassembled_packet);
                    a2dp->signal_conn.reassembled_packet = NULL;
                }
            }
            break;

			default:
				break;
        }

		/* discard the fragment */
		SourceDrop(source, packet_size);
	}
}


/****************************************************************************/
void a2dpHandleSignalPacket(const A2DP *a2dp, Source source)
{
    if (!source || !SourceIsValid(source))
		return;

	/* Check if this is for the signalling channel */
	if(StreamSinkFromSource(source) == a2dp->signal_conn.sink)
		MessageSend((Task)&a2dp->task, A2DP_INTERNAL_SIGNAL_PACKET_IND, 0);
}


/****************************************************************************/
void a2dpHandleNewSignalPacket(A2DP *a2dp)
{
	Source source = StreamSourceFromSink(a2dp->signal_conn.sink);

    if (!source || !SourceIsValid(source))
		return;

	/* If we're in the right state and there's data in the buffer, process it. */
	if ((a2dp->signal_conn.connection_state == avdtp_connection_connected) && SourceBoundary(source))
		processSignalFragment(a2dp);
}



