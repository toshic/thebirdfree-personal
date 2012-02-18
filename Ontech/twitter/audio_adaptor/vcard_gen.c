/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	Implementation for handling vCard generation functionality
	
FILE
	vcard_gen.c
	
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

#include "audioAdaptor_private.h"
#include "vcard_gen.h"
#include "pb_access.h"
#include "buffer.h"
#include "folder.h"

/* Listing Format constants */

static const uint8 gVCardEntryHeader[] = {'B','E','G','I','N',':','V','C','A','R','D','\r','\n','V','E','R','S','I','O','N',':'};
static const uint8 gVCardEntryVersion21[] = {'2','.','1','\r','\n'};
static const uint8 gVCardEntryVersion30[] = {'3','.','0','\r','\n'};
static const uint8 gVCardEntryName21[] = {'N',':'};
static const uint8 gVCardEntryName30[] = {'F','N',':'};
static const uint8 gVCardEntryTel[] = {'T','E','L',':'};
static const uint8 gVCardEntryTelMobile[] = {'T','E','L',';','M','O','B','I','L','E',':'};
static const uint8 gVCardEntryTelBusiness[] = {'T','E','L',';','W','O','R','K',':'};
static const uint8 gVCardEntryFooter[] = {'E','N','D',':','V','C','A','R','D','\r','\n'};
static const uint8 gVCardEntryNewline[] = {'\r','\n'};


/* Local Functions */

static void vcgResetState(void);
static bool vcgFillBuffer(pbaSearchResult *pEntry);
static bool vcgAddElement(const uint8 *pElement, uint16 pSizeElement, const uint8 *pHeader, uint16 pSizeHeader);

/* Interface Functions */

/*
	Start downloading a vCard entry
*/
bool vcgGetFirstEntryBuffer(uint16 pEntry, pbap_format_values pFormat)
{
	bool lComplete = TRUE;
	pbap_phone_book lCurrent = folderCurrentFolder();
	
	if (!allocBuffer(PBAPS_MIN_BUFFER_SIZE))
	{
		DEBUG_VGEN(("    Cannot Allocate Buffer\n"));
		Panic();
	}

	if (pbaOpenPhonebook(lCurrent))
	{
		if (pbaGotoEntry(pEntry))
		{
			pbaSearchResult lEntry;
			pbaGetCurrentEntry(&lEntry);
			vcgResetState();
			the_app->vCardData.format = pFormat;
			lComplete = vcgFillBuffer(&lEntry);
		}
	}
	
	if (lComplete)
		pbaClosePhonebook();
		
	return lComplete;
}

/*
	Download the next buffer of the vCard Entry
*/
bool vcgGetNextEntryBuffer(void)
{
	bool lComplete = TRUE;
	pbaSearchResult lEntry;

	resetBuffer();	
	pbaGetCurrentEntry(&lEntry);
	lComplete = vcgFillBuffer(&lEntry);
	
	if (lComplete)
		pbaClosePhonebook();
		
	return lComplete;
}

/*
	Clean up from downloading the vCard entry
*/
void vcgCleanupListBuffer(void)
{
	freeBuffer();
	pbaClosePhonebook();
}

/*
	Start downloading a complete phonebook
*/
bool vcgGetFirstPhonebookBuffer(pbap_phone_book pBook, uint16 pListStart, pbap_format_values pFormat, uint16 pMaxList)
{
	bool lComplete = TRUE;
	
	DEBUG_VGEN(("vcgGetFirstPhonebookBuffer\n"));
	
	if (!allocBuffer(PBAPS_MIN_BUFFER_SIZE))
	{
		DEBUG_VGEN(("    Cannot Allocate Buffer\n"));
		Panic();
	}

	if (pbaOpenPhonebook(pBook))
	{
		if (pbaGotoEntry(pListStart))
		{
			bool lFoundEntry = TRUE;
			bool lFilled = TRUE;
			pbaSearchResult lEntry;
			the_app->vCardData.format = pFormat;
			the_app->vCardData.maxList = pMaxList;
			
			pbaGetCurrentEntry(&lEntry);
			
			while (lFoundEntry && lFilled && the_app->vCardData.maxList > 0)
			{
				vcgResetState();
				lFilled = vcgFillBuffer(&lEntry);
				if (lFilled)
				{
					lFoundEntry = pbaGotoNextEntry(&lEntry);
					the_app->vCardData.maxList--;
					the_app->srchData.count++;
				}
			}
			
			lComplete = (!lFoundEntry || (the_app->vCardData.maxList == 0));
		}
	}
		
	return lComplete;
}

/*
	Download the next buffer of a complete phonebook
*/
bool vcgGetNextPhonebookBuffer(void)
{
	bool lFoundEntry = TRUE;
	bool lFilled = TRUE;
	pbaSearchResult lEntry;
	
	DEBUG_VGEN(("vcgGetNextPhonebookBuffer\n"));

	resetBuffer();
	pbaGetCurrentEntry(&lEntry);
	
	while (lFoundEntry && lFilled && the_app->vCardData.maxList > 0)
	{
		lFilled = vcgFillBuffer(&lEntry);
		if (lFilled)
		{
			lFoundEntry = pbaGotoNextEntry(&lEntry);
			vcgResetState();
			the_app->vCardData.maxList--;
			the_app->srchData.count++;
		}
	}

	return (!lFoundEntry || (the_app->vCardData.maxList == 0));
}


