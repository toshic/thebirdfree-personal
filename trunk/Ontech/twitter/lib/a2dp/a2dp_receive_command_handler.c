/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_receive_command_handler.c        

DESCRIPTION

NOTES

*/



/****************************************************************************
	Header files
*/

#include "a2dp_caps_parse.h"
#include "a2dp_codec_handler.h"
#include "a2dp_free.h"
#include "a2dp_get_sep.h"
#include "a2dp_receive_command_handler.h"
#include "a2dp_reconfigure_handler.h"
#include "a2dp_send_packet_handler.h"
#include "a2dp_signalling_handler.h"
#include "a2dp_suspend_handler.h"

#include <print.h>
#include <stdlib.h>
#include <string.h>
#include <sink.h>

static bool isSepResourceInUse(A2DP* a2dp, uint8 resource_id)
{
	uint16 i;
	sep_type *pSeps = a2dp->sep.sep_list->seps;

	for (i=0; i<a2dp->sep.sep_list->size_seps; i++)
	{
		if (pSeps->sep_config->resource_id == resource_id)
		{
			if (pSeps->configured)
				return 1;
		}

		pSeps++;
	}

	return 0;
}


static uint8 getLocalSeid(A2DP* a2dp)
{
	if (a2dp->sep.current_sep)
		return a2dp->sep.current_sep->sep_config->seid;
	else
		return 0;
}


/****************************************************************************
NAME	
	validateCommandPDU

DESCRIPTION
	Does a quick check to see if the length of a command packet is reasonable
	for its type.  Typically this is just the mandatory fields, so any optional
	fields should be checked during processing.

RETURNS
	bool - TRUE if packet is reasonable
*/
static bool validateCommandPDU(const uint8 *ptr, uint16 packet_size)
{
	uint16 min_length;

	/* check there is at least a command */
	if (packet_size < 2)
		return FALSE;

	/* check for packets that require more data */
	switch (ptr[1])
	{
		/* no payload */
		case avdtp_null:
		case avdtp_discover:
		default:
			min_length= 2;
			break;

		case avdtp_get_configuration:
		case avdtp_get_capabilities:
		case avdtp_open:
		case avdtp_start:
		case avdtp_close:
		case avdtp_suspend:
		case avdtp_abort:
			min_length = 3;
			break;

		case avdtp_reconfigure:
		case avdtp_set_configuration:
			min_length = 5;
			break;
	}

	if (packet_size < min_length)
		return FALSE;

	return TRUE;
}


/****************************************************************************
NAME	
	processGeneralRejectResponse

DESCRIPTION
	The remote device has rejected our command.
		
RETURNS
	void
*/
static void processGeneralRejectResponse(A2DP* a2dp, const uint8* ptr)
{
	if (a2dp->signal_conn.pending_transaction_label != (ptr[0] >> 4))
		return;

	/* handle reject for recoverable states such as
	   optional features. */
	if (a2dp->signal_conn.signalling_state == avdtp_state_local_suspending)
	{
		sendSuspendCfm(a2dp, a2dp->media_conn.sink, a2dp_rejected_by_remote_device);

		/* return to open */
		a2dpSetSignallingState(a2dp, avdtp_state_open);
	}
	else if (a2dp->signal_conn.signalling_state == avdtp_state_reconfiguring)
	{
		a2dpSendReconfigureCfm(a2dp, a2dp_rejected_by_remote_device);

		/* return to open */
		a2dpSetSignallingState(a2dp, avdtp_state_open);
	}
	else
	{
		/* can't handle rejection - abort */
		a2dpAbortSignalling(a2dp, FALSE, FALSE);
	}
}


