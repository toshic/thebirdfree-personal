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

#ifndef	SYNCS_H_
#define	SYNCS_H_

#include <panic.h>
#include <message.h>

#include "goep.h"

/*!
	@brief SYNC Server status. 
*/
typedef enum 
{
	/*! Last operation was successful.*/
	syncs_success,					
	/*! Last operation failed.*/
    syncs_failure,
	/*! Operation failed due to being in the wrong state.*/
	syncs_wrong_state,
	/*! Remote host has disconnected when it wasn't expected. Link Loss? */
	syncs_unscheduled_disconnect,
	/*! Remote Host has aborted the transfer. */
	syncs_remote_abort,
	
	/*! SDP Record registration failed during Init */
	syncs_sdp_failure,
	
	syncs_end_of_status_list
} syncs_lib_status;



/*! 
	@brief Possible return values to a GET request.
*/
typedef enum
{
	syncs_get_ok,
	
	syncs_get_eol
} syncs_get_result;


/*! 
	@brief Handle of the current PBAP Server Session.
   
	For client-side session management, this value can be used in comparisons,
	but it's meaning cannot be relied upon.
 */
struct __syncsState;
typedef struct __syncsState SYNCS; 

/*
   @brief Upstream Messages for the PBAP Client Library   
*/
#define SYNCS_MESSAGE_BASE	0x6200

/*
	Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
	/* Session Control */
	SYNCS_INIT_CFM = SYNCS_MESSAGE_BASE,
	
	SYNCS_CONNECT_IND,
	SYNCS_CONNECT_CFM,
	SYNCS_AUTH_RESULT_IND,
	SYNCS_DISCONNECT_IND,
	
	SYNCS_SET_PB_ROOT_IND,
	SYNCS_SET_PB_REPOSITORY_IND,
	SYNCS_SET_PB_BOOK_IND,
	SYNCS_SET_PB_PARENT_IND,
	
	SYNCS_GET_VCARD_LIST_START_IND,
	SYNCS_GET_VCARD_LIST_NEXT_IND,
	SYNCS_GET_VCARD_LIST_COMPLETE_IND,
	
	SYNCS_GET_VCARD_ENTRY_START_IND,
	SYNCS_GET_VCARD_ENTRY_NEXT_IND,
	SYNCS_GET_VCARD_ENTRY_COMPLETE_IND,
	
	SYNCS_GET_PHONEBOOK_START_IND,
	SYNCS_GET_PHONEBOOK_NEXT_IND,
	SYNCS_GET_PHONEBOOK_COMPLETE_IND,
	
	SYNCS_MESSAGE_TOP
} SYNCsMessageId;
#endif


/*!
	@brief This message returns the result of an PbapsInit attempt.
*/
typedef struct
{
	/*!< PBAPS Session Handle. */
	SYNCS            *syncs;
	/*!< The current status of the PBAPS library. */
    syncs_lib_status status;
} SYNCS_INIT_CFM_T;

/*!
     @brief This message is sent when an remote client attempts to make a
     connection.
*/
typedef struct
{
	/*! PBAPS Session Handle.*/
	SYNCS    *syncs;		
	/*! Bluetooth address of the remote client. */
	bdaddr  bd_addr;		
	/*! Maximum size of packet supported by this GOEP connection */
	uint16  maxPacketLen;	
} SYNCS_CONNECT_IND_T;

/*!
     @brief This message is sent when a remote connection completes.
*/
typedef struct
{
	/*! PBAPS Session Handle.*/
	SYNCS            *syncs;	    
	/*! The current status of the PBAPS library. */
	syncs_lib_status status;     	
	/*! Maximum size of packet supported by this GOEP connection */
	uint16          maxPacketLen;	
} SYNCS_CONNECT_CFM_T;

/*!
     @brief This message is sent when an authentication challenge completes during a connect.
*/
typedef struct
{
	/*! PBAPS Session Handle.*/
	SYNCS *syncs;
	uint8 digest[GOEP_SIZE_DIGEST];
	uint8 nonce[GOEP_SIZE_DIGEST];
	uint16 size_userid;
	uint8 userid[1];
} SYNCS_AUTH_RESULT_IND_T;

/*!
     @brief This message indicates that the remote client has started a
     DISCONNECT request.
*/
typedef struct
{
	/*!< PBAPS Session Handle.*/
	SYNCS    *syncs;
} SYNCS_DISCONNECT_IND_T;


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
void SyncsInit(Task theAppTask, uint16 priority, uint8 stores);

/*!
    @brief Respond to a connection request.  
    @param pbaps PBAPS Session Handle.
    @param accept TRUE to accept the connection, or FALSE to refuse it.
    @param pktSize Maximum packet size that can be accepted on this connection.
	
	This function should be called on recept of a PBAPS_CONNECT_IND message.
	A PBAPS_CONNECT_CFM message will be received on completion.
*/
void SyncsConnectResponse(SYNCS *syncs, bool accept, uint16 pktSize);

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
void SyncsConnectAuthChallenge(SYNCS *syncs, const uint8 *nonce, uint8 options, uint16 size_realm, const uint8 *realm);


#endif /* PBAP_SERVER_H_ */