/* Local Functions */

static void vcgResetState(void)
{
	the_app->vCardData.sentHeader = FALSE;
	the_app->vCardData.sentFooter = FALSE;
	the_app->vCardData.sentName = FALSE;
	the_app->vCardData.sentPhone = FALSE;
	the_app->vCardData.sentMobile = FALSE;
	the_app->vCardData.sentBusiness = FALSE;
}

static bool vcgFillBuffer(pbaSearchResult *pEntry)
{
	bool lComplete = FALSE;
	bool lSpace = TRUE;
	uint16 lSize;

	if (!the_app->vCardData.sentHeader)
	{
		lSize = sizeof(gVCardEntryHeader) + sizeof(gVCardEntryVersion21);
		if (lSize <= the_app->buffer.freeSpace)
		{
			memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], gVCardEntryHeader, sizeof(gVCardEntryHeader));
			the_app->buffer.nextPos += sizeof(gVCardEntryHeader);
			if (the_app->vCardData.format == pbap_format_30)
				memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], gVCardEntryVersion30, sizeof(gVCardEntryVersion30));
			else
				memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], gVCardEntryVersion21, sizeof(gVCardEntryVersion21));
			the_app->buffer.nextPos += sizeof(gVCardEntryVersion21);
			the_app->buffer.used += lSize;
			the_app->buffer.freeSpace -= lSize;
			the_app->vCardData.sentHeader = TRUE;
		}
		else
			lSpace = FALSE;
	}
	
	if (lSpace && !the_app->vCardData.sentName)
	{
		if (the_app->vCardData.format == pbap_format_30)
			lSpace = vcgAddElement(pEntry->namePtr, pEntry->nameSize, gVCardEntryName30, sizeof(gVCardEntryName30));
		else
			lSpace = vcgAddElement(pEntry->namePtr, pEntry->nameSize, gVCardEntryName21, sizeof(gVCardEntryName21));
		if (lSpace)
			the_app->vCardData.sentName = TRUE;
	}
	
	if (lSpace && !the_app->vCardData.sentPhone)
	{
		lSpace = vcgAddElement(pEntry->phonePtr, pEntry->phoneSize, gVCardEntryTel, sizeof(gVCardEntryTel));
		if (lSpace)
			the_app->vCardData.sentPhone = TRUE;
	}
		
	if (lSpace && !the_app->vCardData.sentMobile)
	{
		lSpace = vcgAddElement(pEntry->mobilePtr, pEntry->mobileSize, gVCardEntryTelMobile, sizeof(gVCardEntryTelMobile));
		if (lSpace)
			the_app->vCardData.sentMobile = TRUE;
	}
		
	if (lSpace && !the_app->vCardData.sentBusiness)
	{
		lSpace = vcgAddElement(pEntry->businessPtr, pEntry->businessSize, gVCardEntryTelBusiness, sizeof(gVCardEntryTelBusiness));
		if (lSpace)
			the_app->vCardData.sentBusiness = TRUE;
	}
		
	if (lSpace && !the_app->vCardData.sentFooter)
	{
		lSize = sizeof(gVCardEntryFooter);
		if (lSize <= the_app->buffer.freeSpace)
		{
			memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], gVCardEntryFooter, lSize);
			the_app->buffer.nextPos += lSize;
			the_app->buffer.used += lSize;
			the_app->buffer.freeSpace -= lSize;
			the_app->vCardData.sentFooter = TRUE;
		}
	}
	
	lComplete = (the_app->vCardData.sentName && the_app->vCardData.sentPhone && the_app->vCardData.sentMobile && the_app->vCardData.sentBusiness
					&& the_app->vCardData.sentHeader && the_app->vCardData.sentFooter);
	
	return lComplete;
}

static bool vcgAddElement(const uint8 *pElement, uint16 pSizeElement, const uint8 *pHeader, uint16 pSizeHeader)
{
	bool lSpace = TRUE;
	uint16 lSize;
	
	if (pElement)
	{
		lSize = pSizeElement + pSizeHeader + sizeof(gVCardEntryNewline);
		if (lSize <= the_app->buffer.freeSpace)
		{
			memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], pHeader, pSizeHeader);
			the_app->buffer.nextPos += pSizeHeader;
			memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], pElement, pSizeElement);
			the_app->buffer.nextPos += pSizeElement;
			memcpy(&the_app->buffer.buffer[the_app->buffer.nextPos], gVCardEntryNewline, sizeof(gVCardEntryNewline));
			the_app->buffer.nextPos += sizeof(gVCardEntryNewline);
			the_app->buffer.used += lSize;
			the_app->buffer.freeSpace -= lSize;
		}
		else
			lSpace = FALSE;
	}

	return lSpace;
}
