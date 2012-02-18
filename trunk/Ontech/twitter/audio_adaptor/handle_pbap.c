/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	Implementation for handling PBAP library messages and functionality
	
FILE
	handle_pbap.c
	
*/

/****************************************************************************
    Header files
*/

#include <connection.h>
#include <pbaps.h>
#include <print.h>
#include <panic.h>
#include <pbap_common.h>
#include <stdlib.h>

#include "handle_pbap.h"
#include "folder.h"
#include "vcard_gen.h"
#include "pb_access.h"
#include "audioAdaptor_private.h"
#include "SppServer.h"

/* Message Handler Prototypes */
static void handlePbapInitCfm(PBAPS_INIT_CFM_T *pMsg);
static void handlePbapConnectInd(PBAPS_CONNECT_IND_T *pMsg);
static void handlePbapConnectCfm(PBAPS_CONNECT_CFM_T *pMsg);
static void handlePbapDisconnectInd(PBAPS_DISCONNECT_IND_T *pMsg);

static void handlePbapSetPhoneBook(PBAPS_SET_PB_BOOK_IND_T *pMsg);
static void handlePbapSetParent(PBAPS_SET_PB_PARENT_IND_T *pMsg);
static void handlePbapSetRoot(PBAPS_SET_PB_ROOT_IND_T *pMsg);
static void handlePbapSetRepository(PBAPS_SET_PB_REPOSITORY_IND_T *pMsg);

static void handlePbapGetVCardListStartInd(PBAPS_GET_VCARD_LIST_START_IND_T *pMsg);
static void handlePbapGetVCardListNextInd(PBAPS_GET_VCARD_LIST_NEXT_IND_T *pMsg);
static void handlePbapGetVCardListCompleteInd(PBAPS_GET_VCARD_LIST_COMPLETE_IND_T *pMsg);

static void handlePbapGetVCardEntryStartInd(PBAPS_GET_VCARD_ENTRY_START_IND_T *pMsg);
static void handlePbapGetVCardEntryNextInd(PBAPS_GET_VCARD_ENTRY_NEXT_IND_T *pMsg);
static void handlePbapGetVCardEntryCompleteInd(PBAPS_GET_VCARD_ENTRY_COMPLETE_IND_T *pMsg);

static void handlePbapGetPhonebookStartInd(PBAPS_GET_PHONEBOOK_START_IND_T *pMsg);
static void handlePbapGetPhonebookNextInd(PBAPS_GET_PHONEBOOK_NEXT_IND_T *pMsg);
static void handlePbapGetPhonebookCompleteInd(PBAPS_GET_PHONEBOOK_COMPLETE_IND_T *pMsg);

/* Interface Functions */

/*
  Initialise the PBAP System
*/
void initPbap(void)
{
	DEBUG_PBAP(("####initPbap\n"));
	/* Initialise the PBAP library */
	PbapsInit(&the_app->task, 1, PBAP_REP_LOCAL);
}

