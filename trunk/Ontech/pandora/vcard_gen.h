/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	Interface definition for handling vCard generation functionality

	
FILE
	vcard_gen.h
*/


#ifndef VCARD_GEN_H
#define VCARD_GEN_H

#include <pbap_common.h>

/*
	Start downloading a vCard entry
*/
bool vcgGetFirstEntryBuffer(uint16 pEntry, pbap_format_values pFormat);

/*
	Download the next buffer of the vCard Entry
*/
bool vcgGetNextEntryBuffer(void);

/*
	Clean up from downloading the vCard entry
*/
void vcgCleanupListBuffer(void);

/*
	Start downloading a complete phonebook
*/
bool vcgGetFirstPhonebookBuffer(pbap_phone_book pBook, uint16 pListStart, pbap_format_values pFormat, uint16 pMaxList);

/*
	Download the next buffer of a complete phonebook
*/
bool vcgGetNextPhonebookBuffer(void);

#endif /* VCARD_GEN_H */
