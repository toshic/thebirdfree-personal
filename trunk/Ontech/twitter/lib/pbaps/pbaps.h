/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbaps.h
    
DESCRIPTION
	PhoneBook Access Profile Server Library.

*/
/*!
@file pbaps.h
@brief PhoneBook Access Profile Server Library.


Library Dependecies : connection, region, service, goep, goep_apphdrs, bdaddr, pbap_common

Library variants:-
		pbaps - pbaps with no debug
		pbaps_debug - pbaps with debug checks
		pbaps_debug_print - pbaps with debug checks and print output
*/

#ifndef	PBAPS_H_
#define	PBAPS_H_

#include <panic.h>
#include <message.h>

#include "goep.h"
#include <pbap_common.h>

/*!
	@brief PBAP Server status. 
*/
typedef enum 
{
	/*! Last operation was successful.*/
	pbaps_success,					
	/*! Last operation failed.*/
    pbaps_failure,
	/*! Operation failed due to being in the wrong state.*/
	pbaps_wrong_state,
	/*! Remote host has disconnected when it wasn't expected. Link Loss? */
	pbaps_unscheduled_disconnect,
	/*! Remote Host has aborted the transfer. */
	pbaps_remote_abort,
	
	/*! SDP Record registration failed during Init */
	pbaps_sdp_failure,
	
	/*! Couldn't allocation memory for the search value in a PullvCardList Function */
	pbaps_pull_vcl_no_search_memory,
	/*! A folder was specified for a PullvCardList Request but it was invalid */
	pbaps_pull_vcl_invalid_folder,
	
	pbaps_end_of_status_list
} pbaps_lib_status;

/*! 
	@brief Possible return values to set phonebook requests.
*/
typedef enum 
{
	pbaps_spb_ok, 
	pbaps_spb_unauthorised, 
	pbaps_spb_no_repository,
	pbaps_spb_no_phonebook,
	
	pbaps_spb_eol
} pbaps_set_phonebook_result;

/*! 
	@brief Possible return values to a GET request.
*/
typedef enum
{
	pbaps_get_ok,
	
	pbaps_get_eol
} pbaps_get_result;


/*! 
	@brief Handle of the current PBAP Server Session.
   
	For client-side session management, this value can be used in comparisons,
	but it's meaning cannot be relied upon.
 */
struct __pbapsState;
typedef struct __pbapsState PBAPS; 

/*
   @brief Upstream Messages for the PBAP Client Library   
*/
#define PBAPS_MESSAGE_BASE	0x6100

/*
	Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
	/* Session Control */
	PBAPS_INIT_CFM = PBAPS_MESSAGE_BASE,
	
	PBAPS_CONNECT_IND,
	PBAPS_CONNECT_CFM,
	PBAPS_AUTH_RESULT_IND,
	PBAPS_DISCONNECT_IND,
	
	PBAPS_SET_PB_ROOT_IND,
	PBAPS_SET_PB_REPOSITORY_IND,
	PBAPS_SET_PB_BOOK_IND,
	PBAPS_SET_PB_PARENT_IND,
	
	PBAPS_GET_VCARD_LIST_START_IND,
	PBAPS_GET_VCARD_LIST_NEXT_IND,
	PBAPS_GET_VCARD_LIST_COMPLETE_IND,
	
	PBAPS_GET_VCARD_ENTRY_START_IND,
	PBAPS_GET_VCARD_ENTRY_NEXT_IND,
	PBAPS_GET_VCARD_ENTRY_COMPLETE_IND,
	
	PBAPS_GET_PHONEBOOK_START_IND,
	PBAPS_GET_PHONEBOOK_NEXT_IND,
	PBAPS_GET_PHONEBOOK_COMPLETE_IND,
	
	PBAPS_MESSAGE_TOP
} PbapsMessageId;
#endif


/*!
	@brief This message returns the result of an PbapsInit attempt.
*/
typedef struct
{
	/*!< PBAPS Session Handle. */
	PBAPS            *pbaps;
	/*!< The current status of the PBAPS library. */
    pbaps_lib_status status;
} PBAPS_INIT_CFM_T;

