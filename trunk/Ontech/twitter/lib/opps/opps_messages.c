/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    opps_messages.c
    
DESCRIPTION
	Functions to send messages to the task using this OPPS instance.
*/

#include <panic.h>
#include <message.h>
#include <source.h>

#include "opps.h"
#include "opps_private.h"

void oppsMsgSendInitCfm(oppsState *state, opps_lib_status status)
{
	MAKE_OPPS_MESSAGE(OPPS_INIT_CFM);
	message->opps = (OPPS*)state;
	message->status = status;
            
    MessageSend(state->theAppTask, OPPS_INIT_CFM, message);
}

void oppsMsgSendConnectCfm(oppsState *state, opps_lib_status status, uint16 pktSize)
{
	MAKE_OPPS_MESSAGE(OPPS_CONNECT_CFM);
	message->opps = (OPPS*)state;
	message->status = status;
	message->maxPacketLen = pktSize;
            
    MessageSend(state->theAppTask, OPPS_CONNECT_CFM, message);
}

void oppsMsgSendPushObjStartInd(oppsState *state, Source src, uint16 nameOffset, uint16 nameLength,
				uint16 typeOffset, uint16 typeLength, uint32 totalLength, uint16 packetLen,
				uint16 packetOffset, bool moreData)
{
	MAKE_OPPS_MESSAGE(OPPS_PUSH_OBJ_START_IND);
	message->opps = (OPPS*)state;
	message->src = src;
	message->nameOffset = nameOffset;
	message->nameLength = nameLength;
	message->typeLength = typeLength;
	message->typeOffset = typeOffset;
	message->totalLength = totalLength;
	message->packetLen = packetLen;
	message->packetOffset = packetOffset;
	message->moreData = moreData;
            
    MessageSend(state->theAppTask, OPPS_PUSH_OBJ_START_IND, message);
}

void oppsMsgSendPushBcStartInd(oppsState *state, Source src, uint16 nameOffset, uint16 nameLength,
										uint32 totalLength, uint16 packetLen, uint16 packetOffset,
										bool moreData)
{
	MAKE_OPPS_MESSAGE(OPPS_PUSH_BC_START_IND);
	message->opps = (OPPS*)state;
	message->src = src;
	message->nameOffset = nameOffset;
	message->nameLength = nameLength;
	message->totalLength = totalLength;
	message->packetLen = packetLen;
	message->packetOffset = packetOffset;
	message->moreData = moreData;
            
    MessageSend(state->theAppTask, OPPS_PUSH_BC_START_IND, message);
}

void oppsMsgSendPushObjectCompleteInd(oppsState *state, opps_lib_status status)
{
	MAKE_OPPS_MESSAGE(OPPS_PUSH_OBJ_COMPLETE_IND);
	message->opps = (OPPS*)state;
	message->status = status;
            
    MessageSend(state->theAppTask, OPPS_PUSH_OBJ_COMPLETE_IND, message);
}

void oppsMsgSendPushBcCompleteInd(oppsState *state, opps_lib_status status)
{
	MAKE_OPPS_MESSAGE(OPPS_PUSH_BC_COMPLETE_IND);
	message->opps = (OPPS*)state;
	message->status = status;
            
    MessageSend(state->theAppTask, OPPS_PUSH_BC_COMPLETE_IND, message);
}

void oppsMsgSendPullBcCompleteInd(oppsState *state, opps_lib_status status)
{
	MAKE_OPPS_MESSAGE(OPPS_PULL_BC_COMPLETE_IND);
	message->opps = (OPPS*)state;
	message->status = status;
            
    MessageSend(state->theAppTask, OPPS_PULL_BC_COMPLETE_IND, message);
}

