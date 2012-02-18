/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    ftpc_messages.c
    
DESCRIPTION
	Functions to send messages to the task using this OPPC instance.
*/

#include <panic.h>
#include <message.h>
#include <source.h>

#include "oppc.h"
#include "oppc_private.h"

void oppcMsgSendConnectCfm(oppcState *state, oppc_lib_status status, uint16 pktSize)
{
	MAKE_OPPC_MESSAGE(OPPC_CONNECT_CFM);
	message->oppc = (OPPC*)state;
	message->status = status;
	message->packetSize = pktSize;
            
    MessageSend(state->theAppTask, OPPC_CONNECT_CFM, message);
}

void oppcMsgSendDisConnectCfm(oppcState *state, oppc_lib_status status)
{
	MAKE_OPPC_MESSAGE(OPPC_DISCONNECT_IND);
	message->oppc = (OPPC*)state;
	message->status = status;
            
    MessageSend(state->theAppTask, OPPC_DISCONNECT_IND, message);
}

void oppcMsgPullBCCompleteInd(oppcState *state, oppc_lib_status status)
{
	MAKE_OPPC_MESSAGE(OPPC_PULL_BC_COMPLETE_IND);
	message->oppc = (OPPC*)state;
	message->status = status;
            
    MessageSend(state->theAppTask, OPPC_PULL_BC_COMPLETE_IND, message);
}

void oppcMsgPushCompleteInd(oppcState *state, oppc_lib_status status)
{
	MAKE_OPPC_MESSAGE(OPPC_PUSH_COMPLETE_IND);
	message->oppc = (OPPC*)state;
	message->status = status;
            
    MessageSend(state->theAppTask, OPPC_PUSH_COMPLETE_IND, message);
}