/*!
     @brief This message is sent when an remote client attempts to make a
     connection.
*/
typedef struct
{
	/*! PBAPS Session Handle.*/
	PBAPS    *pbaps;		
	/*! Bluetooth address of the remote client. */
	bdaddr  bd_addr;		
	/*! Maximum size of packet supported by this GOEP connection */
	uint16  maxPacketLen;	
} PBAPS_CONNECT_IND_T;

/*!
     @brief This message is sent when a remote connection completes.
*/
typedef struct
{
	/*! PBAPS Session Handle.*/
	PBAPS            *pbaps;	    
	/*! The current status of the PBAPS library. */
	pbaps_lib_status status;     	
	/*! Maximum size of packet supported by this GOEP connection */
	uint16          maxPacketLen;	
} PBAPS_CONNECT_CFM_T;

/*!
     @brief This message is sent when an authentication challenge completes during a connect.
*/
typedef struct
{
	/*! PBAPS Session Handle.*/
	PBAPS *pbaps;
	uint8 digest[GOEP_SIZE_DIGEST];
	uint8 nonce[GOEP_SIZE_DIGEST];
	uint16 size_userid;
	uint8 userid[1];
} PBAPS_AUTH_RESULT_IND_T;

/*!
     @brief This message indicates that the remote client has started a
     DISCONNECT request.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	PBAPS    *pbaps;
} PBAPS_DISCONNECT_IND_T;

/*!
     @brief This message indicates that the remote client has wishes the current
	 folder to be changed to the root folder.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	PBAPS *pbaps;
} PBAPS_SET_PB_ROOT_IND_T;

/*!
     @brief This message indicates that the remote client has wishes the current
	 folder to be changed to a repository folder.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	PBAPS *pbaps;
	/*! Identifier of the target repository */
	pbap_phone_repository repository;
} PBAPS_SET_PB_REPOSITORY_IND_T;

/*!
     @brief This message indicates that the remote client has wishes the current
	 folder to be changed to a phonebook folder.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	PBAPS *pbaps;
	/*! Identifier of the target phonebook */
	pbap_phone_book book;
} PBAPS_SET_PB_BOOK_IND_T;

/*!
     @brief This message indicates that the remote client wishes the current
	 folder to be changed to a phonebook folder.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	PBAPS *pbaps;
} PBAPS_SET_PB_PARENT_IND_T;

/*!
     @brief This message indicates that the remote client has requested the start
				of the GetvCardListing Function.
*/
typedef struct
{
	/*! PBAPS Session Handle.*/
	PBAPS            *pbaps;	    
	/*! The current status of the PBAPS library. */
	pbaps_lib_status status;     	
	pbap_order_values order;
	/*! Value to use for searching.  Ownership passes to the application.
		Pointer MUST BE free'd by the application. */
	uint8 *srchVal;
	uint16 size_srchVal;
	/*! Phonebook to obtain the list from. pbap_b_unknown for current. */
	pbap_phone_book pbook;
	pbap_search_values srchAttr;
	uint16 maxList;
	uint16 listStart;
} PBAPS_GET_VCARD_LIST_START_IND_T;

/*!
     @brief This message indicates that the remote client has requested the next
	 			packet of a PullvCardListing Function.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	PBAPS    *pbaps;
} PBAPS_GET_VCARD_LIST_NEXT_IND_T;

/*!
     @brief This message indicates that a PullvCardListing Function has completed.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	PBAPS    *pbaps;
	/*! The current status of the PBAPS library. */
	pbaps_lib_status status;     	
} PBAPS_GET_VCARD_LIST_COMPLETE_IND_T;

