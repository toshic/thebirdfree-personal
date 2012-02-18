/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_init.c        

DESCRIPTION
	This file contains the initialisation code for the A2DP profile library.

NOTES

*/



/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_init.h"
#include "a2dp_l2cap_handler.h"
#include "a2dp_private.h"
#include "a2dp_profile_handler.h"

#include <message.h>
#include <panic.h>
#include <print.h>
#include <stdlib.h>
#include <string.h>

/*
  SDP Service Record generated from a2dp_sink.sdp by sdpgen.pl
*/
static const uint8 a2dp_sink_service_record[] =
{
  0x09,     /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35,     /* DataElSeq 3 bytes */
  0x03,
    0x19,   /* uuid AudioSink(0x110b) */
    0x11,
    0x0b,
  0x09,     /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35,     /* DataElSeq 16 bytes */
  0x10,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
      0x09, /* uint16 0x0019 */
        0x00,
        0x19,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AVDTP(0x0019) */
      0x00,
      0x19,
      0x09, /* uint16 0x0102 */
        0x01,
        0x02,
  0x09,     /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35,     /* DataElSeq 8 bytes */
  0x08,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AdvancedAudioDistribution(0x110d) */
      0x11,
      0x0d,
      0x09, /* uint16 0x0102 */
        0x01,
        0x02,
  0x09,     /* SupportedFeatures(0x0311) = "0x0001" */
    0x03,
    0x11,
  0x09,     /* uint16 0x0001 */
    0x00,
    0x01,
}; /* 48 bytes */


/*
  SDP Service Record generated from a2dp_source.sdp by sdpgen.pl
*/
static const uint8 a2dp_source_service_record[] =
{
  0x09,     /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35,     /* DataElSeq 3 bytes */
  0x03,
    0x19,   /* uuid AudioSource(0x110a) */
    0x11,
    0x0a,
  0x09,     /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35,     /* DataElSeq 16 bytes */
  0x10,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
      0x09, /* uint16 0x0019 */
        0x00,
        0x19,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AVDTP(0x0019) */
      0x00,
      0x19,
      0x09, /* uint16 0x0102 */
        0x01,
        0x02,
  0x09,     /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35,     /* DataElSeq 8 bytes */
  0x08,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AdvancedAudioDistribution(0x110d) */
      0x11,
      0x0d,
      0x09, /* uint16 0x0102 */
        0x01,
        0x02,
  0x09,     /* SupportedFeatures(0x0311) = "0x0001" */
    0x03,
    0x11,
  0x09,     /* uint16 0x0001 */
    0x00,
    0x01,
}; /* 48 bytes */


/****************************************************************************/
static bool validateSeps(sep_data_type *seps, uint16 size_seps)
{
	uint16 i, j;

	for (i = 0; i < size_seps; i++)
	{
		/* if no config is supplied then invalid params */
		if (!seps[i].sep_config)
			return FALSE;

		for (j = 0; j < i; ++j)
		{
			/* if any SEIDs are equal then invalid params */
			if (seps[i].sep_config->seid == seps[j].sep_config->seid)
				return FALSE;
		}
	}

	return TRUE;
}


/****************************************************************************/
void a2dpInitTask(A2DP *a2dp, Task clientTask)
{
    /* Set the handler function */
    a2dp->task.handler = a2dpProfileHandler;

    /* Set up the lib client */
    a2dp->clientTask = clientTask;

	/* Set up the signalling connection */
	a2dp->signal_conn.connection_state = avdtp_connection_idle;
	a2dp->signal_conn.signalling_state = avdtp_state_idle;
	a2dp->signal_conn.reassembled_packet = NULL;
	a2dp->signal_conn.reassembled_packet_length = 0;
	a2dp->signal_conn.transaction_label = 0;
	a2dp->signal_conn.pending_transaction_label = 0;
	a2dp->signal_conn.sink = 0;
	a2dp->signal_conn.connect_then_open_media = FALSE;
	a2dp->signal_conn.waiting_response = 0;

	/* Set up the media connection */
	a2dp->media_conn.sink = 0;
	a2dp->media_conn.media_connecting = 0;
	a2dp->media_conn.media_connected = 0;

	/* Set up the SEP info */
	a2dp->sep.remote_seid = 0;
	a2dp->sep.current_sep = NULL;
	a2dp->sep.sep_list = NULL;
	a2dp->sep.configured_service_caps = NULL;
	a2dp->sep.configured_service_caps_size = 0;
	a2dp->sep.list_discovered_remote_seids = NULL;
	a2dp->sep.list_preferred_local_seids = NULL;
}