/****************************************************************************
NAME	
	processDiscover

DESCRIPTION
	Process an incoming discover request. Build response - assume it will 
	always be less than 48 bytes and hence fit in a single L2CAP packet.
	
RETURNS
	void
*/
static void processDiscover(A2DP *a2dp, const uint8 *ptr)
{
	uint8 pdu_size = 2;
	Sink sink = a2dp->signal_conn.sink;
    uint8 *resp = NULL;
	sep_type *pSeps = a2dp->sep.sep_list->seps;
	uint16 i;

	resp = a2dpGrabSink(sink, 2);

    if (!sink || !SinkIsValid(sink) || (resp == NULL))
		return;

	/* Build header with transaction label from request. */
	resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_accept;
	resp[1] = (uint8) avdtp_discover;

	for (i=0; i<a2dp->sep.sep_list->size_seps; i++)
	{
        if ((resp = a2dpGrabSink(sink, 2)) == NULL)
			break;
		resp[0] = pSeps->sep_config->seid << 2;

		if (isSepResourceInUse(a2dp, pSeps->sep_config->resource_id) || pSeps->in_use)	
			resp[0] |= 2;

		resp[1] = (uint8) ((pSeps->sep_config->media_type << 4) | ((pSeps->sep_config->role) << 3));
		pdu_size += 2;

		pSeps++;
	}

    (void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, pdu_size));
}


/****************************************************************************
NAME	
	processGetCaps

DESCRIPTION
	Process an incoming Get_Capabilities request
	
RETURNS
	void
*/
static void processGetCaps(A2DP* a2dp, const uint8 *ptr, sep_type *pSeps)
{
	/* Build response - assume it will always be less than
	   48 bytes and hence fit in a single L2CAP packet. */
	uint8 pdu_size = 0;
	Sink sink = a2dp->signal_conn.sink;
    uint8 *resp = NULL;

    if (!sink || !SinkIsValid(sink))
		return;

	if (pSeps == NULL)
	{
		/* Invalid SEP - reject request */
        if ((resp = a2dpGrabSink(sink, 3)) == NULL)
			return;
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
		resp[1] = (uint8) avdtp_get_capabilities;
		resp[2] = (uint8) avdtp_bad_acp_seid;
		pdu_size = 3;
	}
	else
	{
		/* build header with transaction label from request. */
		if ((resp = a2dpGrabSink(sink, 2+pSeps->sep_config->size_caps)) == NULL)
			return;
		pdu_size = 2 + pSeps->sep_config->size_caps;
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_accept;
		resp[1] = (uint8) avdtp_get_capabilities;

		/* copy in the caps supplied by the application */
		memmove(&resp[2], pSeps->sep_config->caps, pSeps->sep_config->size_caps);
	}

    (void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, pdu_size));
}


/****************************************************************************
NAME	
	processSetConfig

DESCRIPTION
	Process an incoming Set_Configuration request
	
RETURNS
	void
*/
static void processSetConfig(A2DP* a2dp, const uint8 *ptr, uint16 packet_size, sep_type *pSeps)
{
	/* Build response - assume it will always be less than
	   48 bytes and hence fit in a single L2CAP packet. */
	uint16 pdu_size = 4;
	Sink sink = a2dp->signal_conn.sink;
    uint8 unsupported_service;
	uint8 error_cat, error_code;
    uint8 *resp = NULL;
	uint8 tmpResp[2];

    if (!sink || !SinkIsValid(sink))
		return;

	if (pSeps == NULL)
	{
		/* Invalid SEP - reject request */       
		tmpResp[0] = 0;
		tmpResp[1] = (uint8) avdtp_bad_acp_seid;
	}
	else if (isSepResourceInUse(a2dp, pSeps->sep_config->resource_id) || pSeps->in_use)
	{
		/* SEP is already in use - reject */       
		tmpResp[0] = 0; /* Service Capabilities were not the problem */
		tmpResp[1] = (uint8) avdtp_sep_in_use;
	}
	else if (!a2dpValidateServiceCaps(&ptr[4], packet_size-4, FALSE, FALSE, &error_cat, &error_code))
	{
		/* bad caps - reject */       
		tmpResp[0] = error_cat;
		tmpResp[1] = error_code;
	}
    else if (!a2dpAreServicesCategoriesCompatible(pSeps->sep_config->caps, pSeps->sep_config->size_caps, &ptr[4], packet_size-4, &unsupported_service))
	{
		/* 
			Check that configuration only asks for services the local SEP supports 
			set config does not match our caps - reject 
		*/        
		tmpResp[0] = unsupported_service;
		tmpResp[1] = (uint8) avdtp_unsupported_configuration;
    }
	else if (a2dpFindMatchingCodecSpecificInformation(pSeps->sep_config->caps, &ptr[4], 0) == NULL)
	{
		/* 
			Check the codec specific attributes are compatible 
			set config does not match our caps - reject 
		*/    
		tmpResp[0] = AVDTP_SERVICE_MEDIA_CODEC;
		tmpResp[1] = (uint8) avdtp_unsupported_configuration;
	}
	else
	{     
		pdu_size = 2;
		
		/* SEP is now configured */
		a2dpSetSignallingState(a2dp, avdtp_state_configured);

		/* Store a pointer to the current SEID data */
		a2dp->sep.current_sep = pSeps;

		/* Mark this SEP as in use */
		a2dp->sep.current_sep->configured = 1;

		/* Store remote SEID */
		a2dp->sep.remote_seid = (ptr[3] >> 2) & 0x3f;

		/* Store configuration */
		a2dp->sep.configured_service_caps_size = packet_size - 4;

		a2dp->sep.configured_service_caps = (uint8*)malloc(a2dp->sep.configured_service_caps_size);
		memmove(a2dp->sep.configured_service_caps, &ptr[4], a2dp->sep.configured_service_caps_size);
	}

	if ((resp = a2dpGrabSink(sink, pdu_size)) == NULL)
		return;

	if (pdu_size > 2)
	{
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
		resp[2] = tmpResp[0];
		resp[3] = tmpResp[1];
	}
	else
	{
		resp[0] = (ptr[0] & 0xf0) |  avdtp_message_type_accept;
	}
	resp[1] = (uint8) avdtp_set_configuration;

    (void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, pdu_size));
}


