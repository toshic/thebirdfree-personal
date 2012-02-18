/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	Interface definition for accessing phonebooks

	
FILE
	pb_access.h
*/


#ifndef PB_ACCESS_H
#define PB_ACCESS_H

#include <file.h>

#include <pbaps.h>
#include "pbap_common.h"

/*
	Structure containing results of a search or goto element
*/
typedef struct
{
	const uint8 *namePtr;
	const uint8 *phonePtr;
	const uint8 *mobilePtr;
	const uint8 *businessPtr;
	uint16 nameSize;
	uint16 phoneSize;
	uint16 mobileSize;
	uint16 businessSize;
	uint16 index;
} pbaSearchResult;


/*
	Check to see if a phonebook is supported
*/
bool pbaPhoneBookSupported(pbap_phone_book pBook);

/*
	Open Phonebook
*/
bool pbaOpenPhonebook(pbap_phone_book pBook);

/*
	Close Phonebook
*/
void pbaClosePhonebook(void);

/* Get current entry */
void pbaGetCurrentEntry(pbaSearchResult *pResult);

/* Find next entry (uses search data) */
bool pbaFindNextEntry(pbaSearchResult *pResult);

/* 
	Goto next entry
*/
bool pbaGotoNextEntry(pbaSearchResult *pResult);

/*
	Goto entry
*/
bool pbaGotoEntry(uint16 pEntry);

/*
	Get the number of elements in a phonebook
*/
uint16 pbaGetNumberElements(pbap_phone_book pBook);

#endif /* PB_ACCESS_H */
