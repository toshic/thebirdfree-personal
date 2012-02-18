/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbap_common.h
    
DESCRIPTION
 Helper library for PhoneBook Access Profile Server and Client Libraries.

*/
/*!
@file pbap_common.h
@brief Helper library for PhoneBook Access Profile Server and Client Libraries.


Library Dependecies : None
*/

#ifndef PBAP_COMMON_H_
#define PBAP_COMMON_H_

#include <goep.h>

/*! @name SDP Attributes
	@brief SDP Attribute defines.
*/
/*! \{ */
/*!
	@brief Supported Repositories UUID.
*/
#define PBAP_REPOS 0x0314
/*! \} */


/*!
	@brief Valid phonebook repositories.
*/
typedef enum 
{
	pbap_current,
	pbap_local, 
	pbap_sim1, 
	
	pbap_r_unknown
} pbap_phone_repository;

/*! @name Supported Repositories
	@brief Mask Values for Supported Repositories
	
	These values are masked together to generate the supported repositories
	for SDP registration and searching.
*/
/*! \{ */
/*!
	@brief Mask for Local Repository.
*/
#define PBAP_REP_LOCAL (1<<0)
/*!
	@brief Mask for SIM1 Repository.
*/
#define PBAP_REP_SIM1  (1<<1)
/*! \} */

/*! @name Supported features
	@brief Mask Values for Supported Features
	
	These values are masked together to generate the supported features
	for SDP registration and searching.
*/
/*! \{ */
/*!
	@brief Mask for Phonebook Download.
*/
#define PBAP_SUP_DOWNLOAD (1<<0)
/*!
	@brief Mask for Phonebook Browsing.
*/
#define PBAP_SUP_BROWSE  (1<<1)
/*! \} */

/*!
	@brief Valid phonebook folders.
*/
typedef enum
{
	pbap_telecom, 
	pbap_pb, 
	pbap_ich, 
	pbap_och, 
	pbap_mch, 
	pbap_cch,
	
	pbap_b_unknown
} pbap_phone_book;

/*!
	@brief Phonebook Access Profile Specific Parameters.
*/
typedef enum 
{
	/*! Order. */
	pbap_param_order = 0x01,
	/*! Search Value */
	pbap_param_srch_val = 0x02, 
	/*! Search Atribute */
	pbap_param_srch_attr = 0x03, 
	/*! Maximum List Count */
	pbap_param_max_list = 0x04, 
	/*! List Start Offset */
	pbap_param_strt_offset = 0x05, 
	/*! Filter */
	pbap_param_filter = 0x06, 
	/*! Format */
	pbap_param_format = 0x07, 
	/*! Phonebook Size */
	pbap_param_pbook_size = 0x08, 
	/*! New Missed Calls */
	pbap_param_missed_calls = 0x09
} pbap_goep_parameters;

/*!
	@name Parameter Default Values.
	@brief Phonebook Access Profile Specific Parameter Default Values.
*/
/*! \{ */
/*! @brief Order. 
  	[Indexed]
*/
#define pbap_param_order_def pbap_order_idx

/*! @brief Search Atribute.
  	[Name]
*/
#define pbap_param_srch_attr_def pbap_search_name

/*! @brief Maximum List Count.
  	[All / Unrestricted]
*/
#define pbap_param_max_list_def 65535

/*! @brief List Start Offset.
  	[Start from item 0]
*/
#define pbap_param_strt_offset_def 0 

/*! @brief Filter.
  	[No Filter]
*/
#define pbap_param_filter_def 0

/*! @brief Format.
  	[vCard 2.1]
*/
#define pbap_param_format_def pbap_format_21

/*! @brief Phonebook Size.
  	[Unknown]
*/
#define pbap_param_pbook_size_def 0

/*! @brief New Missed Calls.
  	[No new]
*/
#define pbap_param_missed_calls_def 0
/*! \} */

/*!
	@brief Order values for use with the PullvCardListing function.
*/
typedef enum 
{
	/*! Indexed. */
	pbap_order_idx = 0x00,
	/*! Alphanumeric */
	pbap_order_alpha = 0x01, 
	/*! Phonetic */
	pbap_order_phone = 0x02,
	
	pbap_order_default
} pbap_order_values;

/*!
	@brief Search Attributes to use with the PullvCardListing function.
*/
typedef enum 
{
	/*! Name. */
	pbap_search_name = 0x00,
	/*! Number */
	pbap_search_number = 0x01, 
	/*! Sound */
	pbap_search_sound = 0x02,
	
	pbap_a_unknown
} pbap_search_values;

/*!
	@brief vCard formats to use with the PullvCardEntry and PullPhonebook functions.
*/
typedef enum 
{
	/*! vCard 2.1. */
	pbap_format_21 = 0x00,
	/*! vCard 3.0 */
	pbap_format_30 = 0x01,
	/*! Use default value */
	pbap_format_def
} pbap_format_values;

/*!
	@name Filter Attribute Mask Values.
	@brief Mask values for the Phonebook Access Profile Specific Parameter Filter.
*/
/*! \{ */
/*! @brief vCard Version (VERSION). 
*/
#define pbap_filter_version (1<<0)

/*! @brief Formatted Name (FN). 
*/
#define pbap_filter_fn (1<<1)

/*! @brief Structured Presentation of Name (N). 
*/
#define pbap_filter_n (1<<2)

/*! @brief Photo Associated with the name (PHOTO). 
*/
#define pbap_filter_photo (1<<3)

/*! @brief Birthday (BDAY). 
*/
#define pbap_filter_bday (1<<4)

/*! @brief Delivery Address (ADR). 
*/
#define pbap_filter_adr (1<<5)

