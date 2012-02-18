/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_receive_response_handler.c        

DESCRIPTION

NOTES

*/



/****************************************************************************
	Header files
*/

#include "a2dp_caps_parse.h"
#include "a2dp_codec_handler.h"
#include "a2dp_connect_handler.h"
#include "a2dp_current_sep_handler.h"
#include "a2dp_free.h"
#include "a2dp_get_sep.h"
#include "a2dp_receive_response_handler.h"
#include "a2dp_reconfigure_handler.h"
#include "a2dp_send_command_handler.h"
#include "a2dp_signalling_handler.h"
#include "a2dp_start_handler.h"
#include "a2dp_suspend_handler.h"

#include <print.h>
#include <stdlib.h>
#include <string.h>


/****************************************************************************
NAME	
	validateResponsePDU

DESCRIPTION
	This function is called to validate a response PDU.
	
RETURNS
	void
*/
static bool validateResponsePDU(uint16 packet_size)
{
	if (packet_size < 2)
		/* must contain at least a header and signal ID */
		return FALSE;

	return TRUE;
}


/****************************************************************************
NAME	
	startBuildingConfiguration

DESCRIPTION
	Starts to build the SET_CONFIGURATION request based on the local and remote caps
	
RETURNS
	void
*/
static bool startBuildingConfiguration(A2DP *a2dp, const uint8 *remote_caps, uint16 size_caps)
{
	const uint8 *remote_codec = NULL;
	uint8 *config = NULL;
	uint8 *temp = NULL;
	uint16 config_write_offset = 0;
	uint16 config_size = 0;
	const uint8 *ptr = NULL;
	uint8 *returned_caps = NULL;

	/* start to build configuration by allocating space for at least a header */
	config = (uint8 *) malloc(2);
	if (!config)
		return FALSE;
	
	config_size = 2;
	/* fill in media transport as it is always present */
	config[config_write_offset++] = AVDTP_SERVICE_MEDIA_TRANSPORT;
	config[config_write_offset++] = 0; /* LOSC is always zero */
	
	/* Insert Content Protection */
	ptr = remote_caps;
	if ((a2dpGetContentProtection(ptr, size_caps, returned_caps) == avdtp_scms_protection) && (a2dp->sep.current_sep->sep_config->role == a2dp_sink))
	{
		
		temp = realloc(config, config_size + 4);
		if (!temp)
		{
			free(config);
			return FALSE;
		}
		config = temp;
		
		config[config_write_offset++] = AVDTP_SERVICE_CONTENT_PROTECTION;
		config[config_write_offset++] = 2; /* LOSC is 2 */
		config[config_write_offset++] = AVDTP_CP_TYPE_SCMS_LSB;
		config[config_write_offset++] = AVDTP_CP_TYPE_SCMS_MSB;
		config_size += 4;
	}

	/*
		Codec Configuration.
	*/
	remote_codec = a2dpFindMatchingCodecSpecificInformation(a2dp->sep.current_sep->sep_config->caps, remote_caps, 1);
	if (!remote_codec)
	{
		free(config);
		/* couldn't find matching codec */
		return FALSE;
	}

	/* Grow the configuration list and store it, so that codec can be written
	   at the end when the application sends AVDT_CONFIGURE_CODEC_RSP. */
	temp = realloc(config, config_size + 2 + remote_codec[1]);
	if (!temp)
	{
		free(config);/*lint !e449 */
		return FALSE;
	}

	/* Free old service caps - free handles NULL condition */
	free(a2dp->sep.configured_service_caps);
	a2dp->sep.configured_service_caps = temp;
	a2dp->sep.configured_service_caps_size = config_size + 2 + remote_codec[1];
	
	/* Codec match found.
	   Raise event in library to ask for the configuration it requires if this SEP states that the library should select the appropriate settings.
	   Otherwise the event needs to be sent to the application, so it can configure the remote codec settings.
	*/
	if (a2dp->sep.current_sep->sep_config->library_selects_settings)
	{
        uint16 caps_size = (2 + remote_codec[1]);
	
		MAKE_A2DP_MESSAGE_WITH_LEN(A2DP_INTERNAL_CONFIGURE_CODEC_RSP, caps_size);       

	    memmove(message->codec_service_caps, remote_codec, caps_size);        
            
		/* For SBC codec clamp the bitpools */
		if (a2dp->sep.current_sep->sep_config->caps[4] == (AVDTP_MEDIA_TYPE_AUDIO<<2))
		{
			if (a2dp->sep.current_sep->sep_config->caps[5] == AVDTP_MEDIA_CODEC_SBC)
			{
				/* If local max is less than remote max clamp to local max */
				if (message->codec_service_caps[7] > a2dp->sep.current_sep->sep_config->caps[9])
					message->codec_service_caps[7] = a2dp->sep.current_sep->sep_config->caps[9];		
			}
		}

		message->accept = a2dpHandleSelectingCodecSettings(a2dp, caps_size, message->codec_service_caps);
		message->size_codec_service_caps = caps_size;

		MessageSend(&a2dp->task, A2DP_INTERNAL_CONFIGURE_CODEC_RSP, message);
	}
	else
	{
		uint16 caps_size = (2 + remote_codec[1]);
		MAKE_A2DP_MESSAGE_WITH_LEN(A2DP_CONFIGURE_CODEC_IND, caps_size);
        message->a2dp = a2dp;
		message->size_codec_service_caps = caps_size;
	    memmove(message->codec_service_caps, remote_codec, caps_size);        
            
		/* For SBC codec clamp the bitpools */
		if (a2dp->sep.current_sep->sep_config->caps[4] == (AVDTP_MEDIA_TYPE_AUDIO<<2))
		{
			if (a2dp->sep.current_sep->sep_config->caps[5] == AVDTP_MEDIA_CODEC_SBC)
			{
				/* If local max is less than remote max clamp to local max */
				if (message->codec_service_caps[7] > a2dp->sep.current_sep->sep_config->caps[9])
					message->codec_service_caps[7] = a2dp->sep.current_sep->sep_config->caps[9];		
			}
		}

		MessageSend(a2dp->clientTask, A2DP_CONFIGURE_CODEC_IND, message);
	}

	a2dpFreeSeidListMemory(a2dp);

	return TRUE;
}


