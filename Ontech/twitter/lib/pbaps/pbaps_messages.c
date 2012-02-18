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
#include <pbap_common.h>

#include "pbaps.h"
#include "pbaps_private.h"


void pbapsMsgSendInitCfm(pbapsState *state, pbaps_lib_status status)
{
	MAKE_PBAPS_MESSAGE(PBAPS_INIT_CFM);
	if (status == pbaps_success)
		message->pbaps = (PBAPS*)state;
	else
		message->pbaps = (PBAPS*)NULL;
	message->status = status;
            
    MessageSend(state->theAppTask, PBAPS_INIT_CFM, message);
}

void pbapsMsgSendConnectCfm(pbapsState *state, pbaps_lib_status status, uint16 pktSize)
{
	MAKE_PBAPS_MESSAGE(PBAPS_CONNECT_CFM);
	message->pbaps = (PBAPS*)state;
	message->status = status;
	message->maxPacketLen = pktSize;
            
    MessageSend(state->theAppTask, PBAPS_CONNECT_CFM, message);
}

void pbapsMsgSendGetvCardListStartInd(pbapsState *state, pbaps_lib_status status,
										pbap_order_values order, uint8 *srchVal, uint16 size_srchVal,
										pbap_phone_book pbook,
										pbap_search_values srchAttr, uint16 maxList, uint16 listStart)
{
	MAKE_PBAPS_MESSAGE(PBAPS_GET_VCARD_LIST_START_IND);
	message->pbaps = (PBAPS*)state;
	message->status = status;
	
	message->order = order;
	message->srchVal = srchVal;
	message->size_srchVal = size_srchVal;
	message->pbook = pbook;
	message->srchAttr = srchAttr;
	message->maxList = maxList;
	message->listStart = listStart;
            
    MessageSend(state->theAppTask, PBAPS_GET_VCARD_LIST_START_IND, message);
}

void pbapsMsgSendGetvCardListCompleteInd(pbapsState *state, pbaps_lib_status status)
{
	MAKE_PBAPS_MESSAGE(PBAPS_GET_VCARD_LIST_COMPLETE_IND);
	message->pbaps = (PBAPS*)state;
	message->status = status;
            
    MessageSend(state->theAppTask, PBAPS_GET_VCARD_LIST_COMPLETE_IND, message);
}

void pbapsMsgSendGetvCardEntryStartInd(pbapsState *state, uint16 entry, pbap_format_values format, uint32 filter_lo, uint32 filter_hi)
{
	MAKE_PBAPS_MESSAGE(PBAPS_GET_VCARD_ENTRY_START_IND);
	message->pbaps = (PBAPS*)state;
	
	message->entry = entry;
	message->format = format;
	message->filter_lo = filter_lo;
	message->filter_hi = filter_hi;
            
    MessageSend(state->theAppTask, PBAPS_GET_VCARD_ENTRY_START_IND, message);
}

void pbapsMsgSendGetvCardEntryCompleteInd(pbapsState *state, pbaps_lib_status status)
{
	MAKE_PBAPS_MESSAGE(PBAPS_GET_VCARD_ENTRY_COMPLETE_IND);
	message->pbaps = (PBAPS*)state;
	message->status = status;
            
    MessageSend(state->theAppTask, PBAPS_GET_VCARD_ENTRY_COMPLETE_IND, message);
}

void pbapsMsgSendGetPhonebookStartInd(pbapsState *state, pbap_phone_repository repository, pbap_phone_book phonebook, 
									  	pbap_format_values format, uint32 filter_lo, uint32 filter_hi, 
										uint16 maxList, uint16 listStart)
{
	MAKE_PBAPS_MESSAGE(PBAPS_GET_PHONEBOOK_START_IND);
	message->pbaps = (PBAPS*)state;
	
	message->repository = repository;
	message->phonebook = phonebook;
	message->format = format;
	message->filter_lo = filter_lo;
	message->filter_hi = filter_hi;
	message->maxList = maxList;
	message->listStart = listStart;
            
    MessageSend(state->theAppTask, PBAPS_GET_PHONEBOOK_START_IND, message);
}

void pbapsMsgSendGetPhonebookCompleteInd(pbapsState *state, pbaps_lib_status status)
{
	MAKE_PBAPS_MESSAGE(PBAPS_GET_PHONEBOOK_COMPLETE_IND);
	message->pbaps = (PBAPS*)state;
	message->status = status;
            
    MessageSend(state->theAppTask, PBAPS_GET_PHONEBOOK_COMPLETE_IND, message);
}

void pbapsMsgSendGetCompleteInd(pbapsState *state, pbaps_running_command command, pbaps_lib_status status)
{
	switch (command)
	{
	case pbaps_com_PullvCardList:
		pbapsMsgSendGetvCardListCompleteInd(state, status);
		break;
	case pbaps_com_PullvCard:
		pbapsMsgSendGetvCardEntryCompleteInd(state, status);
		break;
	case pbaps_com_PullPhonebook:
		pbapsMsgSendGetPhonebookCompleteInd(state, status);
		break;
	default:
		break;
	}
}