/*!
     @brief This message indicates that a PullvCardEntry Function has started.
*/
typedef struct
{
	PBAPS *pbaps;
	/*! Entry to pull */
	uint16 entry;
	pbap_format_values format;
	/*! Filter Low 32 bits */
	uint32 filter_lo;
	/*! Filter High 32 bits */
	uint32 filter_hi;
} PBAPS_GET_VCARD_ENTRY_START_IND_T;

/*!
     @brief This message indicates that the remote client has requested the next
	 			packet of a PullvCardEntry Function.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	PBAPS    *pbaps;
} PBAPS_GET_VCARD_ENTRY_NEXT_IND_T;

/*!
     @brief This message indicates that a PullvCardEntry Function has completed.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	PBAPS    *pbaps;
	/*! The current status of the PBAPS library. */
	pbaps_lib_status status;     	
} PBAPS_GET_VCARD_ENTRY_COMPLETE_IND_T;

/*!
     @brief This message indicates that a PullPhonebook Function has started.
*/
typedef struct
{
	PBAPS *pbaps;
	pbap_phone_repository repository;
	pbap_phone_book phonebook;
	pbap_format_values format;
	/*! Filter Low 32 bits */
	uint32 filter_lo;
	/*! Filter High 32 bits */
	uint32 filter_hi;
	uint16 maxList;
	uint16 listStart;
} PBAPS_GET_PHONEBOOK_START_IND_T;

/*!
     @brief This message indicates that the remote client has requested the next
	 			packet of a PullPhonebook Function.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	PBAPS    *pbaps;
} PBAPS_GET_PHONEBOOK_NEXT_IND_T;

/*!
     @brief This message indicates that a PullPhonebook Function has completed.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	PBAPS    *pbaps;
	/*! The current status of the PBAPS library. */
	pbaps_lib_status status;     	
} PBAPS_GET_PHONEBOOK_COMPLETE_IND_T;


/*   Downstream API for the PBAP Server Library   */

/*!
	@brief Initialise the PBAP Server Library.
	@param theAppTask The current application task.
	@param priority The profile instance low power mode priority. For a set of
	profile instances with connections on a given ACL the instance with the
	highest priority value wins and the low power mode on the ACL is set
	according to it's power table.
	@param repsositories Repositories supported by the server.  Mask values
	defined in pbap_common.h.
	
	PBAPS_INIT_CFM message will be received by the application. 
*/
void PbapsInit(Task theAppTask, uint16 priority, uint8 repositories);

/*!
    @brief Respond to a connection request.  
    @param pbaps PBAPS Session Handle.
    @param accept TRUE to accept the connection, or FALSE to refuse it.
    @param pktSize Maximum packet size that can be accepted on this connection.
	
	This function should be called on recept of a PBAPS_CONNECT_IND message.
	A PBAPS_CONNECT_CFM message will be received on completion.
*/
void PbapsConnectResponse(PBAPS *pbaps, bool accept, uint16 pktSize);

/*
	@brief Start an authentication challenge during connect.
    @param pbaps PBAPS Session Handle.
	@param nonce MD5 Digest Nonce to use. Must be a 16byte string.
	@param options Optional options to use.
	@param size_realm Length of the optional Realm string.
	@param realm Option realm string.
	
	This function can be called by a GOEP Server on receipt of a PBAPS_CONNECT_IND message to start
	and authentication challenge.
	A PBAPS_AUTH_RESULT_IND message will be received to indicate the result of the authentication challenge.
*/
void PbapsConnectAuthChallenge(PBAPS *pbaps, const uint8 *nonce, uint8 options, uint16 size_realm, const uint8 *realm);

/*!
    @brief Respond to a Set Phonebook Root request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	
	This function should be called on recept of a PBAPS_SET_PB_ROOT_IND message.
*/
void PbapsSetPhonebookRootResponse(PBAPS *pbaps, pbaps_set_phonebook_result result);

/*!
    @brief Respond to a Set Phonebook Repository request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	
	This function should be called on recept of a PBAPS_SET_PB_REPOSITORY_IND message.
*/
void PbapsSetPhonebookRepositoryResponse(PBAPS *pbaps, pbaps_set_phonebook_result result);

