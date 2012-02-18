/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbaps_messages.h
    
DESCRIPTION
	PhoneBook Access Profile Server Library - message send functions.

*/

#include <vm.h>
#include <print.h>

#include "syncs.h"
#include "syncs_private.h"


void syncsMsgSendInitCfm(syncsState *state, syncs_lib_status status)
{
	MAKE_SYNCS_MESSAGE(SYNCS_INIT_CFM);
	if (status == syncs_success)
		message->syncs = (SYNCS*)state;
	else
		message->syncs = (SYNCS*)NULL;
	message->status = status;
            
    MessageSend(state->theAppTask, SYNCS_INIT_CFM, message);
}

void syncsMsgSendConnectCfm(syncsState *state, syncs_lib_status status, uint16 pktSize)
{
	MAKE_SYNCS_MESSAGE(SYNCS_CONNECT_CFM);
	message->syncs = (SYNCS*)state;
	message->status = status;
	message->maxPacketLen = pktSize;
            
    MessageSend(state->theAppTask, SYNCS_CONNECT_CFM, message);
}


