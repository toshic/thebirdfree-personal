/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	Implementation for accessing phonebooks
	
FILE
	pb_access.c
	
*/

/****************************************************************************
    Header files
*/

#include <print.h>
#include <stdlib.h>
#include <panic.h>
#include <stream.h>
#include <string.h>

#include "pb_access.h"
#include "audioAdaptor_private.h"

typedef struct
{
	const char *str;
	uint16 len;
} pbaPhonebookNames;

static const char  pbStr[] = "pb.dat";
static const char ichStr[] = "ich.dat";
static const char ochStr[] = "och.dat";
static const char mchStr[] = "mch.dat";
static const char cchStr[] = "cch.dat";

#define PBA_LIST_ENTRY(i) {(i), sizeof((i))-1}

static const pbaPhonebookNames gpbaPhonebookNamesList[] = 
							{ 
								PBA_LIST_ENTRY( pbStr),
								PBA_LIST_ENTRY(ichStr),
								PBA_LIST_ENTRY(ochStr),
								PBA_LIST_ENTRY(mchStr),
								PBA_LIST_ENTRY(cchStr)
							};

#define NAME_CHAR ('N')
#define PHONE_CHAR ('P')
#define MOBILE_CHAR ('M')
#define BUSINESS_CHAR ('B')
#define END_CHAR ('\n')
							
/* Local Function Definitions */

void zeroSearch(pbaSearchResult *pSearch);
uint16 findFields(const uint8 *pBuffer, pbaSearchResult *pSearch);
static bool pbaSearchEntry(pbaSearchResult *pResult);
static bool pbaSearchStr(const uint8 *str1, const uint8 *str2);

/* Interface Functions */

/*
	Check to see if a phonebook is supported
*/
bool pbaPhoneBookSupported(pbap_phone_book pBook)
{
	bool lRet = FALSE;
	char *pb_filename;

	if ((pBook >= pbap_pb) && (pBook < pbap_b_unknown))
	{
		FILE_INDEX idx;

		pb_filename = PanicUnlessMalloc(gpbaPhonebookNamesList[pBook-1].len + 3);
		sprintf(pb_filename,"%d/%s",the_app->pb_index,gpbaPhonebookNamesList[pBook-1].str);
		
		/* pbap_pb = 1, so need to knock 1 off to make list 0 indexed */
/*		idx = FileFind(FILE_ROOT, gpbaPhonebookNamesList[pBook-1].str, gpbaPhonebookNamesList[pBook-1].len);*/

		idx = FileFind(FILE_ROOT,pb_filename,strlen(pb_filename));
		free(pb_filename);

		if (idx != FILE_NONE)
		{
			lRet = TRUE;
		}
	}
	else
	{
		DEBUG_PBAP(("    Not a valid phonebook\n"));
	}
	return lRet;
}

/*
	Open Phonebook
*/
bool pbaOpenPhonebook(pbap_phone_book pBook)
{
	bool lRet = FALSE;
	char *pb_filename;

	DEBUG_PBAP(("pbaOpenPhonebook\n"));
	if (the_app->phonebookData.open)
	{
		DEBUG_PBAP(("    Phonebook already open\n"));
	}
	else
	{
		if ((pBook >= pbap_pb) && (pBook < pbap_b_unknown))
		{
			FILE_INDEX idx;

			pb_filename = PanicUnlessMalloc(gpbaPhonebookNamesList[pBook-1].len + 3);
			sprintf(pb_filename,"%d/%s",the_app->pb_index,gpbaPhonebookNamesList[pBook-1].str);
			
			/* pbap_pb = 1, so need to knock 1 off to make list 0 indexed */
/*			idx = FileFind(FILE_ROOT, gpbaPhonebookNamesList[pBook-1].str, gpbaPhonebookNamesList[pBook-1].len);*/
			idx = FileFind(FILE_ROOT,pb_filename,strlen(pb_filename));
			free(pb_filename);
			
			if (idx != FILE_NONE)
			{
				the_app->phonebookData.open = TRUE;
				the_app->phonebookData.bookSrc = StreamFileSource(idx);
				/* pbap_pb phone book as a 0 indexed entry, others do not */
				the_app->phonebookData.entry = (pBook == pbap_pb) ? 0 : 1;
				the_app->phonebookData.usedFirst = FALSE;
				lRet = TRUE;
			}
			else
			{
				DEBUG_PBAP(("    Unable to find phonebook\n"));
			}
		}
		else
		{
			DEBUG_PBAP(("    Not a valid phonebook\n"));
		}
	}
	
	return lRet;
}