/*!
    @brief Respond to a Set Phonebook sub-folder request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	
	This function should be called on recept of a PBAPS_SET_PB_BOOK_IND message.
*/
void PbapsSetPhonebookBookResponse(PBAPS *pbaps, pbaps_set_phonebook_result result);

/*!
    @brief Respond to a Set Phonebook parent request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	
	This function should be called on recept of a PBAPS_SET_PB_PARENT_IND message.
*/
void PbapsSetPhonebookParentResponse(PBAPS *pbaps, pbaps_set_phonebook_result result);

/*!
    @brief Respond to a Get vCard List request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param packet Packet to send.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	If data is in a VM Source (e.g. from the filesystem),
	PbapsGetvCardListFirstPacketSource should be used instead of
	PbapsGetvCardListFirstPacket.
	
	This function should be called on recept of a PBAPS_GET_VCARD_LIST_START_IND message.
	A PBAPS_GET_VCARD_LIST_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_LIST_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardListFirstPacket(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, uint8 *packet, uint16 size_packet, bool onlyPacket);

/*!
    @brief Respond to a Get vCard List request sending data from a source.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param src Source containing the packet.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	This function should be called on recept of a PBAPS_GET_VCARD_LIST_START_IND message.
	A PBAPS_GET_VCARD_LIST_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_LIST_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardListFirstPacketSource(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, Source src, uint16 size_packet, bool onlyPacket);

/*!
    @brief Respond to a Get vCard List request sending application parameters.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param pbook_size The size of the phonebook if MaxListCount was zero.
	@param new_missed The number of new missed calls when the phonebook object us mch.
	
	This function should be called on recept of a PBAPS_GET_VCARD_LIST_START_IND message.
*/
void PbapsGetvCardListFirstPacketParams(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, uint16 pbook_size, uint8 new_missed);

/*!
    @brief Send the next packet of a Get vCard List request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param packet The packet to send
    @param lastPacket Is this the last packet to transfer.
	
	If data is in a VM Source (e.g. from the filesystem),
	PbapsGetvCardListNextPacketSource should be used instead of
	PbapsGetvCardListNextPacket.
	
	This function should be called on receipt of a PBAPS_GET_VCARD_LIST_NEXT_IND message.
	A PBAPS_GET_VCARD_LIST_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_LIST_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardListNextPacket(PBAPS *pbaps, uint16 size_packet,  const uint8 *packet, const bool lastPacket);

/*!
    @brief Send the next packet of a Get vCard List request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param src The source containing the packet
    @param lastPacket Is this the last packet to transfer.
	
	This function should be called on receipt of a PBAPS_GET_VCARD_LIST_NEXT_IND message.
	A PBAPS_GET_VCARD_LIST_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_LIST_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardListNextPacketSource(PBAPS *pbaps, uint16 size_packet,  Source src, const bool lastPacket);

/*!
    @brief Respond to a Get vCard Entry request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Entry
	@param packet Packet to send.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	If data is in a VM Source (e.g. from the filesystem),
	PbapsGetvCardEntryFirstPacketSource should be used instead of
	PbapsGetvCardEntryFirstPacket.
	
	This function should be called on recept of a PBAPS_GET_VCARD_ENTRY_START_IND message.
	A PBAPS_GET_VCARD_ENTRY_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_ENTRY_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardEntryFirstPacket(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, uint8 *packet, uint16 size_packet, bool onlyPacket);

/*!
    @brief Respond to a Get vCard Entry request sending data from a source.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Entry
	@param src Source containing the packet.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	This function should be called on recept of a PBAPS_GET_VCARD_ENTRY_START_IND message.
	A PBAPS_GET_VCARD_ENTRY_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_ENTRY_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardEntryFirstPacketSource(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, Source src, uint16 size_packet, bool onlyPacket);

/*!
    @brief Send the next packet of a Get vCard Entry request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param packet The packet to send
    @param lastPacket Is this the last packet to transfer.
	
	If data is in a VM Source (e.g. from the filesystem),
	PbapsGetvCardEntryNextPacketSource should be used instead of
	PbapsGetvCardEntryNextPacket.
	
	This function should be called on receipt of a PBAPS_GET_VCARD_ENTRY_NEXT_IND message.
	A PBAPS_GET_VCARD_ENTRY_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_ENTRY_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardEntryNextPacket(PBAPS *pbaps, uint16 size_packet,  const uint8 *packet, const bool lastPacket);

/*!
    @brief Send the next packet of a Get vCard Entry request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param src The source containing the packet
    @param lastPacket Is this the last packet to transfer.
	
	This function should be called on receipt of a PBAPS_GET_VCARD_ENTRY_NEXT_IND message.
	A PBAPS_GET_VCARD_ENTRY_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_VCARD_ENTRY_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetvCardEntryNextPacketSource(PBAPS *pbaps, uint16 size_packet,  Source src, const bool lastPacket);

/*!
    @brief Respond to a Get Phonebook request sending application parameters.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param pbook_size The size of the phonebook if MaxListCount was zero.
	@param new_missed The number of new missed calls when the phonebook object us mch.
	
	This function should be called on recept of a PBAPS_GET_PHONEBOOK_START_IND message.
*/
void PbapsGetPhonebookFirstPacketParams(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, uint16 pbook_size, uint8 new_missed);

