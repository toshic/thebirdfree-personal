/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbap_common.c
    
DESCRIPTION
 Helper library for PhoneBook Access Profile Server and Client Libraries.

*/


#include <vm.h>
#include <string.h>

#include "pbap_common.h"



/* Strings for phonebook directories */
static const uint8 pbap_str_telecom[]	= {0,'t',0,'e',0,'l',0,'e',0,'c',0,'o',0,'m',0,0};
static const uint8 pbap_str_pb[]		= {0,'p',0,'b',0,0};
static const uint8 pbap_str_ich[]		= {0,'i',0,'c',0,'h',0,0};
static const uint8 pbap_str_och[]		= {0,'o',0,'c',0,'h',0,0};
static const uint8 pbap_str_mch[]		= {0,'m',0,'c',0,'h',0,0};
static const uint8 pbap_str_cch[]		= {0,'c',0,'c',0,'h',0,0};



/* Strings for Stores Directories */
static const uint8 pbap_str_sim1[] =  {0,'S',0,'I',0,'M',0,'1',0,0};


/* String for the PBAP target */
static const uint8 pbap_str_target[] =  {0x79, 0x61, 0x35, 0xf0,   0xf0, 0xc5,   0x11, 0xd8,
										 	0x09, 0x66,   0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};

/* String for the vCard Listing mime Type */
static const uint8 pbap_str_vcardlisting[] =  {"x-bt/vcard-listing"};

/* String for the vCard Entry mime Type */
static const uint8 pbap_str_vcard[] =  {"x-bt/vcard"};

/* String for the Phonebook mime Type */
static const uint8 pbap_str_phonebook[] =  {"x-bt/phonebook"};




/*!
	@brief Convert a phone Repository ID into a phone Repository name
	@param store Store to find the name of
	@param len Length of the returned name
	
	Returns Pointer to the Repository name.  len contains the length of the returned name.
	On Error returns NULL and the contents of len are unchanged.
	
*/
const uint8 *PbapcoGetRepositoryNameFromID(pbap_phone_repository repos, uint16 *len)
{
	uint16 idx = repos - pbap_sim1;
	const uint8 *ret = 0;
	
	const uint8 *pbap_repos_names[] = {&pbap_str_sim1[0]};
	const uint16 pbap_repos_names_size[] = {sizeof(pbap_str_sim1)};

	/* check is within range of the number of repositories */	
	if (idx < sizeof(pbap_repos_names))
	{
		*len = pbap_repos_names_size[idx];
		ret = pbap_repos_names[idx];
	}
				 
	return ret;
}

/*!
	@brief Convert a phone Repository name into a phone Repository ID
	@param store Store to find the ID of
	@param len Length of the Repository name
	
	Returns Store ID.
	On Error returns pbap_r_unknown.
	
*/
pbap_phone_repository PbapcoGetRepositoryIDFromName(const uint8 *repos, uint16 len)
{
	pbap_phone_book ret = pbap_sim1;
	const uint8 *pbap_repos_names[] = {&pbap_str_sim1[0]};	
	
	while (ret < pbap_r_unknown)
	{
		if (memcmp(repos, pbap_repos_names[ret-pbap_sim1], len) == 0)
			break;
		ret++;
	}
	
	return ret;
}

/*!
	@brief Convert a phone book ID into a phone book name
	@param book Book to find the name of
	@param len Length of the returned name
	
	Returns Pointer to the book name.  len contains the length of the returned name.
	On Error returns NULL and the contents of len are unchanged.
	
*/
const uint8 *PbapcoGetBookNameFromID(pbap_phone_book book, uint16 *len)
{
	const uint8 *ret = 0;
	
	const uint8 *pbap_dir_names[] = {&pbap_str_telecom[0], &pbap_str_pb[0], &pbap_str_ich[0], 
										&pbap_str_och[0], &pbap_str_mch[0], &pbap_str_cch[0]};
	const uint16 pbap_dir_names_size[] = {sizeof(pbap_str_telecom), sizeof(pbap_str_pb), sizeof(pbap_str_ich), 
										sizeof(pbap_str_och), sizeof(pbap_str_mch), sizeof(pbap_str_cch)};

	/* check is within range of the number of phone book dirs  */
	if (book < sizeof(pbap_dir_names))
	{
		*len = pbap_dir_names_size[book];
		ret = pbap_dir_names[book];
	}
				 
	return ret;
}

/*!
	@brief Convert a phone book name into a phone book ID
	@param book Book to find the ID of
	@param len Length of the book name
	
	Returns Book ID.
	On Error returns pbap_b_unknown.
	
*/
pbap_phone_book PbapcoGetBookIDFromName(const uint8 *book, uint16 len)
{
	pbap_phone_book ret = pbap_telecom;
	
	const uint8 *pbap_dir_names[] = {&pbap_str_telecom[0], &pbap_str_pb[0], &pbap_str_ich[0], 
										&pbap_str_och[0], &pbap_str_mch[0], &pbap_str_cch[0]};
	
	while (ret < pbap_b_unknown)
	{
		if (memcmp(book, pbap_dir_names[ret], len) == 0)
			break;
		ret++;
	}
	
	return ret;
}

/*!
	@brief Get a pointer to the PBAB vCard Listing mime type
	@param len Length of the returned name
	
	Returns Pointer to the string.  len contains the length of the returned string.
	
	The pointer MUST NOT be freed.
*/
const uint8 *PbapcoGetvCardListingMimeType(uint16 *len)
{
	*len = sizeof(pbap_str_vcardlisting);
	return &pbap_str_vcardlisting[0];
}

/*!
	@brief Get a pointer to the PBAB vCard mime type
	@param len Length of the returned name
	
	Returns Pointer to the string.  len contains the length of the returned string.
	
	The pointer MUST NOT be freed.
*/
const uint8 *PbapcoGetvCardMimeType(uint16 *len)
{
	*len = sizeof(pbap_str_vcard);
	return &pbap_str_vcard[0];
}

/*!
	@brief Get a pointer to the PBAB target string
	@param len Length of the returned name
	
	Returns Pointer to the target.  len contains the length of the returned name.
	
	The pointer MUST NOT be freed.
*/
const uint8 *PbapcoGetTargetString(uint16 *len)
{
	*len = sizeof(pbap_str_target);
	return &pbap_str_target[0];
}

/*!
	@brief Get a pointer to the PBAB Phonebook mime type
	@param len Length of the returned name
	
	Returns Pointer to the string.  len contains the length of the returned string.
	
	The pointer MUST NOT be freed.
*/
const uint8 *PbapcoGetPhonebookMimeType(uint16 *len)
{
	*len = sizeof(pbap_str_phonebook);
	return &pbap_str_phonebook[0];
}

