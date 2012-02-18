/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbaps_int_handler.h
    
DESCRIPTION
	PhoneBook Access Profile Server Library - handler functions for internal messages.

*/

#include <vm.h>
#include <print.h>
#include <goep.h>

#include "syncs.h"
#include "syncs_private.h"

static void handleIntConnectResponse(syncsState *state, SYNCS_INT_CONNECTION_RESP_T *msg);
static void handleIntAuthChallenge(syncsState *state, SYNCS_INT_AUTH_CHALLENGE_T *msg);

static void handleIntGetSendFirstSrc(syncsState *state, SYNCS_GET_SEND_FIRST_SRC_T *msg);
static void handleIntGetSendNext(syncsState *state, SYNCS_GET_SEND_NEXT_T *msg);
static void handleIntGetSendNextSrc(syncsState *state, SYNCS_GET_SEND_NEXT_SRC_T *msg);

/****************************************************************************
NAME	
    pbapsIntHandler

DESCRIPTION
    Handler for messages received by the PBABS Task.
*/
void syncsIntHandler(Task task, MessageId id, Message message)
{
	/* Get task control block */
	syncsState *state = (syncsState*)task;
	
	if ((id >= SYNCS_INT_CONNECTION_RESP) && (id <= SYNCS_INT_ENDOFLIST))
	{
		switch (id)
		{
		case SYNCS_INT_CONNECTION_RESP:
			handleIntConnectResponse(state, (SYNCS_INT_CONNECTION_RESP_T*)message);
			break;
		case SYNCS_INT_AUTH_CHALLENGE:
			handleIntAuthChallenge(state, (SYNCS_INT_AUTH_CHALLENGE_T*)message);
			break;
			
		case SYNCS_GET_SEND_FIRST_SRC:
			handleIntGetSendFirstSrc(state, (SYNCS_GET_SEND_FIRST_SRC_T*)message);
			break;
		case SYNCS_GET_SEND_NEXT:
			handleIntGetSendNext(state, (SYNCS_GET_SEND_NEXT_T*)message);
			break;
		case SYNCS_GET_SEND_NEXT_SRC:
			handleIntGetSendNextSrc(state, (SYNCS_GET_SEND_NEXT_SRC_T*)message);
			break;
			
			default:
			PRINT(("SYNCS Unhandled message : 0x%X\n",id));
			break;
		};
	}
	else
	{
		syncsGoepHandler(state, id, message);
	}
}

static void handleIntConnectResponse(syncsState *state, SYNCS_INT_CONNECTION_RESP_T *msg)
{
    PRINT(("handleIntConnectResponse\n"));
    
    if (state->currCom!= syncs_com_Connecting)
    { /* Not idle, Send Error Message */
		syncsMsgSendConnectCfm(state, syncs_wrong_state, 0);
    }
    else
    {
		if (msg->accept)
		{ /* Send accept response code */
			GoepConnectResponse(state->handle, goep_svr_resp_OK, msg->pktSize);
		}
		else
		{ /* Send reject response code */
			GoepConnectResponse(state->handle, goep_svr_resp_BadRequest, msg->pktSize);
	        state->currCom= syncs_com_None;
		}
    }
}

static void handleIntAuthChallenge(syncsState *state, SYNCS_INT_AUTH_CHALLENGE_T *msg)
{
    PRINT(("handleIntAuthChallenge\n"));
    
    if (state->currCom!= syncs_com_Connecting)
    { /* Not idle, Send Error Message */
		syncsMsgSendConnectCfm(state, syncs_wrong_state, 0);
    }
    else
    {
		GoepConnectAuthChallenge(state->handle, msg->nonce, msg->options, msg->size_realm, msg->realm);
    }
}

static void handleIntGetSendFirstSrc(syncsState *state, SYNCS_GET_SEND_FIRST_SRC_T *msg)
{
    PRINT(("handleIntGetSendFirstSrc\n"));

    if (state->currCom != msg->command)
    {
		PRINT(("       Error\n"));
    }
    else
    {
		if (msg->result == syncs_get_ok)
		{
			PRINT(("	   Start\n"));
		}
		else
		{
			GoepRemoteGetResponse(state->handle, goep_svr_resp_BadRequest, 0,
					0, NULL, 0, NULL, 0, NULL, TRUE);
		}
    }
}

static void handleIntGetSendNext(syncsState *state, SYNCS_GET_SEND_NEXT_T *msg)
{
    PRINT(("handleIntGetSendNext\n"));

    if (state->currCom != msg->command)
    {
		PRINT(("       Error\n"));
    }
    else
    {
		GoepRemoteGetResponsePkt(state->handle, msg->size_packet, msg->packet, msg->lastPacket);
    }
}

static void handleIntGetSendNextSrc(syncsState *state, SYNCS_GET_SEND_NEXT_SRC_T *msg)
{
    PRINT(("handleIntGetSendNextSrc\n"));

    if (state->currCom != msg->command)
    {
		PRINT(("       Error\n"));
    }
    else
    {
		GoepRemoteGetResponsePktSource(state->handle, msg->size_packet, msg->src, msg->lastPacket);
    }
}