/****************************************************************************
NAME	
	startBuildingReconfigure

DESCRIPTION
	Starts to build the RECONFIGURE request based on the local and remote caps
	
RETURNS
	void
*/
static bool startBuildingReconfigure(A2DP *a2dp, const uint8 *remote_caps, uint16 size_caps)
{
	const uint8 *remote_codec = NULL;

	/* Check only media and content protection in caps? */

	size_caps = size_caps;

	/* Make sure the new reconfigure capabilities fit with the remote capabilities */
	remote_codec = a2dpFindMatchingCodecSpecificInformation(a2dp->sep.reconfigure_caps, remote_caps, 1);
	if (!remote_codec)
	{
		/* couldn't find matching codec */
		return FALSE;
	}

	return a2dpSendReconfigure(a2dp, a2dp->sep.reconfigure_caps, a2dp->sep.reconfigure_caps_size);
}


/****************************************************************************
NAME	
	storeCurrentConfiguration

DESCRIPTION
	Stores the new configuration.
	
RETURNS
	void
*/
static bool storeCurrentConfiguration(A2DP *a2dp)
{
	uint8 *returned_caps = NULL;
	uint8 *temp = NULL;
	uint16 size_service_caps = a2dp->sep.reconfigure_caps_size;
	const uint8 *service_caps = a2dp->sep.reconfigure_caps;
	uint16 config_size = 2;
	uint8 *config = (uint8 *) malloc(2);
	if (!config)
		return FALSE;
	
	/* fill in media transport as it is always present */
	config[0] = AVDTP_SERVICE_MEDIA_TRANSPORT;
	config[1] = 0; /* LOSC is always zero */

	/* Pick the required CODEC info / content protection info as required */
	
	if (a2dpFindCodecSpecificInformation(&service_caps, &size_service_caps))
	{
		temp = realloc(config, config_size + 2 + service_caps[1]);
		
		if (!temp)
		{
			free(config);
			return FALSE;
		}

		config = temp;

		/* New codec information exists in the reconfigure caps so use this info */
		memmove(&config[2],service_caps,2 + service_caps[1]);
		config_size = config_size + 2 + service_caps[1];
	}
	else
	{
		/* Use the old codec information if it exists */
		size_service_caps = a2dp->sep.configured_service_caps_size;
		service_caps = a2dp->sep.configured_service_caps;
		if (a2dpFindCodecSpecificInformation(&service_caps, &size_service_caps))
		{
			temp = realloc(config, config_size + 2 + service_caps[1]);

			if (!temp)
			{
				free(config);
				return FALSE;
			}

			config = temp;

			memmove(&config[2],service_caps,2 + service_caps[1]);
			config_size = config_size + 2 + service_caps[1];
		}
	}

	size_service_caps = a2dp->sep.reconfigure_caps_size;
	service_caps = a2dp->sep.reconfigure_caps;
	if (a2dpGetContentProtection(service_caps, size_service_caps, returned_caps) == avdtp_scms_protection)
	{
		/* New content protection information exists in the reconfigure caps so use this info */
		temp = realloc(config, config_size + 2 + returned_caps[1]);

		if (!temp)
		{
			free(config);
			return FALSE;
		}

		config = temp;

		memmove(&config[config_size], returned_caps, 2 + returned_caps[1]);
		config_size += (2 + returned_caps[1]);
	}
	else
	{
		/* Use the old content protection information if it exists */
		size_service_caps = a2dp->sep.configured_service_caps_size;
		service_caps = a2dp->sep.configured_service_caps;
		if (a2dpGetContentProtection(service_caps, size_service_caps, returned_caps) == avdtp_scms_protection)
		{
			temp = realloc(config, config_size + 2 + returned_caps[1]);

			if (!temp)
			{
				free(config);
				return FALSE;
			}

			config = temp;

			memmove(&config[config_size], returned_caps, 2 + returned_caps[1]);
			config_size += (2 + returned_caps[1]);
		}
	}

	a2dpFreeConfiguredCapsMemory(a2dp);

	a2dp->sep.configured_service_caps_size = config_size;
	a2dp->sep.configured_service_caps = (uint8*)malloc(config_size);

	if (!a2dp->sep.configured_service_caps)
	{
		free(config);
		return FALSE;
	}

	memmove(a2dp->sep.configured_service_caps, config, config_size);

	free(config);

	return TRUE;
}