/****************************************************************************/
void a2dpSendInitCfmToClient(A2DP *a2dp, a2dp_status_code status, device_sep_list *seps)
{
    MAKE_A2DP_MESSAGE(A2DP_INIT_CFM);
    message->status = status;
	message->sep_list = seps;
    MessageSend(a2dp->clientTask, A2DP_INIT_CFM, message);

    /* Free the a2dp task */
    free(a2dp);
}


/****************************************************************************/
void A2dpInit(Task clientTask, uint16 role, service_record_type *service_records, uint16 size_seps, sep_data_type *seps)
{
    A2DP *a2dp = PanicUnlessNew(A2DP);
	sep_type *pSeps = (sep_type *) seps;
	device_sep_list *sep_entry = NULL;
	uint16 i = 0;

    /* Initialise the task data */
    a2dpInitTask(a2dp, clientTask);

	if (!seps || !size_seps || !validateSeps(seps, size_seps))
	{
		a2dpSendInitCfmToClient(a2dp, a2dp_invalid_parameters, 0);
		return;
	}
	
	/* Use this variable to keep track of the service records registered */
	a2dp->sep.configured_service_caps_size = 0;

	if (service_records)
	{
		if (service_records->size_service_record_a && service_records->service_record_a)
		{
			/* Client has supplied their own record so register it without checking */
			ConnectionRegisterServiceRecord(&a2dp->task, service_records->size_service_record_a, service_records->service_record_a);
			a2dp->sep.configured_service_caps_size++;
		}
		if (service_records->size_service_record_b && service_records->service_record_b)
		{
			/* Client has supplied their own record so register it without checking */
			ConnectionRegisterServiceRecord(&a2dp->task, service_records->size_service_record_b, service_records->service_record_b);
			a2dp->sep.configured_service_caps_size++;
		}
	}
	else
	{
		/* Client using default library record */
		if (role & A2DP_INIT_ROLE_SINK) 
		{
			ConnectionRegisterServiceRecord(&a2dp->task, sizeof(a2dp_sink_service_record), a2dp_sink_service_record);
			PRINT(("Register Sink Service Rec\n"));
			a2dp->sep.configured_service_caps_size++;
		}
		if (role & A2DP_INIT_ROLE_SOURCE)
		{
			ConnectionRegisterServiceRecord(&a2dp->task, sizeof(a2dp_source_service_record), a2dp_source_service_record);
			PRINT(("Register Source Service Rec\n"));
			a2dp->sep.configured_service_caps_size++;
		}
	} 

	sep_entry = (device_sep_list *)malloc(sizeof(uint16) + sizeof(sep_type)*size_seps); 

	if (!sep_entry)
	{
		a2dpSendInitCfmToClient(a2dp, a2dp_insufficient_memory, 0);
		return;
	}

	if (!a2dp->sep.configured_service_caps_size)
	{
		/* Skip the service record registering if the user doesn't require any at this point. */
		a2dpRegisterL2cap(a2dp);
	}

	sep_entry->size_seps = size_seps;
	memmove(sep_entry->seps, pSeps, sizeof(sep_type)*size_seps); 

	pSeps = sep_entry->seps;
	for (i = 0; i < size_seps; i++)
	{
		pSeps->configured = 0;

		PRINT(("Store SEP info:\n"));
		PRINT(("	SEID=%d\n",pSeps->sep_config->seid));
		PRINT(("	in_use=%d\n",pSeps->in_use));
		PRINT(("	configured=%d\n",pSeps->configured));

		PRINT(("	resource_id=%d\n",pSeps->sep_config->resource_id));
		PRINT(("	media_type=%d\n",pSeps->sep_config->media_type));
		PRINT(("	role=%d\n",pSeps->sep_config->role));
		PRINT(("	library_selects_settings=%d\n",pSeps->sep_config->library_selects_settings));
		PRINT(("	flush_timeout=%d\n",pSeps->sep_config->flush_timeout));
		
		pSeps++;
	}

	/* Store the SEP list that will be returned to the app once initialisation of the library is complete */
	a2dp->sep.sep_list = sep_entry;
}