/*! @brief Delivery Label (LABEL). 
*/
#define pbap_filter_label (1<<6)

/*! @brief Telephone Number (TEL). 
*/
#define pbap_filter_tel (1<<7)

/*! @brief Electronic Mail Address (EMAIL). 
*/
#define pbap_filter_email (1<<8)

/*! @brief Electronic Mail (MAILER). 
*/
#define pbap_filter_mailer (1<<9)

/*! @brief Time Zone (TZ). 
*/
#define pbap_filter_tz (1<<10)

/*! @brief Geographic Position (GEO). 
*/
#define pbap_filter_geo (1<<11)

/*! @brief Job Title (TITLE). 
*/
#define pbap_filter_title (1<<12)

/*! @brief Role within the Organisation (ROLE). 
*/
#define pbap_filter_role (1<<13)

/*! @brief Organisation Logo (LOGO). 
*/
#define pbap_filter_logo (1<<14)

/*! @brief vCard of person representing (AGENT). 
*/
#define pbap_filter_agent (1<<15)

/*! @brief Name of the Organisation (ORG). 
*/
#define pbap_filter_org (1<<16)

/*! @brief Comments (NOTE). 
*/
#define pbap_filter_note (1<<17)

/*! @brief vCard Revision (REV). 
*/
#define pbap_filter_rev (1<<18)

/*! @brief Pronunciation of Name (SOUND). 
*/
#define pbap_filter_sound (1<<19)

/*! @brief URL (URL). 
*/
#define pbap_filter_url (1<<20)

/*! @brief Unique ID (UID). 
*/
#define pbap_filter_uid (1<<21)

/*! @brief Public Encryption Key (KEY). 
*/
#define pbap_filter_key (1<<22)
 
/*! @brief Nickname (NICKNAME). 
*/
#define pbap_filter_nickname (1<<23)
 
/*! @brief Catagories (CATAGORIES). 
*/
#define pbap_filter_catagory (1<<24)
 
/*! @brief Product ID (PROID). 
*/
#define pbap_filter_proid (1<<25)
 
/*! @brief Class Information (CLASS). 
*/
#define pbap_filter_class (1<<26)
 
/*! @brief String used for sorting operations (SORT-STRING). 
*/
#define pbap_filter_sort (1<<27)
 
/*! @brief Timestamp (X-IRMC-CALL-DATETIME). 
*/
#define pbap_filter_call_datetime (1<<28)
 
/*! @brief Proprietary filters in use.
  		Used in the HIGH 32 bits.
*/
#define pbap_filter_hi_proprietary (1<<8)
/*! \} */

/*!
	@name Manditory Filter Attribute Mask Values.
	@brief Manditory Mask values for the Phonebook Access Profile Specific Parameter Filter.
*/
/*! \{ */
/*! @brief vCard version 2.1 manditory attributes. 
*/
#define pbap_filter_vcard21 (pbap_filter_n | pbap_filter_tel)

/*! @brief vCard version 3.0 manditory attributes. 
*/
#define pbap_filter_vcard30 (pbap_filter_fn | pbap_filter_tel)

/*! \} */

/*!
	@brief Convert a phone Repository ID into a phone Repository name
	@param store Store to find the name of
	@param len Length of the returned name
	
	Returns Pointer to the Repository name.  len contains the length of the returned name.
	On Error returns NULL and the contents of len are unchanged.
	
	The pointer MUST NOT be freed.
*/
const uint8 *PbapcoGetRepositoryNameFromID(pbap_phone_repository repos, uint16 *len);

/*!
	@brief Convert a phone Repository name into a phone Repository ID
	@param store Store to find the ID of
	@param len Length of the Repository name
	
	Returns Store ID.
	On Error returns pbap_r_unknown.
	
*/
pbap_phone_repository PbapcoGetRepositoryIDFromName(const uint8 *repos, uint16 len);

/*!
	@brief Convert a phone book ID into a phone book name
	@param book Book to find the name of
	@param len Length of the returned name
	
	Returns Pointer to the book name.  len contains the length of the returned name.
	On Error returns NULL and the contents of len are unchanged.
	
	The pointer MUST NOT be freed.
*/
const uint8 *PbapcoGetBookNameFromID(pbap_phone_book book, uint16 *len);

/*!
	@brief Convert a phone book name into a phone book ID
	@param book Book to find the ID of
	@param len Length of the book name
	
	Returns Book ID.
	On Error returns pbap_b_unknown.
	
*/
pbap_phone_book PbapcoGetBookIDFromName(const uint8 *book, uint16 len);

/*!
	@brief Get a pointer to the PBAB target string
	@param len Length of the returned name
	
	Returns Pointer to the target.  len contains the length of the returned name.
	
	The pointer MUST NOT be freed.
*/
const uint8 *PbapcoGetTargetString(uint16 *len);

/*!
	@brief Get a pointer to the PBAB vCard Listing mime type
	@param len Length of the returned name
	
	Returns Pointer to the string.  len contains the length of the returned string.
	
	The pointer MUST NOT be freed.
*/
const uint8 *PbapcoGetvCardListingMimeType(uint16 *len);

/*!
	@brief Get a pointer to the PBAB vCard mime type
	@param len Length of the returned name
	
	Returns Pointer to the string.  len contains the length of the returned string.
	
	The pointer MUST NOT be freed.
*/
const uint8 *PbapcoGetvCardMimeType(uint16 *len);

/*!
	@brief Get a pointer to the PBAB Phonebook mime type
	@param len Length of the returned name
	
	Returns Pointer to the string.  len contains the length of the returned string.
	
	The pointer MUST NOT be freed.
*/
const uint8 *PbapcoGetPhonebookMimeType(uint16 *len);

#endif /* PBAP_COMMON_H_ */