/*
	Close Phonebook
*/
void pbaClosePhonebook(void)
{
	uint16 lSize;
	
	lSize = SourceSize(the_app->phonebookData.bookSrc);
	while (lSize>0)
	{
		SourceDrop(the_app->phonebookData.bookSrc, lSize);
		lSize = SourceSize(the_app->phonebookData.bookSrc);
	}
	
	the_app->phonebookData.open = FALSE;
	the_app->phonebookData.bookSrc = 0;
}

/* Get current entry */
void pbaGetCurrentEntry(pbaSearchResult *pResult)
{
	const uint8 *lSrc = SourceMap(the_app->phonebookData.bookSrc);
	
	DEBUG_PBAP(("pbaGetCurrentEntry\n"));
	
	if (!the_app->phonebookData.open)
	{
		DEBUG_PBAP(("    PHONEBOOK NOT OPEN\n"));
		return;
	}
	
	zeroSearch(pResult);
	pResult->index = the_app->phonebookData.entry;
	findFields(lSrc, pResult);
}

/* Find next entry (uses search data) */
bool pbaFindNextEntry(pbaSearchResult *pResult)
{
	bool lFound = FALSE;
	const uint8 *lSrc = SourceMap(the_app->phonebookData.bookSrc);
	uint16 lSize;
	
	DEBUG_PBAP(("pbaFindNextEntry\n"));
	
	if (!the_app->phonebookData.open)
	{
		DEBUG_PBAP(("    PHONEBOOK NOT OPEN\n"));
		return FALSE;
	}
	
	if (the_app->phonebookData.usedFirst)
	{
		/* Drop entry that has just been used */
		lSize = findFields(lSrc, pResult);
		SourceDrop(the_app->phonebookData.bookSrc, lSize);
		the_app->phonebookData.entry++;
	}
	else
	{ /* Indicate that the first entry has been processed */
		the_app->phonebookData.usedFirst = TRUE;
	}
	
	while ((SourceSize(the_app->phonebookData.bookSrc)>0) && (!lFound))
	{
		zeroSearch(pResult);
		pResult->index = the_app->phonebookData.entry;
		lSize = findFields(lSrc, pResult);
		
		/* Perform Search */
		lFound = pbaSearchEntry(pResult);
		
		if (!lFound)
		{
			SourceDrop(the_app->phonebookData.bookSrc, lSize);
			the_app->phonebookData.entry++;
		}
	}
	
	return lFound;
}

/*
	Goto entry
*/
bool pbaGotoEntry(uint16 pEntry)
{
	const uint8 *lSrc = SourceMap(the_app->phonebookData.bookSrc);
	pbaSearchResult lSearch;
	uint16 lSize;
	
	DEBUG_PBAP(("pbaGotoEntry\n"));
	
	while ((SourceSize(the_app->phonebookData.bookSrc)>0) && (the_app->phonebookData.entry < pEntry))
	{
		lSize = findFields(lSrc, &lSearch);
		
		if (the_app->phonebookData.entry < pEntry)
		{
			SourceDrop(the_app->phonebookData.bookSrc, lSize);
		}
		the_app->phonebookData.entry++;
	}
	/* Indicate the current entry should be examined during searching */
	the_app->phonebookData.usedFirst = FALSE;

	return (the_app->phonebookData.entry >= pEntry) ? TRUE : FALSE;
}

/* 
	Goto next entry
*/
bool pbaGotoNextEntry(pbaSearchResult *pResult)
{
	const uint8 *lSrc = SourceMap(the_app->phonebookData.bookSrc);
	uint16 lSize;
	bool lComplete = FALSE;
	
	DEBUG_PBAP(("pbaGotoNextEntry\n"));

	/* Remove Current entry */	
	lSize = findFields(lSrc, pResult);
	SourceDrop(the_app->phonebookData.bookSrc, lSize);
	the_app->phonebookData.entry++;
	
	/* Fill Out  Current entry */	
	if (SourceSize(the_app->phonebookData.bookSrc) > 0)
	{
		zeroSearch(pResult);
		lSize = findFields(lSrc, pResult);
		lComplete = TRUE;
	}

	return lComplete;
}

