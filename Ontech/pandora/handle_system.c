/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	Implementation for handling system messages and functionality
	
FILE
	handle_system.c
	
*/

/****************************************************************************
    Header files
*/

#include <print.h>
#include <vm.h>

#include "handle_system.h"
#include "audioAdaptor_private.h"

/* Message Handler Prototypes */
static void handleMoreData(Source pSrc);
static void handleMoreSpace(Sink pSink);


void handleSystemMessages(MessageId pId, Message pMessage)
{
	switch (pId)
	{
	case MESSAGE_MORE_DATA:
			handleMoreData(((MessageMoreData*)pMessage)->source);
			break;
	case MESSAGE_MORE_SPACE:
			handleMoreSpace(((MessageMoreSpace*)pMessage)->sink);
			break;
		default:
		DEBUG(("Unhandled System Message\n"));
		break;
	}
}

/* Message Handlers */

static void handleMoreData(Source pSrc)
{
	DEBUG(("MESSAGE_MORE_DATA\n"));
}

static void handleMoreSpace(Sink pSink)
{
	DEBUG(("MESSAGE_MORE_SPACE\n"));
}