/*!
    @brief Respond to a Get Phonebook request.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param packet Packet to send.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	If data is in a VM Source (e.g. from the filesystem),
	PbapsGetPhonebookFirstPacketSource should be used instead of
	PbapsGetPhonebookFirstPacket.
	
	This function should be called on recept of a PBAPS_GET_PHONEBOOK_START_IND message.
	A PBAPS_PHONEBOOK_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_PHONEBOOK_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetPhonebookFirstPacket(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, uint8 *packet, uint16 size_packet, bool onlyPacket);

/*!
    @brief Respond to a Get vCard List request sending data from a source.  
    @param pbaps PBAPS Session Handle.
    @param result Result of the request.
	@param totalLen Total length of the vCard Listing
	@param src Source containing the packet.
	@param size_packet Size of the packet being sent.
	@param onlyPacket Only 1 packet to send.
	
	This function should be called on recept of a PBAPS_GET_PHONEBOOK_START_IND message.
	A PBAPS_PHONEBOOK_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_PHONEBOOK_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetPhonebookFirstPacketSource(PBAPS *pbaps, pbaps_get_result result, uint32 totalLen, Source src, uint16 size_packet, bool onlyPacket);

/*!
    @brief Send the next packet of a Get Phonebook request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param packet The packet to send
    @param lastPacket Is this the last packet to transfer.
	
	If data is in a VM Source (e.g. from the filesystem),
	PbapsGetPhonebookNextPacketSource should be used instead of
	PbapsGetPhonebookNextPacket.
	
	This function should be called on receipt of a PBAPS_GET_PHONEBOOK_NEXT_IND message.
	A PBAPS_GET_PHONEBOOK_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_PHONEBOOK_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetPhonebookNextPacket(PBAPS *pbaps, uint16 size_packet,  const uint8 *packet, const bool lastPacket);

/*!
    @brief Send the next packet of a Get vCard List request.  
    @param pbaps PBAPS Session Handle.
    @param size_packet Length of this packet
    @param src The source containing the packet
    @param lastPacket Is this the last packet to transfer.
	
	This function should be called on receipt of a PBAPS_GET_PHONEBOOK_NEXT_IND message.
	A PBAPS_GET_PHONEBOOK_NEXT_IND message will be received to request the next packet.
	A PBAPS_GET_PHONEBOOK_COMPLETE_IND message will be received after sending the final packet.
*/
void PbapsGetPhonebookNextPacketSource(PBAPS *pbaps, uint16 size_packet,  Source src, const bool lastPacket);

#endif /* PBAP_SERVER_H_ */

