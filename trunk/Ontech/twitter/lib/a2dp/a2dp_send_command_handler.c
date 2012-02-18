/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_send_command_handler.c        

DESCRIPTION

NOTES

*/



/****************************************************************************
	Header files
*/

#include "a2dp_send_command_handler.h"
#include "a2dp_send_packet_handler.h"

#include <print.h>
#include <stdlib.h>
#include <string.h>
#include <sink.h>

/*****************************************************************************/
static bool a2dpSendCommand(A2DP* a2dp, uint16 sink_size, uint8 command, uint16 seid0, uint16 seid1, const uint8* caps, uint16 caps_size)
{
	uint8* ptr;
	uint16 data_offset = 3;
	Sink sink = a2dp->signal_conn.sink;

	PRINT(("a2dpSendSetConfiguration\n"));

	if (!sink || !SinkIsValid(sink))
		/* can't get the sink */
		return FALSE;

	if (a2dp->signal_conn.pending_transaction_label && (command != avdtp_close) && (command != avdtp_abort))
		/* command is already pending. */
		return FALSE;

	if ((ptr = a2dpGrabSink(sink, sink_size)) == NULL)
		return FALSE;

	a2dp->signal_conn.transaction_label = (a2dp->signal_conn.transaction_label + 1) & 0xf;
	
	if(a2dp->signal_conn.transaction_label == 0)
		a2dp->signal_conn.transaction_label = 1;
	
	a2dp->signal_conn.pending_transaction_label = a2dp->signal_conn.transaction_label;
	
	/* get a transaction ID and store it to match with the response. */
	ptr[0] = (a2dp->signal_conn.transaction_label << 4) | avdtp_message_type_command;
	ptr[1] = (uint8) command;

	if (seid0 != 0xFFFF)
		ptr[2] = seid0 << 2;

	if (seid1 != 0xFFFF)
	{
		ptr[3] = seid1 << 2;
		data_offset++;
	}

	if (caps_size)
		memmove(&ptr[data_offset], caps, caps_size);

    return a2dpSendPacket(sink, a2dp->signal_conn.mtu, sink_size);
}


/****************************************************************************/
bool a2dpSendDiscover(A2DP* a2dp)
{
	PRINT(("a2dpSendDiscover\n"));

	return a2dpSendCommand(a2dp, 2, avdtp_discover, 0xFFFF, 0xFFFF, 0, 0);
}


/*****************************************************************************/
bool a2dpSendGetCapabilities(A2DP* a2dp, uint8 seid)
{
	PRINT(("a2dpSendGetCapabilities\n"));
   
	return a2dpSendCommand(a2dp, 3, avdtp_get_capabilities, seid, 0xFFFF, 0, 0);
}


/*****************************************************************************/
bool a2dpSendSetConfiguration(A2DP* a2dp, const uint8* config, uint16 config_size)
{
	PRINT(("a2dpSendSetConfiguration\n"));

	/* Write the codec configuration to the end of our configuration */
	memmove(a2dp->sep.configured_service_caps+a2dp->sep.configured_service_caps_size-config_size, config, config_size);

	return a2dpSendCommand(a2dp, a2dp->sep.configured_service_caps_size+4, avdtp_set_configuration, a2dp->sep.remote_seid, a2dp->sep.current_sep->sep_config->seid, a2dp->sep.configured_service_caps, a2dp->sep.configured_service_caps_size);
}


/*****************************************************************************/
bool a2dpSendOpen(A2DP* a2dp)
{
	PRINT(("a2dpSendOpen\n"));

	return a2dpSendCommand(a2dp, 3, avdtp_open, a2dp->sep.remote_seid, 0xFFFF, 0, 0);
}


/****************************************************************************/
bool a2dpSendReconfigure(A2DP* a2dp, const uint8* config, uint16 config_size)
{
	return a2dpSendCommand(a2dp, a2dp->sep.reconfigure_caps_size+3, avdtp_reconfigure, a2dp->sep.remote_seid, 0xFFFF, a2dp->sep.reconfigure_caps, a2dp->sep.reconfigure_caps_size);
}


/****************************************************************************/
bool a2dpSendStart(A2DP* a2dp)
{
	PRINT(("a2dpSendStart\n"));

	return a2dpSendCommand(a2dp, 3, avdtp_start, a2dp->sep.remote_seid, 0xFFFF, 0, 0);
}


/****************************************************************************/
bool a2dpSendSuspend(A2DP* a2dp)
{
	PRINT(("a2dpSendSuspend\n"));

	return a2dpSendCommand(a2dp, 3, avdtp_suspend, a2dp->sep.remote_seid, 0xFFFF, 0, 0);
}


/****************************************************************************/
bool a2dpSendClose(A2DP* a2dp)
{
	PRINT(("a2dpSendClose\n"));

	return a2dpSendCommand(a2dp, 3, avdtp_close, a2dp->sep.remote_seid, 0xFFFF, 0, 0);
}


/****************************************************************************/
bool a2dpSendAbort(A2DP* a2dp)
{
	PRINT(("a2dpSendAbort\n"));

	return a2dpSendCommand(a2dp, 3, avdtp_abort, a2dp->sep.remote_seid, 0xFFFF, 0, 0);
}


