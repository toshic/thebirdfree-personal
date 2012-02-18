/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_handler.c
    
DESCRIPTION
	Message Handler for the Generic Object Exchange Profile (GOEP) library.
	Receives messages from the Connection Manager.

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/

#include <message.h>
#include <app/message/system_message.h>
#include <connection.h>

#include <stream.h>
#include <source.h>

#include <panic.h>
#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/sdc_prim.h>
#include <string.h>
#include <service.h>

#include <sdp_parse.h>
#include "goep.h"
#include "goep_private.h"
#include "goep_cli_state.h"
#include "goep_packet.h"

static void handleRfcommRegisterCfm(goepState *state, CL_RFCOMM_REGISTER_CFM_T *msg);
static void handleSDPServSrchAttrCfm(goepState *state, CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *msg);
static void handleRfcConnectCfm(goepState *state, CL_RFCOMM_CONNECT_CFM_T *msg);
static void handleRfcConnectInd(goepState *state, CL_RFCOMM_CONNECT_IND_T *msg);
static void handleRfcDisconnectInd(goepState *state, CL_RFCOMM_DISCONNECT_IND_T *msg);

static void handleMoreData(goepState *state , Source source);

static void cleanConInfo(goepState *state);



void goepHandler(Task task, MessageId id, Message message)
{
	/* Get task control block */
	goepState *state = (goepState*)task;

    switch(id)
    {
	case CL_RFCOMM_REGISTER_CFM:
		handleRfcommRegisterCfm(state, (CL_RFCOMM_REGISTER_CFM_T*)message);
		break;
	case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
		handleSDPServSrchAttrCfm(state, (CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T*)message);
		break;
	case CL_RFCOMM_CONNECT_CFM:
		handleRfcConnectCfm(state, (CL_RFCOMM_CONNECT_CFM_T*)message);
		break;
	case CL_RFCOMM_CONNECT_IND:
		handleRfcConnectInd(state, (CL_RFCOMM_CONNECT_IND_T*)message);
		break;
	case CL_RFCOMM_DISCONNECT_IND:
		handleRfcDisconnectInd(state, (CL_RFCOMM_DISCONNECT_IND_T*)message);
		break;
	case MESSAGE_MORE_DATA:
        handleMoreData(state , ((MessageMoreData*)message)->source);
		break;
	default:
		GOEP_DEBUG(("GOEP Unhandled Message : %d , 0x%0X\n",id,id) ); 
		break;
	}
}

static void handleRfcommRegisterCfm(goepState *state, CL_RFCOMM_REGISTER_CFM_T *msg)
{
	MAKE_GOEP_MESSAGE(GOEP_CHANNEL_IND);
	GOEP_DEBUG(("CL_RFCOMM_REGISTER_CFM\n"));
	
	message->goep = state;
	
	if (msg->status==success)
	{
		message->status=goep_success;
		message->channel = msg->server_channel;
	}
	else
	{
		
		message->status=goep_failure;
	}
	MessageSend(state->theApp, GOEP_CHANNEL_IND, message);
}

static void handleSDPServSrchAttrCfm(goepState *state, CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *msg)
{
	GOEP_DEBUG(("CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM\n")); 
	
	if (msg->status==success)
	{
		uint8 *ptr;
		uint8 size;
		uint8 rem_chan;
		uint8* rfcomm_channels;
		uint8 size_rfcomm_channels = 1, channels_found = 0;
		
		ptr = msg->attributes;
		size = msg->size_attributes;
		
		rfcomm_channels = PanicUnlessMalloc(size_rfcomm_channels * sizeof(uint8));
		
		if (SdpParseGetMultipleRfcommServerChannels(size, ptr, size_rfcomm_channels, &rfcomm_channels, &channels_found))
		{
			GOEP_DEBUG(("ConnectionRfcommConnectRequest call\n"));
			rem_chan = rfcomm_channels[0];
			free(rfcomm_channels);
			ConnectionRfcommConnectRequest(&state->task, &state->bdAddr, state->rfcChan, rem_chan, NULL);
		}
		else
		{
			state->state= goep_initialised;
		
			goepMsgSendConnectConfirm(state, goep_server_unsupported, 0);
		}
	}
	else
	{
		state->state= goep_initialised;
		
		cleanConInfo(state);
		
		goepMsgSendConnectConfirm(state, goep_failure, 0);
	}
}

static void handleRfcConnectCfm(goepState *state, CL_RFCOMM_CONNECT_CFM_T *msg)
{
	GOEP_DEBUG(("CL_RFCOMM_CONNECT_CFM\n"));
	
	if (msg->status==success)
	{
		state->sink=msg->sink;
		
		if (state->role == goep_Client)
		{
	        GOEP_DEBUG(("Send Connect\n"));
			
			if (goepSendConnect(state)!=goep_success)
			{ /* Failed to send packet */
				state->state= goep_initialised;
            	GOEP_DEBUG(("Failed\n"));
				goepMsgSendConnectConfirm(state, goep_failure, 0);
			}
		}

		/* Check for data in the buffer */
		handleMoreData(state, StreamSourceFromSink(msg->sink));
	}
	else
	{
        GOEP_DEBUG(("No Connect\n"));
		state->state= goep_initialised;
		
		cleanConInfo(state);
	
		goepMsgSendConnectConfirm(state, goep_failure, 0);
	}
}

static void handleRfcConnectInd(goepState *state, CL_RFCOMM_CONNECT_IND_T *msg)
{
	GOEP_DEBUG(("CL_RFCOMM_CONNECT_IND\n"));
	
	if (state->role == goep_Server)
	{
		if ((state->state == goep_initialised) || (state->state == goep_connecting))
		{
			state->state = goep_connecting;
			memmove(&(state->bdAddr), &msg->bd_addr, sizeof(bdaddr));
			
			ConnectionRfcommConnectResponse(&state->task, TRUE, &msg->bd_addr, msg->server_channel, NULL);
		}
		else
		{
			ConnectionRfcommConnectResponse(&state->task, FALSE, &msg->bd_addr, msg->server_channel, NULL);
		}
	}
	else
	{ /* Don't have a valid client for this channel, reject the connection */
		ConnectionRfcommConnectResponse(&state->task, FALSE, &msg->bd_addr, msg->server_channel, NULL);
	}
}

static void handleMoreData(goepState *state , Source source)
{
	GOEP_DEBUG(("MESSAGE_MORE_DATA\n") );
	
	if (state->srcUsed == 0)
		handleClientStates(state, source);
}

static void handleRfcDisconnectInd(goepState *state, CL_RFCOMM_DISCONNECT_IND_T *msg)
{
    GOEP_DEBUG(("CL_RFCOMM_DISCONNECT_IND\n") ); 
	
	cleanConInfo(state);
	
    if ((state->state != goep_connect_abort) && (state->state != goep_connect_refused))
    { /* Don't send if something went wrong with the connect and we are aborting it */
		if (state->state == goep_connect_cancel)
			goepMsgSendConnectConfirm(state, goep_connect_cancelled, 0);
		else
			goepMsgSendDisconnectConfirm(state, ((state->state==goep_disconnecting) ? goep_success:goep_remote_disconnect));
    }
	
    state->state= goep_initialised;
}

static void cleanConInfo(goepState *state)
{
	/* Clean up connection data and return to initialised state */
    if (state->conInfo)
    	free(state->conInfo);

    state->conLen=0;
    state->conInfo=NULL;
    state->conID=0;
    state->useConID=FALSE;
}