/****************************************************************************
NAME	
	processGetConfig

DESCRIPTION
	Process an incoming Get_Capabilities request.
	
RETURNS
	void
*/
static void processGetConfig(A2DP* a2dp, const uint8* ptr, sep_type *pSeps)
{
	/* Build response - assume it will always be less than
	   48 bytes and hence fit in a single L2CAP packet. */
	Sink sink = a2dp->signal_conn.sink;
	uint8 pdu_size;
    uint8 *resp = NULL;

    if (!sink || !SinkIsValid(sink))
		return;

	if (pSeps == NULL)
	{
        if ((resp = a2dpGrabSink(sink, 3)) == NULL)
			return;
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
		resp[1] = (uint8) avdtp_get_configuration;
		resp[2] = (uint8) avdtp_bad_acp_seid;
		pdu_size = 3;
	}
	else if ( ((a2dp->signal_conn.signalling_state != avdtp_state_configured) &&
			  (a2dp->signal_conn.signalling_state != avdtp_state_local_opening) &&
			  (a2dp->signal_conn.signalling_state != avdtp_state_open) &&
			  (a2dp->signal_conn.signalling_state != avdtp_state_local_starting) &&
			  (a2dp->signal_conn.signalling_state != avdtp_state_local_suspending) &&
				(a2dp->signal_conn.signalling_state != avdtp_state_reconfig_reading_caps) &&
				(a2dp->signal_conn.signalling_state != avdtp_state_reconfiguring) &&
			  (a2dp->signal_conn.signalling_state != avdtp_state_streaming)) ||
			  (getLocalSeid(a2dp) != ((ptr[2]>>2) & 0x3f)))
	{
		/* This command is not valid in this state. */
        if ((resp = a2dpGrabSink(sink, 3)) == NULL)
			return;
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
		resp[1] = (uint8) avdtp_get_configuration;
		resp[2] = (uint8) avdtp_bad_state;
		pdu_size = 3;
	}
	else
	{
		/* build header with transaction label from request. */
		if ((resp = a2dpGrabSink(sink, 2 + a2dp->sep.configured_service_caps_size)) == NULL)
			return;
		pdu_size = 2 + a2dp->sep.configured_service_caps_size;
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_accept;
		resp[1] = (uint8) avdtp_get_configuration;

		/* copy in the agreed configuration for this SEP.
		   Note that this only returns the last SET_CONFIG or RECONFIGURE
		   parameters and not the global configuration - is this ok? */
		memmove(&resp[2],a2dp->sep.configured_service_caps, a2dp->sep.configured_service_caps_size);
	}

    (void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, pdu_size));
}