/****************************************************************************
NAME	
	handleDiscoverResponse

DESCRIPTION
	Handle Discover response from remote device.
	
RETURNS
	void
*/
static void handleDiscoverResponse(A2DP* a2dp, const uint8* ptr, uint16 packet_size)
{
	uint16 i;

	/* we only expect this if we are the initiator in the discovering state */
	if (a2dp->signal_conn.signalling_state != avdtp_state_discovering)
		return;

	/*
		A valid response contains ACCEPT and at least one entry.
	*/
	if (((ptr[0] & 3) == avdtp_message_type_accept) && (packet_size > 2))
	{
		PRINT(("Discover accepted\n"));
		/*
			We must have been the initiator 
			Loop through response looking for potential matches
			and for those that match, read back the capabilities.
		*/
		/* clear record of discovered SEPs */
		if (a2dp->sep.list_discovered_remote_seids)
		{
			free(a2dp->sep.list_discovered_remote_seids);
			a2dp->sep.list_discovered_remote_seids = NULL;
		}
		a2dp->sep.max_discovered_remote_seids = 0;
	
		ptr+=2;
		
		for(i=0;i<(packet_size-2);i+=2)
		{
			uint8 media_type = ptr[1] >> 4;
			uint8 tsep = (ptr[1] >> 3) & 1;

			PRINT(("media_type=%d tsep=%d ptr[0]=%d\n",media_type,tsep,ptr[0]));
			PRINT(("test ptr[0] & 2=%d local_role^1=%d \n",ptr[0] & 2,a2dp->sep.current_sep->sep_config->role^1));

			/* check that the SEP is not in use, the media type matches and also that the SEP
			   type is the opposite. */
			if (!(ptr[0] & 2) &&
				(media_type == (uint8)a2dp->sep.current_sep->sep_config->media_type) &&
				(tsep == ((uint8)a2dp->sep.current_sep->sep_config->role^1)))
			{
				a2dp->sep.max_discovered_remote_seids++;

				if (a2dp->sep.list_discovered_remote_seids)
					a2dp->sep.list_discovered_remote_seids = (uint8 *) realloc(a2dp->sep.list_discovered_remote_seids, sizeof(uint8) * a2dp->sep.max_discovered_remote_seids);
				else
					a2dp->sep.list_discovered_remote_seids = (uint8 *) malloc(sizeof(uint8));

				a2dp->sep.list_discovered_remote_seids[a2dp->sep.max_discovered_remote_seids - 1] = ptr[0]>>2;
			}

			/* move the next result */
			ptr+=2;
		}
		
		if (a2dp->sep.max_discovered_remote_seids)	
		{
			/* read back first seid */
			a2dp->sep.remote_seid = a2dp->sep.list_discovered_remote_seids[0];
			/* start reading caps from the remote SEPs */
			a2dp->sep.current_discovered_remote_seid = 0;
			PRINT(("local_seid = %d  remote_seid = %d\n",a2dp->sep.list_preferred_local_seids[0], a2dp->sep.remote_seid));
			a2dpSetSignallingState(a2dp, avdtp_state_reading_caps);
			(void) a2dpSendGetCapabilities(a2dp, a2dp->sep.remote_seid);
			return;
		}
	}

	/* device does not have matching endpoint so stop */
	a2dpResetSignalling(a2dp, FALSE, FALSE);
}


