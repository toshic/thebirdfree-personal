/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	Implementation for for handling folder based functionality
	
FILE
	folder.c
	
*/

/****************************************************************************
    Header files
*/

#include <print.h>
#include <pbap_common.h>
#include <panic.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "folder.h"
#include "pb_access.h"
#include "buffer.h"
#include "audioAdaptor_private.h"

/* Listing Format constants */

static const uint8 gVCardListHeader[] = {'<','?','x','m','l',' ','v','e','r','s','i','o','n','=','"','1','.','0','"','?','>','\n',
										'<','!','D','O','C','T','Y','P','E',' ','v','c','a','r','d','-','l','i','s','t','i','n','g',' ','S','Y','S','T','E','M',' ','"','v','c','a','r','d','-','l','i','s','t','i','n','g','.','d','t','d','"','>','\n',
										'<','v','C','a','r','d','-','l','i','s','t','i','n','g',' ','v','e','r','s','i','o','n',' ','"','1','.','0','"','>','\n'};
static const uint8 gVCardListFooter[] = {'<','/','v','C','a','r','d','-','l','i','s','t','i','n','g','>','\n'};
static const uint8 gVCardListEntryStart[] = {'<','c','a','r','d',' ','h','a','n','d','l','e',' ','=',' ','"'};
static const uint8 gVCardListEntryMiddle[] = {'.','v','c','f','"',' ','n','a','m','e',' ','=',' ','"'};
static const uint8 gVCardListEntryEnd[] = {'"','/','>','\n'};

/* Local Functions Definitions */

static bool folderFillBuffer(void);
static bool folderAddToBuffer(uint16 pIndex, const uint8 *pNamePtr, uint16 pSize);

/* Interface Functions */

/*
	Initialise the folder sub-system
*/
void folderInit(void)
{
	pbap_phone_book lBook;
	
	DEBUG(("folderInit\n"));
	
	/* Indicate root as current folder */
	the_app->folderData.current = pbap_b_unknown;
	
	/* Find Supported Phonebooks */
	for (lBook = pbap_pb; lBook < pbap_b_unknown; lBook++)
	{
		DEBUG(("    Checking folder %d : ", lBook));
		
		if (pbaPhoneBookSupported(lBook))
		{ /* Phonebook supported */
			DEBUG(("Supported\n"));
			the_app->folderData.supBooks |= (1<<lBook);
		}
		else
		{ /* Phonebook unsupported */
			DEBUG(("Unsupported\n"));
		}
	}
}

/*
	Set current folder to a child folder
*/
pbaps_set_phonebook_result folderSetChild(pbap_phone_book pBook)
{
	pbaps_set_phonebook_result lRes = pbaps_spb_ok;
	
	switch (the_app->folderData.current)
	{
		case pbap_b_unknown: /* Root Folder */
			if (pBook != pbap_telecom)
			{
				DEBUG(("   Change to telecom when not at root, rejecting\n"));
				lRes = pbaps_spb_no_phonebook;
			}
			break; 
		case pbap_telecom:
			if ((pBook >= pbap_pb) && (pBook <= pbap_cch))
			{ /* Check to ensure required phonebook is supported */
				if (!(the_app->folderData.supBooks | (1<<pBook)))
				{
					DEBUG(("   Requested phonebook not supported, rejecting\n"));
					lRes = pbaps_spb_no_phonebook;
				}
			}
			else
			{
				DEBUG(("    Change to a phonebook when not in telecom, rejecting\n"));
				lRes = pbaps_spb_no_phonebook;
			}
			break;
		case pbap_pb:
		case pbap_ich: 
		case pbap_och: 
		case pbap_mch: 
		case pbap_cch:
		default:
			DEBUG(("    Already in a phonebook, no child folders. Rejecting\n"));
			lRes = pbaps_spb_no_phonebook;
			break;
	
	}
	if (lRes == pbaps_spb_ok)
	{
		the_app->folderData.current = pBook;
	}
	
	return lRes;
}