/****************************************************************************
NAME	
	processReconfigure

DESCRIPTION
	Process an incoming reconfigure request. Build response - assume it will 
	always be less than 48 bytes and hence fit in a single L2CAP packet.
	
RETURNS
	void
*/
static void processReconfigure(A2DP *a2dp, const uint8 *ptr, uint16 packet_size, sep_type *pSeps)
{
	uint8 error_cat, error_code;
	uint16 pdu_size = 4;
	uint8 *resp = NULL;
	uint8 tmpResp[2];
	Sink sink = a2dp->signal_conn.sink;
	uint8 unsupported_service;

	/*
		AVDTP test TP/SIG/SMG/BI-14-C is very pedantic about the order in which
		we should return errors, even though the spec does not state a validation
		procedure.  This code is therefore more verbose than it needs to be
		in order to generate the correct errors in the correct order.
	*/
	if (!a2dpValidateServiceCaps(&ptr[3], packet_size-3, FALSE, TRUE, &error_cat, &error_code))
	{
		/* 
			Check that caps from remote device parse and look reasonable 
			bad caps - reject 
		*/
		tmpResp[0] = error_cat;
		tmpResp[1] = error_code;
	}
	else if (getLocalSeid(a2dp) != ((ptr[2]>>2) & 0x3f))
	{
		/* SEP is already in use - reject */
		tmpResp[0] = 0;
		tmpResp[1] = (uint8) avdtp_sep_not_in_use;
	}
	/* check that the service caps are valid for reconfigure */
	else if (!a2dpValidateServiceCaps(&ptr[3], packet_size-3, TRUE, FALSE, &error_cat, &error_code))
	{
		/* 
			Check that caps from remote device parse and look reasonable 
			bad caps - reject 
		*/
		tmpResp[0] = error_cat;
		tmpResp[1] = error_code;
	}
	else if (a2dp->signal_conn.signalling_state != avdtp_state_open)
	{
		/* SEP is already in use - reject */
		tmpResp[0] = 0;
		tmpResp[1] = (uint8) avdtp_sep_not_in_use;
	}
    else
	if (!a2dpAreServicesCategoriesCompatible(pSeps->sep_config->caps, pSeps->sep_config->size_caps, &ptr[3], packet_size-3, &unsupported_service))
    {
		/* 
			Check that configuration only asks for services the local SEP supports
			set config does not match our caps - reject 
		*/	
		tmpResp[0] = unsupported_service;
		tmpResp[1] = (uint8) avdtp_unsupported_configuration;
    }
	else if (a2dpFindMatchingCodecSpecificInformation(pSeps->sep_config->caps, &ptr[3], 0) == NULL)
	{
		/* 
			Check the codec specific attributes are compatible
			requested codec is not compatible with our caps 
		*/		
		tmpResp[0] = AVDTP_SERVICE_MEDIA_CODEC;
		tmpResp[1] = (uint8) avdtp_unsupported_configuration;
	}
	else
	{		
		pdu_size = 2;

		/* Free any old configuration */
		a2dpFreeConfiguredCapsMemory(a2dp);

		/* Store configuration */
		a2dp->sep.configured_service_caps_size = packet_size - 3;

		a2dp->sep.configured_service_caps = (uint8*)malloc(a2dp->sep.configured_service_caps_size);
		memmove(a2dp->sep.configured_service_caps, &ptr[3], a2dp->sep.configured_service_caps_size);

		{
			/* Choose the codec params that the app needs to know about and send it this information */
			MAKE_A2DP_MESSAGE(A2DP_INTERNAL_SEND_CODEC_PARAMS_REQ);
			message->send_reconfigure_message = 0;
			MessageSend(&a2dp->task, A2DP_INTERNAL_SEND_CODEC_PARAMS_REQ, message);
		}
	}

	if ((resp = a2dpGrabSink(sink, pdu_size)) == NULL)
		return;

	if (pdu_size > 2)
	{
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
		resp[2] = tmpResp[0];
		resp[3] = tmpResp[1];
	}
	else
	{
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_accept;
	}
	resp[1] = (uint8) avdtp_reconfigure;

    (void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, pdu_size));
}