/****************************************************************************
NAME	
	handleGetCapabilitiesResponse

DESCRIPTION
	Handle Get Capabilities response from remote device.
	
RETURNS
	void
*/
static void handleGetCapabilitiesResponse(A2DP *a2dp, const uint8* ptr, uint16 packet_size)
{
	uint8 error_cat, error_code;

	/* we only expect this if we are the initiator in the discovering state */
	if ((a2dp->signal_conn.signalling_state != avdtp_state_reading_caps) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_reconfig_reading_caps))
	{
		if (MessageCancelAll(&a2dp->task, A2DP_INTERNAL_GET_CAPS_TIMEOUT_IND))
		{
			/* Must be a get current caps request */
			if ((ptr[0] & 3) == avdtp_message_type_accept)				
				sendGetCurrentSepCapabilitiesCfm(a2dp, a2dp_success, &ptr[2], packet_size-2);
			else
				sendGetCurrentSepCapabilitiesCfm(a2dp, a2dp_rejected_by_remote_device, 0, 0);
		}	
		return;
	}

	if (((ptr[0] & 3) == avdtp_message_type_accept && (packet_size > 2)))
	{
		/* check the service capabilities are valid */
		if (!a2dpValidateServiceCaps(&ptr[2], packet_size-2, FALSE, FALSE, &error_cat, &error_code))
		{
			a2dpResetSignalling(a2dp, FALSE, FALSE);
			return;
		}

		if (a2dp->signal_conn.signalling_state == avdtp_state_reading_caps)
		{
			/* change state (stop watchdog) */
			a2dpSetSignallingState(a2dp, avdtp_state_processing_caps);

			/*
				Try to build the SET_CONFIGURATION command.
			*/
			if (!startBuildingConfiguration(a2dp, &ptr[2], packet_size-2))
			{
				/* Caps not compatible.
				   Is there another remote SEID to try? */
				a2dp->sep.current_discovered_remote_seid++;
				if (a2dp->sep.current_discovered_remote_seid < a2dp->sep.max_discovered_remote_seids)
				{
					a2dp->sep.remote_seid = a2dp->sep.list_discovered_remote_seids[a2dp->sep.current_discovered_remote_seid];
					PRINT(("local_seid = %d  remote_seid = %d\n",a2dp->sep.list_preferred_local_seids[a2dp->sep.current_preferred_local_seid], a2dp->sep.remote_seid));
					/* re-enter the state to reset watchdog */
					a2dpSetSignallingState(a2dp, avdtp_state_reading_caps);
					(void)a2dpSendGetCapabilities(a2dp, a2dp->sep.remote_seid);
					return;
				}
				else
				{
					/* Select next local SEID */
					a2dp->sep.current_preferred_local_seid++;
					if (a2dp->sep.current_preferred_local_seid < a2dp->sep.max_preferred_local_seids)
					{
						sep_type *pSeps = getSepInstanceBySeid(a2dp, a2dp->sep.list_preferred_local_seids[a2dp->sep.current_preferred_local_seid]);

						if (pSeps == NULL)
						{
							/* If the SEID is not valid then don't proceed. */
							a2dpResetSignalling(a2dp, FALSE, FALSE);
							return;
						}

						a2dp->sep.current_sep = pSeps;
						a2dp->sep.current_discovered_remote_seid = 0;
						a2dp->sep.remote_seid = a2dp->sep.list_discovered_remote_seids[0];
						PRINT(("local_seid = %d  remote_seid = %d\n",a2dp->sep.list_preferred_local_seids[a2dp->sep.current_preferred_local_seid], a2dp->sep.remote_seid));
						/* re-enter the state to reset watchdog */
						a2dpSetSignallingState(a2dp, avdtp_state_reading_caps);
						(void)a2dpSendGetCapabilities(a2dp, a2dp->sep.remote_seid);
						return;
					}
					else
					{
						/* oh dear, no match and we've run out of SEPs to query */
						a2dpResetSignalling(a2dp, FALSE, FALSE);
					}
				}
			}
		}
		else if (a2dp->signal_conn.signalling_state == avdtp_state_reconfig_reading_caps)
		{
			/* try building RECONFIGURE */
			if (!startBuildingReconfigure(a2dp, &ptr[2], packet_size-2))
			{
				/* caps aren't compatible */
				a2dpSendReconfigureCfm(a2dp, a2dp_operation_fail);
				
				/* back to open */
				a2dpSetSignallingState(a2dp, avdtp_state_open);
			}
			else
			{
				a2dpSetSignallingState(a2dp, avdtp_state_reconfiguring);
			}
		}
	}
	else
	{
		if (a2dp->signal_conn.signalling_state == avdtp_state_reading_caps)
		{
			/* request was rejected - abort */
			a2dpResetSignalling(a2dp, FALSE, FALSE);
		}
		else
		{
			/* reconfiguration was rejected */
			a2dpSendReconfigureCfm(a2dp, a2dp_rejected_by_remote_device);
			
			/* back to open */
			a2dpSetSignallingState(a2dp, avdtp_state_open);
		}
	}
}