void handlePbapMessages(MessageId pId, Message pMessage)
{
	switch (pId)
	{
	case PBAPS_INIT_CFM:
		handlePbapInitCfm((PBAPS_INIT_CFM_T *)pMessage);
		break;
	case PBAPS_CONNECT_IND:
		handlePbapConnectInd((PBAPS_CONNECT_IND_T *)pMessage);
		break;
	case PBAPS_CONNECT_CFM:
		handlePbapConnectCfm((PBAPS_CONNECT_CFM_T *)pMessage);
		break;
	case PBAPS_DISCONNECT_IND:
		handlePbapDisconnectInd((PBAPS_DISCONNECT_IND_T *)pMessage);
		break;
		
	case PBAPS_SET_PB_ROOT_IND:
		handlePbapSetRoot((PBAPS_SET_PB_ROOT_IND_T *)pMessage);
		break;
	case PBAPS_SET_PB_REPOSITORY_IND:
		handlePbapSetRepository((PBAPS_SET_PB_REPOSITORY_IND_T *)pMessage);
		break;
	case PBAPS_SET_PB_BOOK_IND:
		handlePbapSetPhoneBook((PBAPS_SET_PB_BOOK_IND_T *)pMessage);
		break;
	case PBAPS_SET_PB_PARENT_IND:
		handlePbapSetParent((PBAPS_SET_PB_PARENT_IND_T *)pMessage);
		break;

	case PBAPS_GET_VCARD_LIST_START_IND:
		handlePbapGetVCardListStartInd((PBAPS_GET_VCARD_LIST_START_IND_T *)pMessage);
		break;
	case PBAPS_GET_VCARD_LIST_NEXT_IND:
		handlePbapGetVCardListNextInd((PBAPS_GET_VCARD_LIST_NEXT_IND_T *)pMessage);
		break;
	case PBAPS_GET_VCARD_LIST_COMPLETE_IND:
		handlePbapGetVCardListCompleteInd((PBAPS_GET_VCARD_LIST_COMPLETE_IND_T *)pMessage);
		break;

	case PBAPS_GET_VCARD_ENTRY_START_IND:
		handlePbapGetVCardEntryStartInd((PBAPS_GET_VCARD_ENTRY_START_IND_T *)pMessage);
		break;
	case PBAPS_GET_VCARD_ENTRY_NEXT_IND:
		handlePbapGetVCardEntryNextInd((PBAPS_GET_VCARD_ENTRY_NEXT_IND_T *)pMessage);
		break;
	case PBAPS_GET_VCARD_ENTRY_COMPLETE_IND:
		handlePbapGetVCardEntryCompleteInd((PBAPS_GET_VCARD_ENTRY_COMPLETE_IND_T *)pMessage);
		break;

	case PBAPS_GET_PHONEBOOK_START_IND:
		handlePbapGetPhonebookStartInd((PBAPS_GET_PHONEBOOK_START_IND_T *)pMessage);
		break;
	case PBAPS_GET_PHONEBOOK_NEXT_IND:
		handlePbapGetPhonebookNextInd((PBAPS_GET_PHONEBOOK_NEXT_IND_T *)pMessage);
		break;
	case PBAPS_GET_PHONEBOOK_COMPLETE_IND:
		handlePbapGetPhonebookCompleteInd((PBAPS_GET_PHONEBOOK_COMPLETE_IND_T *)pMessage);
		break;

	default:
		DEBUG_PBAP(("Unhandled PBAP Message\n"));
		break;
	}
}


/* Message Handlers */

static void handlePbapInitCfm(PBAPS_INIT_CFM_T *pMsg)
{
	DEBUG_PBAP(("PBAPS_INIT_CFM : "));

	if (pMsg->status == pbaps_success)
	{
		DEBUG_PBAP(("success\n"));
		the_app->pbapData.pbaps = pMsg->pbaps;
		
		/* Initialise folder sub-system */
		folderInit();
		
		setState(&the_app->appState, PbapStateAppIdle);

		/* Start SPP Server here */
		sppDevInit();
	}
	else
	{ /* Failed to initialise PBAPS */
		DEBUG_PBAP(("failure\n"));
		DEBUG_PBAP(("    Status : %d\n", pMsg->status));	
		Panic();
	}
}

static void handlePbapConnectInd(PBAPS_CONNECT_IND_T *pMsg)
{
	bool lAuth = FALSE;
	
	SendEvent(EVT_PBAP_CONNECT_IND,0);

	DEBUG_PBAP(("PBAPS_CONNECT_IND\n"));
	DEBUG_PBAP(("    BD Addr : 0x%X 0x%X 0x%X,%X\n", pMsg->bd_addr.nap, pMsg->bd_addr.uap, (uint16)(pMsg->bd_addr.lap>>16), (uint16)(pMsg->bd_addr.lap&0xFFFF)));
	if (the_app->appState <= PbapStateConnecting)
	{ /* Application idle, we can accept a connection */
		DEBUG_PBAP(("    Accepting connection\n"));
		lAuth = TRUE;
	}
	else
	{ /* Application busy, reject the connection */
		DEBUG_PBAP(("    Rejecting Connection [State = %d]\n", the_app->appState));
	}
	PbapsConnectResponse(the_app->pbapData.pbaps, lAuth, PBAP_DEF_PACKET);
}