/****************************************************************************
NAME	
	processOpen

DESCRIPTION
	Process an incoming Open request.
	
RETURNS
	void
*/
static void processOpen(A2DP* a2dp, const uint8* ptr, sep_type *pSeps)
{
	/* Build response - assume it will always be less than
	   48 bytes and hence fit in a single L2CAP packet. */
	Sink sink = a2dp->signal_conn.sink;
	uint16 pdu_size = 3;
    uint8 *resp = NULL;
	uint8 tmpResp[1];

    if (!sink || !SinkIsValid(sink))
        return;

	if (pSeps == NULL)
	{
		/* Invalid SEP - reject request */
		tmpResp[0] = (uint8) avdtp_bad_acp_seid;
	}
	else if ((a2dp->signal_conn.signalling_state != avdtp_state_configured) || (getLocalSeid(a2dp) != ((ptr[2]>>2) & 0x3f)))
	{
		/* bad state - reject */
		tmpResp[0] = (uint8) avdtp_bad_state;
	}
	else
	{
		pdu_size = 2;

		/* SEP is configured, so can be opened - accept */

		/* Move to the opening state. We only move to Open once the INT has opened all streaming channels. */
		a2dpSetSignallingState(a2dp, avdtp_state_remote_opening);
	}

	if ((resp = a2dpGrabSink(sink, pdu_size)) == NULL)
		return;

	if (pdu_size > 2)
	{
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
		resp[2] = tmpResp[0];
	}
	else
	{
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_accept;
	}
	resp[1] = (uint8) avdtp_open;	

    (void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, pdu_size));

}


/****************************************************************************
NAME	
	processStart

DESCRIPTION
	Process a Start request.
	
RETURNS
	void
*/
static bool processStart(A2DP* a2dp, const uint8 *ptr, uint16 packet_size, sep_type *pSeps)
{
	/* Build response - assume it will always be less than
	   48 bytes and hence fit in a single L2CAP packet. */
	Sink sink = a2dp->signal_conn.sink;

	/* loop through all SEIDs */
	uint16 seids = packet_size - 2;
    uint8 *resp = NULL;

    if (!sink || !SinkIsValid(sink))
		return FALSE;

	for (;seids != 0; seids--)
	{
		uint8 seid = (ptr[packet_size-seids] >> 2) & 0x3f;

		if (pSeps == NULL)
		{
            if ((resp = a2dpGrabSink(sink, 4)) == NULL)
				return FALSE;
			resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
			resp[1] = (uint8) avdtp_start;
			resp[2] = seid << 2;
			resp[3] = (uint8) avdtp_bad_acp_seid;

			(void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, 4));			
			return TRUE;
		}

		/*
			There is a race condition due to Streams which
			causes us to see data before L2CAP signalling.
			This means that we may see a START request before
			being informed of the media channel(s) opening.
			To avoid this, we ignore the command now and
			come back later.
		*/
		if ((a2dp->signal_conn.signalling_state == avdtp_state_remote_opening) || (a2dp->signal_conn.signalling_state == avdtp_state_local_opening))
		{
			return FALSE;
		}
		if (((a2dp->signal_conn.signalling_state != avdtp_state_open) &&
		     (a2dp->signal_conn.signalling_state != avdtp_state_local_starting)) || (getLocalSeid(a2dp) != seid))
		{
			/* SEP is not in open state */
            if ((resp = a2dpGrabSink(sink, 4)) == NULL)
				return FALSE;
			resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
			resp[1] = (uint8) avdtp_start;
			resp[2] = seid << 2;
			resp[3] = (uint8) avdtp_bad_state;
			/* spec states we only report the first error */
			(void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, 4));
			return TRUE;
		}

		/* We are now streaming - tell application. */
		a2dpSetSignallingState(a2dp, avdtp_state_streaming);
		{
			MAKE_A2DP_MESSAGE(A2DP_START_IND);
			message->media_sink = a2dp->media_conn.sink;
			message->a2dp = a2dp;
			MessageSend(a2dp->clientTask, A2DP_START_IND, message);
		}
	}

	/* All SEIDs started, accept */
	{
        if ((resp = a2dpGrabSink(sink, 2)) == NULL)
			return FALSE;
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_accept;
		resp[1] = (uint8) avdtp_start;
		(void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, 2));
	}
	return TRUE;
}