/****************************************************************************
NAME	
	handleSetConfigurationResponse

DESCRIPTION
	Handle Set Configuration response from remote device.
	
RETURNS
	void
*/
static void handleSetConfigurationResponse(A2DP* a2dp, const uint8* ptr, uint16 packet_size)
{
	/* we only expect this if we are the initiator in the discovering state */
	if (a2dp->signal_conn.signalling_state != avdtp_state_configuring)
		return;

	if ((ptr[0] & 3) == avdtp_message_type_accept)
	{
		/* Mark this SEP as in use */
		a2dp->sep.current_sep->configured = 1;

		/* We are now configured, so try and open the link. */
		a2dpSetSignallingState(a2dp, avdtp_state_local_opening);
		(void) a2dpSendOpen(a2dp);
	}
	else
	{
		/* request was rejected - abort */
		a2dpResetSignalling(a2dp, FALSE, FALSE);

		packet_size = packet_size;
	}
}


/****************************************************************************
NAME	
	handleReconfigureResponse

DESCRIPTION
	Handle Reconfigure response from remote device.
	
RETURNS
	void
*/
static void handleReconfigureResponse(A2DP* a2dp, const uint8* ptr)
{
	/* we only expect this if we are the initiator in the discovering state */
	if (a2dp->signal_conn.signalling_state != avdtp_state_reconfiguring)
		return;

	if ((ptr[0] & 3) == avdtp_message_type_accept)
	{
		/* tell application if it's good news! */
		if (storeCurrentConfiguration(a2dp))
		{
			/* send new codec settings to app */
			MAKE_A2DP_MESSAGE(A2DP_INTERNAL_SEND_CODEC_PARAMS_REQ);
			message->send_reconfigure_message = 1;
			MessageSend(&a2dp->task, A2DP_INTERNAL_SEND_CODEC_PARAMS_REQ, message);
		}
		else
		{
			a2dpSendReconfigureCfm(a2dp, a2dp_operation_fail);
		}	
	}
	else
	{
		/* failed */
		a2dpSendReconfigureCfm(a2dp, a2dp_rejected_by_remote_device);
	}

	/* We now return to the open state. */
	a2dpSetSignallingState(a2dp, avdtp_state_open);
}


