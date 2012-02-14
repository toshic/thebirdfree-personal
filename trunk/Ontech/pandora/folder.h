/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	Interface definition for handling folder based functionality

	
FILE
	folder.h
*/


#ifndef FOLDER_H
#define FOLDER_H

#include <pbap_common.h>
#include <pbaps.h>

/*
	Initialise the folder sub-system
*/
void folderInit(void);

/*
	Set current folder to a child folder
*/
pbaps_set_phonebook_result folderSetChild(pbap_phone_book pBook);

/*
	Set current folder to parent folder
*/
pbaps_set_phonebook_result folderSetParent(void);

/*
	Set current folder to root folder
*/
pbaps_set_phonebook_result folderSetRoot(void);

/*
	Return the current folder
*/
pbap_phone_book folderCurrentFolder(void);

/*
	Get first vCard list buffer
*/
bool folderGetFirstListBuffer(uint16 pStartPos);

/*
	Get next vCard list buffer
*/
bool folderGetNextListBuffer(void);

/*
	Clean up from downloading the vCard list
*/
void folderCleanupListBuffer(void);

#endif /* FOLDER_H */