/*
	Set current folder to parent folder
*/
pbaps_set_phonebook_result folderSetParent(void)
{
	pbaps_set_phonebook_result lRes = pbaps_spb_ok;
	
	switch (the_app->folderData.current)
	{
		case pbap_telecom:
			DEBUG(("    Changing to root folder\n"));
			the_app->folderData.current = pbap_b_unknown; /* Root Folder */
			break;
		case pbap_pb:
		case pbap_ich: 
		case pbap_och: 
		case pbap_mch: 
		case pbap_cch:
			DEBUG(("    Changing to telecom folder\n"));
			the_app->folderData.current = pbap_telecom;
			break;
		case pbap_b_unknown: /* Root Folder */
		default:
			DEBUG(("    Already root, no parent folder. Rejecting\n"));
			lRes = pbaps_spb_no_phonebook;
			break;
	
	}
	
	return lRes;
}

/*
	Set current folder to root folder
*/
pbaps_set_phonebook_result folderSetRoot(void)
{
	the_app->folderData.current = pbap_b_unknown; /* Root Folder */
	return pbaps_spb_ok;
}

/*
	Return the current folder
*/
pbap_phone_book folderCurrentFolder(void)
{
	return the_app->folderData.current;
}

/*
	Get first vCard list buffer
*/
bool folderGetFirstListBuffer(uint16 pStartPos)
{
	bool lFinished = FALSE;
	uint16 lToCopy;
	
	DEBUG(("folderGetFirstListBuffer\n"));
	/* Setup buffer */
	if (!allocBuffer(PBAPS_MIN_BUFFER_SIZE))
	{
		DEBUG(("    Cannot Allocate Buffer\n"));
		Panic();
	}
	/* Open phonebook */
	if (!pbaOpenPhonebook(the_app->folderData.current))
	{
		DEBUG(("    Cannot open phonebook\n"));
		Panic();
	}
	
	/* Copy preamble to buffer */
	lToCopy = (the_app->buffer.sizeBuffer <= sizeof(gVCardListHeader)) ? the_app->buffer.sizeBuffer : sizeof(gVCardListHeader);
	memcpy(the_app->buffer.buffer, gVCardListHeader, lToCopy);
	the_app->buffer.freeSpace -= lToCopy;
	the_app->buffer.nextPos += lToCopy;
	the_app->buffer.used += lToCopy;
	the_app->folderData.listLeft = sizeof(gVCardListHeader) - lToCopy;
	
	/* Ensure we start with a search */
	the_app->folderData.sentCurrent = TRUE;
	
	if (pStartPos>0)
	{
		pbaGotoEntry(pStartPos);
	}
	
	if (the_app->buffer.freeSpace >= 0)
	{
		lFinished = folderFillBuffer();
	}
	
	return lFinished;
}

/*
	Get next vCard list buffer
*/
bool folderGetNextListBuffer(void)
{
	bool lFinished = TRUE;

	resetBuffer();
	
	if (the_app->folderData.listLeft > 0)
	{
		uint16 lToCopy;
		uint16 lStart = sizeof(gVCardListHeader) - the_app->folderData.listLeft;
		lToCopy = (the_app->buffer.sizeBuffer <= the_app->folderData.listLeft) ? the_app->buffer.sizeBuffer : the_app->folderData.listLeft;
		
		memcpy(the_app->buffer.buffer, &gVCardListHeader[lStart], lToCopy);
		the_app->buffer.freeSpace -= lToCopy;
		the_app->buffer.nextPos += lToCopy;
		the_app->buffer.used += lToCopy;
		the_app->folderData.listLeft = the_app->folderData.listLeft - lToCopy;
	}
	
	if (the_app->buffer.freeSpace >= 0)
	{
		lFinished = folderFillBuffer();
	}
	
	return lFinished;
}

/*
	Clean up from downloading the vCard list
*/
void folderCleanupListBuffer(void)
{
	freeBuffer();
	pbaClosePhonebook();
}

/* Local Function Implementation */