/****************************************************************************
NAME	
	handleOpenResponse

DESCRIPTION
	Handle Open response from remote device.
	
RETURNS
	void
*/
static void handleOpenResponse(A2DP* a2dp, const uint8* ptr, uint16 packet_size)
{
	/* we only expect this if we are the initiator in the discovering state */
	if (a2dp->signal_conn.signalling_state != avdtp_state_local_opening)		
		return;

	if ((ptr[0] & 3) == avdtp_message_type_accept)
	{
		/* 
			On acceptance of AVDTP_OPEN, the initiator shall open the transport 
			channels. We know there must always be a media channel, so start 
			by opening it! 
		*/
		a2dpOpenTransportChannel(a2dp, a2dp->sep.current_sep->sep_config->flush_timeout);
	}
	else
	{
		/* request was rejected - abort */
		a2dpAbortSignalling(a2dp, FALSE, FALSE);

		packet_size = packet_size;
	}
}


/****************************************************************************
NAME	
	handleStartResponse

DESCRIPTION
	Handle Start response from remote device.
	
RETURNS
	void
*/
static void handleStartResponse(A2DP* a2dp, const uint8* ptr)
{
	if ((a2dp->signal_conn.signalling_state != avdtp_state_open) && 
		(a2dp->signal_conn.signalling_state != avdtp_state_local_starting) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_streaming))
	{
        sendStartCfm(a2dp, a2dp->media_conn.sink, a2dp_wrong_state);
		return;
	}

	if ((ptr[0] & 3) == avdtp_message_type_accept)
	{
		/* tell application the good news! */
		sendStartCfm(a2dp, a2dp->media_conn.sink, a2dp_success);

		/* end point is now streaming */
		a2dpSetSignallingState(a2dp, avdtp_state_streaming);
	}
	else
	{
		/* pass on failure */
		if (a2dp->signal_conn.signalling_state == avdtp_state_local_starting)
			a2dpSetSignallingState(a2dp, avdtp_state_open);

		sendStartCfm(a2dp, a2dp->media_conn.sink, a2dp_rejected_by_remote_device);
	}
}