static void handlePbapConnectCfm(PBAPS_CONNECT_CFM_T *pMsg)
{
	SendEvent(EVT_PBAP_CONNECT_CFM,pMsg->status);

	DEBUG_PBAP(("PBAPS_CONNECT_CFM\n"));
	
	if (pMsg->status == pbaps_success)
	{
		DEBUG_PBAP(("    Connection Successfull\n"));
		DEBUG_PBAP(("    Max Packet Size = %d bytes\n",pMsg->maxPacketLen));
		setState(&the_app->appState, PbapStateConnected);
		the_app->pbapData.pktSize = pMsg->maxPacketLen;
	}
	else
	{
		DEBUG_PBAP(("    Connection Failed\n"));
		setState(&the_app->appState, PbapStateAppIdle);
	}
}

static void handlePbapDisconnectInd(PBAPS_DISCONNECT_IND_T *pMsg)
{
	SendEvent(EVT_PBAP_DISCONNECT_IND,0);

	DEBUG_PBAP(("PBAPS_DISCONNECT_IND\n"));
	setState(&the_app->appState, PbapStateAppIdle);
}

static void handlePbapSetPhoneBook(PBAPS_SET_PB_BOOK_IND_T *pMsg)
{
	pbaps_set_phonebook_result lRes;
	
	SendEvent(EVT_PBAP_SET_PHONEBOOK_REQ,pMsg->book);

	DEBUG_PBAP(("PBAPS_SET_PB_BOOK_IND\n"));
	DEBUG_PBAP(("    Phonebook %d\n",pMsg->book));
	
	if (the_app->appState == PbapStateConnected)
	{
		lRes = folderSetChild(pMsg->book);
		PbapsSetPhonebookBookResponse(the_app->pbapData.pbaps, lRes);
	}
	else
	{
		DEBUG_PBAP(("    NOT Idle - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapSetParent(PBAPS_SET_PB_PARENT_IND_T *pMsg)
{
	pbaps_set_phonebook_result lRes;
	
	DEBUG_PBAP(("PBAPS_SET_PB_PARENT_IND\n"));
	
	if (the_app->appState == PbapStateConnected)
	{
		lRes = folderSetParent();
		PbapsSetPhonebookParentResponse(the_app->pbapData.pbaps, lRes);
	}
	else
	{
		DEBUG_PBAP(("    NOT Idle - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapSetRoot(PBAPS_SET_PB_ROOT_IND_T *pMsg)
{
	pbaps_set_phonebook_result lRes;
	
	DEBUG_PBAP(("PBAPS_SET_PB_ROOT_IND\n"));
	
	if (the_app->appState == PbapStateConnected)
	{
		lRes = folderSetRoot();
		PbapsSetPhonebookRootResponse(the_app->pbapData.pbaps, lRes);
	}
	else
	{
		DEBUG_PBAP(("    NOT Idle - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapSetRepository(PBAPS_SET_PB_REPOSITORY_IND_T *pMsg)
{
	pbaps_set_phonebook_result lRes;
	
	DEBUG_PBAP(("PBAPS_SET_PB_REPOSITORY_IND\n"));
	DEBUG_PBAP(("    Repository %d\n",pMsg->repository));
	
	if (the_app->appState == PbapStateConnected)
	{
		if (pMsg->repository == pbap_sim1)
		{
			DEBUG_PBAP(("    No Such Repository\n"));
			lRes = pbaps_spb_no_repository;
		}
		else
		{
			lRes = pbaps_spb_ok; 
		}
		PbapsSetPhonebookRepositoryResponse(the_app->pbapData.pbaps, lRes);
	}
	else
	{
		DEBUG_PBAP(("    NOT Idle - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapGetVCardListStartInd(PBAPS_GET_VCARD_LIST_START_IND_T *pMsg)
{
	DEBUG_PBAP(("PBAPS_GET_VCARD_LIST_START_IND\n"));

	if (the_app->appState == PbapStateConnected)
	{
		bool lComplete = FALSE;
		setState(&the_app->appState, PbapStatePullvCardList);
		
		the_app->srchData.srchAttr = pMsg->srchAttr;
		the_app->srchData.srchVal = pMsg->srchVal;
		the_app->srchData.maxList	= pMsg->maxList;
		
		lComplete = folderGetFirstListBuffer(pMsg->listStart);
		PbapsGetvCardListFirstPacket(the_app->pbapData.pbaps, pbaps_get_ok, 0, the_app->buffer.buffer, the_app->buffer.used, lComplete);
	}
	else
	{
		DEBUG_PBAP(("    NOT Idle - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapGetVCardListNextInd(PBAPS_GET_VCARD_LIST_NEXT_IND_T *pMsg)
{
	DEBUG_PBAP(("PBAPS_GET_VCARD_LIST_NEXT_IND\n"));

	if (the_app->appState == PbapStatePullvCardList)
	{
		bool lComplete;
		
		lComplete = folderGetNextListBuffer();
		PbapsGetvCardListNextPacket(the_app->pbapData.pbaps, the_app->buffer.used,  the_app->buffer.buffer, lComplete);
	}
	else
	{
		DEBUG_PBAP(("    NOT Pulling vCard List - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapGetVCardListCompleteInd(PBAPS_GET_VCARD_LIST_COMPLETE_IND_T *pMsg)
{
	DEBUG_PBAP(("PBAPS_GET_VCARD_LIST_COMPLETE_IND\n"));

	if (the_app->appState == PbapStatePullvCardList)
	{
		setState(&the_app->appState, PbapStateConnected);
		
		free(the_app->srchData.srchVal);
		the_app->srchData.srchAttr = 0;
		the_app->srchData.srchVal = NULL;
		the_app->srchData.maxList	= 0;
		
		folderCleanupListBuffer();
	}
	else
	{
		DEBUG_PBAP(("    NOT Pulling vCard List - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapGetVCardEntryStartInd(PBAPS_GET_VCARD_ENTRY_START_IND_T *pMsg)
{
	DEBUG_PBAP(("PBAPS_GET_VCARD_ENTRY_START_IND\n"));
	DEBUG_PBAP(("    Entry : %d\n",pMsg->entry));
	DEBUG_PBAP(("    Format : %d\n",pMsg->format));

	if (the_app->appState == PbapStateConnected)
	{
		bool lComplete = FALSE;
		setState(&the_app->appState, PbapStatePullvCardEntry);
		
		
		lComplete = vcgGetFirstEntryBuffer(pMsg->entry, pMsg->format);
		PbapsGetvCardEntryFirstPacket(the_app->pbapData.pbaps, pbaps_get_ok, 0, the_app->buffer.buffer, the_app->buffer.used, lComplete);
	}
	else
	{
		DEBUG_PBAP(("    NOT Idle - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapGetVCardEntryNextInd(PBAPS_GET_VCARD_ENTRY_NEXT_IND_T *pMsg)
{
	DEBUG_PBAP(("PBAPS_GET_VCARD_ENTRY_NEXT_IND\n"));
	if (the_app->appState == PbapStatePullvCardEntry)
	{
		bool lComplete;
		
		lComplete = vcgGetNextEntryBuffer();
		PbapsGetvCardEntryNextPacket(the_app->pbapData.pbaps, the_app->buffer.used,  the_app->buffer.buffer, lComplete);
	}
	else
	{
		DEBUG_PBAP(("    NOT Pulling vCard Entry - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapGetVCardEntryCompleteInd(PBAPS_GET_VCARD_ENTRY_COMPLETE_IND_T *pMsg)
{
	DEBUG_PBAP(("PBAPS_GET_VCARD_ENTRY_COMPLETE_IND\n"));
	if (the_app->appState == PbapStatePullvCardEntry)
	{
		setState(&the_app->appState, PbapStateConnected);
		
		vcgCleanupListBuffer();
	}
	else
	{
		DEBUG_PBAP(("    NOT Pulling vCard Entry - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapGetPhonebookStartInd(PBAPS_GET_PHONEBOOK_START_IND_T *pMsg)
{
	if(pMsg->maxList)
		SendEvent(EVT_PBAP_PULL_PHONEBOOK_START,pMsg->phonebook);

	DEBUG_PBAP(("PBAPS_GET_PHONEBOOK_START_IND\n"));
	DEBUG_PBAP(("    Repository : %d\n",pMsg->repository));
	DEBUG_PBAP(("    Phonebook : %d\n",pMsg->phonebook));
	DEBUG_PBAP(("    Format : %d\n",pMsg->format));
	DEBUG_PBAP(("    Max List : %d\n",pMsg->maxList));
	DEBUG_PBAP(("    List Start : %d\n",pMsg->listStart));

	if (the_app->appState == PbapStateConnected)
	{
		setState(&the_app->appState, PbapStatePullPhonebook);

		the_app->srchData.count = 0;

		if ((pMsg->repository != pbap_current) && (pMsg->repository != pbap_local))
		{ /* Request for an invalid repository */
			PbapsGetPhonebookFirstPacket(the_app->pbapData.pbaps, pbaps_get_eol, 0, NULL, 0, TRUE);
		}
		else if (pMsg->maxList == 0)
		{ /* Client interested in how big the phonebook is */
			uint16 lNumEntries;
			
			setState(&the_app->appState, PbapStatePullPhonebookSize);
			/* Get number of entries */
			lNumEntries = pbaGetNumberElements(pMsg->phonebook);
			
			PbapsGetPhonebookFirstPacketParams(the_app->pbapData.pbaps, pbaps_get_ok, 0, lNumEntries, 0);
		}
		else if (!pbaPhoneBookSupported(pMsg->phonebook))
		{ /* Request for an unsupported phonebook */
			PbapsGetPhonebookFirstPacket(the_app->pbapData.pbaps, pbaps_get_eol, 0, NULL, 0, TRUE);
		}
		else
		{
			bool lComplete = FALSE;
		
			lComplete = vcgGetFirstPhonebookBuffer(pMsg->phonebook, pMsg->listStart, pMsg->format, pMsg->maxList);
			PbapsGetPhonebookFirstPacket(the_app->pbapData.pbaps, pbaps_get_ok, 0, the_app->buffer.buffer, the_app->buffer.used, lComplete);
		}
	}
	else
	{
		DEBUG_PBAP(("    NOT Idle - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapGetPhonebookNextInd(PBAPS_GET_PHONEBOOK_NEXT_IND_T *pMsg)
{
	DEBUG_PBAP(("PBAPS_GET_PHONEBOOK_NEXT_IND\n"));
	if (the_app->appState == PbapStatePullPhonebook)
	{
		bool lComplete;
		
		lComplete = vcgGetNextPhonebookBuffer();
		PbapsGetPhonebookNextPacket(the_app->pbapData.pbaps, the_app->buffer.used,  the_app->buffer.buffer, lComplete);
	}
	else if (the_app->appState == PbapStatePullPhonebookSize)
	{
		PbapsGetPhonebookNextPacket(the_app->pbapData.pbaps, 0, NULL, TRUE);
	}
	else
	{
		DEBUG_PBAP(("    NOT Pulling Phonebook - Should not be possible\n"));
		Panic();
	}
}

static void handlePbapGetPhonebookCompleteInd(PBAPS_GET_PHONEBOOK_COMPLETE_IND_T *pMsg)
{
	if(the_app->appState == PbapStatePullPhonebook)
		SendEvent(EVT_PBAP_PULL_PHONEBOOK_COMPLETE,the_app->srchData.count);/*pMsg->status);*/

	DEBUG_PBAP(("PBAPS_GET_PHONEBOOK_COMPLETE_IND\n"));
	if ((the_app->appState == PbapStatePullPhonebook) || (the_app->appState == PbapStatePullPhonebookSize))
	{
		setState(&the_app->appState, PbapStateConnected);
		
		vcgCleanupListBuffer();
	}
	else
	{
		DEBUG_PBAP(("    NOT Pulling Phonebook - Should not be possible\n"));
/*		Panic();*/
	}
}
