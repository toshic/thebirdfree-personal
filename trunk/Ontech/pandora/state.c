/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	State management functionality
	
FILE
	state.c
*/


/****************************************************************************
    Header files
*/

#include <print.h>
#include <panic.h>

#include "state.h"
#include "audioAdaptor_private.h"

#if DEBUG_PRINT_ENABLED

typedef struct
{
	const uint8 *str;
	uint16 len;
} stateDebugNames;

static const uint8 unInitStr[] = {'A','p','p','S','t','a','t','e','U','n','i','n','i','t','i','a','l','i','s','e','d'};
static const uint8 conInitStr[] = {'A','p','p','S','t','a','t','e','C','o','n','I','n','i','t','e','d'};
static const uint8 appIdleStr[] = {'A','p','p','S','t','a','t','e','A','p','p','I','d','l','e'};
static const uint8 appConnectingStr[] = {'A','p','p','S','t','a','t','e','C','o','n','n','e','c','t','i','n','g'};
static const uint8 appConnectedStr[] = {'A','p','p','S','t','a','t','e','C','o','n','n','e','c','t','e','d'};
static const uint8 appPullvCardListStr[] = {'A','p','p','S','t','a','t','e','P','u','l','l','v','C','a','r','d','L','i','s','t'};
static const uint8 appPullvCardEntryStr[] = {'A','p','p','S','t','a','t','e','P','u','l','l','v','C','a','r','d','E','n','t','r','y'};
static const uint8 appPullPhonebookStr[] = {'A','p','p','S','t','a','t','e','P','u','l','l','P','h','o','n','e','b','o','o','k'};
static const uint8 appPullPhonebookSizeStr[] = {'A','p','p','S','t','a','t','e','P','u','l','l','P','h','o','n','e','b','o','o','k','S','i','z','e'};

#define STATE_LIST_ENTRY(i) {(i), sizeof((i))}

static const stateDebugNames gStateDebugNamesList[] = 
							{ 
								STATE_LIST_ENTRY(unInitStr),
								STATE_LIST_ENTRY(conInitStr),
								STATE_LIST_ENTRY(appIdleStr),
								STATE_LIST_ENTRY(appConnectingStr),
								STATE_LIST_ENTRY(appConnectedStr),
								STATE_LIST_ENTRY(appPullvCardListStr),
								STATE_LIST_ENTRY(appPullvCardEntryStr),
								STATE_LIST_ENTRY(appPullPhonebookStr),
								STATE_LIST_ENTRY(appPullPhonebookSizeStr)
							};

static void statePrintString(pbap_states state)
{
	uint16 lLen = gStateDebugNamesList[state].len;
	uint16 lCnt;
	
	for (lCnt = 0; lCnt<lLen; lCnt++)
	{
		DEBUG(("%c",gStateDebugNamesList[state].str[lCnt]));
	}
}
							
static void stateLogStateChange(pbap_states pOldState, pbap_states pNewState)
{
	DEBUG(("  Set State - From 0x%d [",pOldState));
	statePrintString(pOldState);
	DEBUG(("] : To 0x%d [",pNewState));
	statePrintString(pNewState);
	DEBUG(("]\n"));
}

#else

#define stateLogStateChange(a,b)

#endif

void setState(pbap_states *pState, pbap_states pNewState)
{
	if ((pNewState >= PbapStateUninitialised) && (pNewState < PbapStateEndOfList))
	{
		stateLogStateChange(*pState, pNewState);
		*pState = pNewState;
	}
	else
	{
		DEBUG(("  Set State - From 0x%d : To 0x%d\n",*pState,pNewState));
		DEBUG(("    INVALID STATE\n"));
		Panic();
	}
}