/****************************************************************************
NAME	
	processClose

DESCRIPTION
	Process a Close request.
	
RETURNS
	void
*/
static void processClose(A2DP* a2dp, const uint8* ptr, sep_type *pSeps)
{
	/* Build response - assume it will always be less than
	   48 bytes and hence fit in a single L2CAP packet. */
	Sink sink = a2dp->signal_conn.sink;
	
	uint16 pdu_size;
    uint8 *resp = NULL;

    if (!sink || !SinkIsValid(sink))
		return;

	if (pSeps == NULL)
	{
		/* Invalid SEP - reject request */
        if ((resp = a2dpGrabSink(sink, 3)) == NULL)
			return;
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
		resp[1] = (uint8) avdtp_close;
		resp[2] = (uint8) avdtp_bad_acp_seid;
		pdu_size = 3;
	}
	else if (((a2dp->signal_conn.signalling_state != avdtp_state_open) && 
		(a2dp->signal_conn.signalling_state != avdtp_state_local_starting) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_local_suspending) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_reconfig_reading_caps) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_reconfiguring) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_streaming)) 
		|| (getLocalSeid(a2dp) != ((ptr[2]>>2) & 0x3f)))
	{
		/* SEP is not in an appropriate state */
        if ((resp = a2dpGrabSink(sink, 3)) == NULL)
			return;
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
		resp[1] = (uint8) avdtp_close;
		resp[2] = (uint8) avdtp_bad_state;
		pdu_size = 3;
	}
	else
	{
		/* accept */
        if ((resp = a2dpGrabSink(sink, 2)) == NULL)
			return;
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_accept;
		resp[1] = (uint8) avdtp_close;
		pdu_size = 2;

		/* we are now closing. */
		a2dpSetSignallingState(a2dp, avdtp_state_remote_closing);

		/* Don't tell the application until the L2CAP channels have gone.
		   If the remote device doesn't close them, the watchdog will fire. */
	}
    (void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, pdu_size));
}


/****************************************************************************
NAME	
	processSuspend

DESCRIPTION
	Process a suspend request.
	
RETURNS
	void
*/
static void processSuspend(A2DP* a2dp, const uint8* ptr, uint16 packet_size, sep_type *pSeps)
{
	/* Build response - assume it will always be less than
	   48 bytes and hence fit in a single L2CAP packet. */
	Sink sink = a2dp->signal_conn.sink;

	/* loop through all SEIDs */
	uint16 seids = packet_size - 2;
    uint8 *resp = NULL;

    if (!sink || !SinkIsValid(sink))
		return;

	for (;seids!=0;seids--)
	{
		uint8 seid = (ptr[packet_size-seids] >> 2) & 0x3f;
	
		if (pSeps == NULL)
		{
			/* Invalid SEP - reject request */
            if ((resp = a2dpGrabSink(sink, 4)) == NULL)
				return;
			resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
			resp[1] = (uint8) avdtp_suspend;
			resp[2] = seid << 2;
			resp[3] = (uint8) avdtp_bad_acp_seid;
			
			/* spec states we only report the first error */
			(void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, 4));
			return;
		}
		if (((a2dp->signal_conn.signalling_state != avdtp_state_streaming) &&
		     (a2dp->signal_conn.signalling_state != avdtp_state_local_suspending)) || (getLocalSeid(a2dp) != seid))
		{
			/* SEP is not in streaming state */
            if ((resp = a2dpGrabSink(sink, 4)) == NULL)
				return;
			resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
			resp[1] = (uint8) avdtp_suspend;
			resp[2] = seid << 2;
			resp[3] = (uint8) avdtp_bad_state;
			
			/* spec states we only report the first error */
			(void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, 4));
			return;
		}

		/* we drop back to open - tell application. */
		a2dpSetSignallingState(a2dp, avdtp_state_open);
		{
			MAKE_A2DP_MESSAGE(A2DP_SUSPEND_IND);
			message->media_sink = a2dp->media_conn.sink;
			message->a2dp = a2dp;
			MessageSend(a2dp->clientTask, A2DP_SUSPEND_IND, message);
		}
	}

	/* all SEIDs started, accept */
	{
        if ((resp = a2dpGrabSink(sink, 2)) == NULL)
			return;
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_accept;
		resp[1] = (uint8) avdtp_suspend;

		(void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, 2));
	}
}