static bool folderFillBuffer()
{
	bool lComplete = FALSE, lSent = TRUE, lEntry = TRUE;
	pbaSearchResult *lSrchRes;
	
	
	DEBUG(("folderFillBuffer\n"));
	
	lSrchRes = PanicUnlessMalloc(sizeof(pbaSearchResult));
	
	if (!the_app->folderData.sentCurrent)
	{
		DEBUG(("    Sending Current\n"));
		/* Get current entry */
		pbaGetCurrentEntry(lSrchRes);
		/* Add to buffer */
		lSent = folderAddToBuffer(lSrchRes->index,  lSrchRes->namePtr, lSrchRes->nameSize);
		if (!lSent)
		{
			DEBUG(("    Unable to add single entry\n"));
			Panic();
		}
		else
		{
			the_app->srchData.count++;
			the_app->srchData.maxList--;
			if (the_app->srchData.maxList == 0)
			{
				lEntry = FALSE;
			}
		}
	}
	
	if (the_app->folderData.sendFooter)
	{
		DEBUG(("    SENDING FOOTER\n"));
		memcpy(the_app->buffer.buffer, gVCardListFooter, sizeof(gVCardListFooter));
		the_app->buffer.freeSpace -= sizeof(gVCardListFooter);
		the_app->buffer.nextPos += sizeof(gVCardListFooter);
		the_app->buffer.used += sizeof(gVCardListFooter);
		lComplete = TRUE;
		the_app->folderData.sendFooter = FALSE;
	}
	else
	{	
		lSent = TRUE;
		/* Get next entry */
		if (lEntry)
		{
			lEntry = pbaFindNextEntry(lSrchRes);
		}
		while (lEntry && lSent)
		{
			lSent = folderAddToBuffer(lSrchRes->index,  lSrchRes->namePtr, lSrchRes->nameSize);
			if (lSent)
			{
				the_app->srchData.count++;
				the_app->srchData.maxList--;
				if (the_app->srchData.maxList == 0)
				{
					lEntry = FALSE;
				}
				else
				{
					/* Get next Entry */
					lEntry = pbaFindNextEntry(lSrchRes);
				}
			}
		}
		if (!lEntry)
		{ /* No more entries to read */
			DEBUG(("    Closing phonebook\n"));
			/* Close phonebook */
			pbaClosePhonebook();
			
			if (sizeof(gVCardListFooter) <= the_app->buffer.freeSpace)
			{ /* Add footer to buffer */
				memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], gVCardListFooter, sizeof(gVCardListFooter));
				the_app->buffer.freeSpace -= sizeof(gVCardListFooter);
				the_app->buffer.nextPos += sizeof(gVCardListFooter);
				the_app->buffer.used += sizeof(gVCardListFooter);
				lComplete = TRUE;
				the_app->folderData.sendFooter = FALSE;
			}
			else
			{ /* remember finished and not sent footer */
				the_app->folderData.sendFooter = TRUE;
			}
		}
	}
	
	free(lSrchRes);
	
	return lComplete;
}

static bool folderAddToBuffer(uint16 pIndex,  const uint8 *pNamePtr, uint16 pSize)
{
	bool lFit = TRUE;
	uint16 lSpaceNeeded = sizeof(gVCardListEntryStart) + sizeof(gVCardListEntryMiddle) + sizeof(gVCardListEntryEnd) + pSize;
	char lIndex[10];
	uint16 lSizeIndex;
	
	/* Calculate the size */
	sprintf(lIndex, "%X", pIndex);
	lSizeIndex = strlen(lIndex);
	lSpaceNeeded += lSizeIndex;
	
	if (lSpaceNeeded <= the_app->buffer.freeSpace)
	{
		/* Copy Header */
		memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], gVCardListEntryStart, sizeof(gVCardListEntryStart));
		the_app->buffer.nextPos += sizeof(gVCardListEntryStart);
		/* Create Index */
		memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], lIndex, lSizeIndex);
		the_app->buffer.nextPos += lSizeIndex;
		/* Copy Middle */
		memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], gVCardListEntryMiddle, sizeof(gVCardListEntryMiddle));
		the_app->buffer.nextPos += sizeof(gVCardListEntryMiddle);
		/* Copy Name */
		if (pSize > 0)
		{
			memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], pNamePtr, pSize);
			the_app->buffer.nextPos += pSize;
		}
		/* Copy End */
		memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], gVCardListEntryEnd, sizeof(gVCardListEntryEnd));
		the_app->buffer.nextPos += sizeof(gVCardListEntryEnd);

		/* Update counters */
		the_app->buffer.freeSpace -= lSpaceNeeded;
		the_app->buffer.used += lSpaceNeeded;
		
		the_app->folderData.sentCurrent = TRUE;
	}
	else
	{
		lFit = FALSE;
		the_app->folderData.sentCurrent = FALSE;
	}
		
	
	return lFit;
}