/*
	Get the number of elements in a phonebook
*/
uint16 pbaGetNumberElements(pbap_phone_book pBook)
{
	const uint8 *lSrc;
	uint16 lNumEntries = 0;
	uint16 lSize;
	pbaSearchResult lSearch;

	DEBUG_PBAP(("pbaGetNumberElements\n"));
	
	pbaOpenPhonebook(pBook);
	lSrc = SourceMap(the_app->phonebookData.bookSrc);
	
	while (SourceSize(the_app->phonebookData.bookSrc)>0)
	{
		lSize = findFields(lSrc, &lSearch);
		SourceDrop(the_app->phonebookData.bookSrc, lSize);
		lNumEntries++;
	}
	
	pbaClosePhonebook();
	DEBUG_PBAP(("    Num Entries %d\n",lNumEntries));
		
	return lNumEntries;
}

/* Local Function Implementations */

void zeroSearch(pbaSearchResult *pSearch)
{
	pSearch->namePtr = NULL;
	pSearch->phonePtr = NULL;
	pSearch->mobilePtr = NULL;
	pSearch->businessPtr = NULL;
	pSearch->nameSize = 0;
	pSearch->phoneSize = 0;
	pSearch->mobileSize = 0;
	pSearch->businessSize = 0;
	pSearch->index = 0;
}

uint16 findFields(const uint8 *pBuffer, pbaSearchResult *pSearch)
{
	const uint8 *lCurr = pBuffer, *lField = pBuffer;
	uint16 lLength = 0;
	
	DEBUG_PBAP(("findFields\n"));
	
	while (*lField != END_CHAR)
	{
		while (*lCurr != END_CHAR)
			lCurr++;
		lLength = lCurr - lField - 1;
		switch (*lField)
		{
			case NAME_CHAR:
				pSearch->namePtr = lField+1;
				pSearch->nameSize = lLength;
				break;
			case PHONE_CHAR:
				pSearch->phonePtr = lField+1;
				pSearch->phoneSize = lLength;
				break;
			case MOBILE_CHAR:
				pSearch->mobilePtr = lField+1;
				pSearch->mobileSize = lLength;
				break;
			case BUSINESS_CHAR:
				pSearch->businessPtr = lField+1;
				pSearch->businessSize = lLength;
				break;
			default:
				DEBUG_PBAP(("    UNKNOWN FIELD\n"));
				Panic();
				break;
		}
		lCurr ++;
		lField = lCurr;
	}
	
	return lCurr - pBuffer + 1;
}

static bool pbaSearchEntry(pbaSearchResult *pResult)
{
	bool lFound = FALSE;
	
	if (the_app->srchData.srchVal)
	{
		switch (the_app->srchData.srchAttr)
		{
			case pbap_search_name:
			case pbap_search_sound:  /* Deliberate Fallthrough */
				{
					if (pResult->namePtr)
					{
						lFound = pbaSearchStr(pResult->namePtr, the_app->srchData.srchVal);
					}
					break;
				}
			case pbap_search_number:
				{
					if (pResult->phonePtr)
					{
						lFound = pbaSearchStr(pResult->phonePtr, the_app->srchData.srchVal);
					}
					if ((pResult->mobilePtr) && !lFound)
					{
						lFound = pbaSearchStr(pResult->mobilePtr, the_app->srchData.srchVal);
					}
					if ((pResult->businessPtr) && !lFound)
					{
						lFound = pbaSearchStr(pResult->businessPtr, the_app->srchData.srchVal);
					}
					break;
				}
			default:
				DEBUG_PBAP(("    UNKNOWN SEARCH CATAGORY\n"));
				Panic();
				break;
		}
	}
	else
		lFound = TRUE;
	return lFound;
}

static bool pbaSearchStr(const uint8 *str1, /* '\n' terminated */
							const uint8 *str2) /* Null terminated */
{
        const uint8 *cp = str1;
        const uint8 *s1, *s2;
        bool lFound = FALSE;

        if ( *str2 )
        {
	        while (*cp != '\n' && !lFound)
	        {
	                s1 = cp;
	                s2 = str2;
	
	                while ( *s1 != '\n' && *s2 && !(*s1-*s2) )
	                        s1++, s2++;
	
	                if (!*s2)
	                        lFound = TRUE;
	
	                cp++;
	        }
        }

        return lFound;

}