/****************************************************************************
NAME	
	processAbort

DESCRIPTION
	Process an Abort request.
	
RETURNS
	void
*/
static void processAbort(A2DP* a2dp, const uint8* ptr)
{
	/* Build response - assume it will always be less than
	   48 bytes and hence fit in a single L2CAP packet. */
	Sink sink = a2dp->signal_conn.sink;
	uint8 *resp = a2dpGrabSink(sink, 2);
		
    if (!sink || !SinkIsValid(sink) || (resp == NULL))
		return;

	/* We always have to acknowledge ABORT, although it should really be a valid SEID */
	resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_accept;
	resp[1] = (uint8) avdtp_abort;
    (void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, 2));

	if (a2dp->sep.current_sep)
	{
		if (((ptr[2]>>2) & 0x3f) == a2dp->sep.current_sep->sep_config->seid)
		{
			/* move to the aborting state and wait for the remote
			   device to close down the link */
			a2dpSetSignallingState(a2dp, avdtp_state_remote_aborting);
			
			/* must close signalling channel if an ABORT is received */
			a2dpResetSignalling(a2dp, FALSE, FALSE);
		}
	}
}



/*****************************************************************************/
bool handleReceiveCommand(A2DP* a2dp, const uint8 *ptr, uint16 packet_size)
{
    Sink sink = a2dp->signal_conn.sink;
    uint8 *resp = NULL;
	sep_type *pSeps = NULL;;

    if (!sink || !SinkIsValid(sink))
        return FALSE;

	/* check command is a reasonable length.
	   We do this check here to prevent the other
	   functions having to do it.
	*/
	if (!validateCommandPDU(ptr, packet_size))
	{
        if ((resp = a2dpGrabSink(sink, 3)) == NULL)
			return FALSE;
		resp[0] = (ptr[0] & 0xf0) | avdtp_message_type_reject;
		resp[1] = (packet_size > 1?ptr[1]:0);
		resp[2] = (uint8) avdtp_bad_length;
		(void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, 3));

		return TRUE;
	}
	
	if (packet_size >= 3)
		pSeps = getSepInstanceBySeid(a2dp, (ptr[2]>>2) & 0x3f);

	/* Call handler function to process command */
	switch (ptr[1])
	{
		case avdtp_null:
			/* strangely, the response to an unsupported
			   command is General Reject which has a
			   null message-type and so looks like a command.
			   We therefore need to process as a response!
			*/
			processGeneralRejectResponse(a2dp, ptr);
			break;

		case avdtp_discover:
			processDiscover(a2dp, ptr);
			break;

		case avdtp_get_capabilities:
			processGetCaps(a2dp, ptr, pSeps);
			break;

		case avdtp_set_configuration:
			processSetConfig(a2dp, ptr, packet_size, pSeps);
			break;

		case avdtp_get_configuration:
			processGetConfig(a2dp, ptr, pSeps);
			break;

		case avdtp_reconfigure:
			processReconfigure(a2dp, ptr, packet_size, pSeps);
			break;

		case avdtp_open:
			processOpen(a2dp, ptr, pSeps);
			break;

		case avdtp_start:
			return processStart(a2dp, ptr, packet_size, pSeps);

		case avdtp_close:
			processClose(a2dp, ptr, pSeps);
			break;

		case avdtp_suspend:
			processSuspend(a2dp, ptr, packet_size, pSeps);
			break;

		case avdtp_abort:
			processAbort(a2dp, ptr);
			break;

		default:
		{
			/* unknown send General reject */
            if ((resp = a2dpGrabSink(sink, 2)) == NULL)
				return FALSE;
			resp[0] = (ptr[0] & 0xf0);
			resp[1] = (uint8) avdtp_null;
			(void) PanicFalse(a2dpSendPacket(sink, a2dp->signal_conn.mtu, 2));
		}
		break;
	}

	return TRUE;
}