/****************************************************************************
NAME	
	handleSuspendResponse

DESCRIPTION
	Handle Suspend response from remote device.
	
RETURNS
	void
*/
static void handleSuspendResponse(A2DP* a2dp, const uint8* ptr)
{
	if (a2dp->signal_conn.signalling_state != avdtp_state_local_suspending)
		return;

	if ((ptr[0] & 3) == avdtp_message_type_accept)
	{
		/* success! */
		sendSuspendCfm(a2dp, a2dp->media_conn.sink, a2dp_success);

		/* suspend was successful */
		a2dpSetSignallingState(a2dp, avdtp_state_open);
	}
	else
	{
		/* Update our local state back to streaming, suspend failed */
		a2dpSetSignallingState(a2dp, avdtp_state_streaming);

		/* pass on failure */
		sendSuspendCfm(a2dp, a2dp->media_conn.sink, a2dp_rejected_by_remote_device);
	}
}


/****************************************************************************
NAME	
	handleCloseResponse

DESCRIPTION
	Handle Close response from remote device.
	
RETURNS
	void
*/
static void handleCloseResponse(A2DP* a2dp, const uint8* ptr)
{
	if ((a2dp->signal_conn.signalling_state != avdtp_state_open) && 
		(a2dp->signal_conn.signalling_state != avdtp_state_local_starting) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_local_suspending) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_reconfig_reading_caps) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_reconfiguring) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_streaming) &&
		/* To handle cross-over of the CLOSE command, we process the response even
		   if we've just accepted the request from the other side.
		   The SEP connection code is smart enough to handle both sides trying to
		   close L2CAP connections! */
		(a2dp->signal_conn.signalling_state != avdtp_state_remote_closing) && 
        (a2dp->signal_conn.signalling_state != avdtp_state_local_closing)) 
	{
		return;
	}

	if ((ptr[0] & 3) == avdtp_message_type_accept)
	{
		/* We are now closing. */
		a2dpSetSignallingState(a2dp, avdtp_state_local_closing);

		/* Close transport channels */
		a2dpCloseMediaConnection(a2dp);
	}
	else
	{
		/* We don't take no for an answer, so we'll abort instead. */
		a2dpAbortSignalling(a2dp, FALSE, FALSE);
	}
}


/****************************************************************************
NAME	
	handleAbortResponse

DESCRIPTION
	Handle Abort response from remote device.
	
RETURNS
	void
*/
static void handleAbortResponse(A2DP* a2dp)
{
	/* the remote device has accepted the abort, so reset the signalling. */
	a2dpResetSignalling(a2dp, FALSE, FALSE);
}


/*****************************************************************************/
void handleReceiveResponse(A2DP* a2dp, const uint8* ptr, uint16 packet_size)
{
	if (a2dp->signal_conn.pending_transaction_label != (ptr[0] >> 4))
		return;

	/* clear pending transaction as we have a response */
	a2dp->signal_conn.pending_transaction_label = 0;

	/* check the PDU is reasonable */
	if (!validateResponsePDU(packet_size))
		return;
	
	PRINT(("handleReceiveResponse %d\n",ptr[1]));

	switch (ptr[1])
	{
		case avdtp_discover:
			handleDiscoverResponse(a2dp, ptr, packet_size);
			break;

		case avdtp_get_capabilities:
			handleGetCapabilitiesResponse(a2dp, ptr, packet_size);
			break;

		case avdtp_set_configuration:
			handleSetConfigurationResponse(a2dp, ptr, packet_size);
			break;

		case avdtp_get_configuration:
			/* We never send the request, so don't expect a reply! */
			break;

		case avdtp_reconfigure:
			handleReconfigureResponse(a2dp, ptr);
			break;

		case avdtp_open:
			handleOpenResponse(a2dp, ptr, packet_size);
			break;

		case avdtp_start:
			handleStartResponse(a2dp, ptr);
			break;

		case avdtp_close:
			handleCloseResponse(a2dp, ptr);
			break;

		case avdtp_suspend:
			handleSuspendResponse(a2dp, ptr);
			break;

		case avdtp_abort:
			handleAbortResponse(a2dp);
			break;

		default:
			break;
	}
}
