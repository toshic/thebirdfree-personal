/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    oppc.h
    
DESCRIPTION
	Interface definition for the OPP Client library

*/
/*!
@file	oppc.h
@brief	Interface to the OPP Client library


Library Dependecies : connection,region,service, goep, bdaddr

Library variants:-
		oppc - oppc with no debug
		oppc_debug - oppc with debug checks
		oppc_debug_print - oppc with debug checks and print output
*/

#ifndef	OPPC_H_
#define	OPPC_H_

#include <bdaddr_.h>
#include <message.h>
#include <source.h>
#include <goep.h>


/*!
	@brief FTP status. 
*/
typedef enum 
{
	/*! Last operation was successful.*/
	oppc_success,		
	/*! Last operation failed.*/
    oppc_failure,		
	/*! A command has been attempted while another command is already
	  running. */
    oppc_not_idle,      
	/*! A command has been attempted while a different multi-packet command is
	  running. */
    oppc_wrong_command, 
	/*! Operation failed due to being in the wrong state.*/
	oppc_wrong_state,
	
	oppc_badrequest,
	oppc_forbidden,
	oppc_notfound,
	
	oppc_aborted,
	
	oppc_end_of_status_list
} oppc_lib_status;

/*! 
	@brief Handle of the current OPP Client Session.
   
	For client-side session management, this value can be used in comparisons,
	but it's meaning cannot be relied upon.
 */
struct __oppcState;
typedef struct __oppcState OPPC; 

/*
   @brief Upstream Messages for the OPP Client Library   
*/
#define OPPC_MESSAGE_BASE	0x6700

/*
	Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
	/* Session Control */
	OPPC_INIT_CFM = OPPC_MESSAGE_BASE,
	OPPC_CONNECT_CFM,
	OPPC_AUTH_REQUEST_IND,
	OPPC_DISCONNECT_IND,
	
	OPPC_PUSH_COMPLETE_IND,
	OPPC_PUSH_MORE_DATA_IND,
	
	OPPC_PULL_BC_START_IND,
	OPPC_PULL_BC_DATA_IND,
	OPPC_PULL_BC_COMPLETE_IND,

	OPPC_MESSAGE_TOP
} OppcMessageId;
#endif


/*!
	@brief This message returns the result of an OppcInit attempt.
*/
typedef struct
{
	/*! OPPC Session Handle.*/
	OPPC            *oppc;		
	/*! The current status of the OPPC library. */
    oppc_lib_status status;			
} OPPC_INIT_CFM_T;


/*!
	@brief This message returns the result of an OppcConnect attempt.
*/
typedef struct
{
	/*! OPPC Session Handle.*/
	OPPC            *oppc;	    
	/*! The current status of the OPPC library. */
    oppc_lib_status status;	        
	/*! Maximum size of packet transferable during this session. Equals
	  min(client_packet_size , server_packet_size). */
	uint16          packetSize;		
} OPPC_CONNECT_CFM_T;

/*!
	@brief This message is received when the remote server requests authentication during connection.
*/
typedef struct
{
	/*! OPPC Session Handle.*/
	OPPC            *oppc;	    
	uint8 nonce[GOEP_SIZE_DIGEST];
	uint8 options;
	uint16 size_realm;
	uint8 realm[1];
} OPPC_AUTH_REQUEST_IND_T;

/*!
	@brief This message returns the result of an OppcDisconnect attempt.
*/
typedef struct
{
	OPPC            *oppc;	/*!< OPPC Session Handle.*/
    oppc_lib_status status;	    /*!< The current status of the OPPC library. */
} OPPC_DISCONNECT_IND_T;


/*!
	@brief This message returns when the push operation is complete.
*/
typedef struct
{
	OPPC            *oppc;	/*!< OPPC Session Handle.*/
    oppc_lib_status status;		/*!< The current status of the OPPC library. */
} OPPC_PUSH_COMPLETE_IND_T;


/*!
	@brief This message returns when the push operation is ready for more data.
*/
typedef struct
{
	OPPC    *oppc;			/*!< OPPC Session Handle.*/
} OPPC_PUSH_MORE_DATA_IND_T;


/*!
	@brief This message returns when a pull business card operation has
	started.
*/
typedef struct
{
	/*! OPPC Session Handle.*/
	OPPC    *oppc;		
	/*! Source containing the data. */
    Source  src;			
	/*! Total object size if known. */
	uint16  objectSize;	    
	/*! Length of name. */
	uint16  nameLen;		
	/*! Offset, into source, to the name. */
	uint16  nameOffset;	    
	/*! Length of the packet.*/
	uint16  packetLen;		
	/*! Offset into the stream of the packet start.*/
	uint16  packetOffset;	
	/*! Is this is the last packet? Yes(TRUE) or No(FALSE).*/
    bool    moreData;		
} OPPC_PULL_BC_START_IND_T;


/*! 
	@brief This message returns when pulled business card data has arrived.
*/
typedef struct
{
	/*! OPPC Session Handle.*/
	OPPC    *oppc;		
	/*! Source containing the data.*/
    Source  src;			
	/*! Length of the packet.*/
	uint16  packetLen;		
	/*! Offset into the stream of the packet start.*/
	uint16  packetOffset;	
	/*! Is this is the last packet? Yes(TRUE) or No(FALSE).*/
    bool    moreData;		
} OPPC_PULL_BC_DATA_IND_T;


/*!
	@brief This message returns when pull business card operation is complete.
*/
typedef struct
{
	OPPC            *oppc;	/*!< OPPC Session Handle.*/
    oppc_lib_status status;	    /*!< The current status of the OPPC library. */
} OPPC_PULL_BC_COMPLETE_IND_T;



/*   Downstream API for the OPPC Library   */

/*!
	@brief Initialise the OPPC Library.

	@param theAppTask The current application task.

	@param priority The profile instance low power mode priority. For a set of
	profile instances with connections on a given ACL the instance with the
	highest priority value wins and the low power mode on the ACL is set
	according to it's power table.
	
	OPPC_INIT_CFM message will be received by the application.
*/
void OppcInit(Task theAppTask, uint16 priority);


/*!
	@brief Open an OPPC Connection with a server.

	@param oppc OPPC Session Handle.

	@param bd_addr The Bluetooth address of the device being replied to.

	@param maxPacketSize Maximum packet size supported by this client.
	
	This will make a Bluetooth connection with the server.  OPPC_CONNECT_CFM
	message will be received by the application.
*/
void OppcConnect(OPPC *oppc, const bdaddr *bdAddr, const uint16 maxPacketSize);


/*
	@brief Respond to an authentication challenge during connect.
	@param oppc OPPC Session Handle.
	@param digest MD5 Digest response. Must be a 16byte string.
	@param size_userid Length of the optional User Id string.
	@param userid Option User Id string.
	@param nonce Optional MD5 Digest Nonce as sent in the Challenge. Must be a 16byte string or NULL.
	
	This function can be called by a OPP Client on receipt of a OPPC_AUTH_REQUEST_IND message to reply
	to an authentication challenge.
	A OPPC_CONNECT_CFM message will be received to indicate that the connection process
	has completed.
*/
void oppcConnectAuthResponse(OPPC *oppc, const uint8 *digest, uint16 size_userid, const uint8 *userid, const uint8 *nonce);

/*!
	@brief Disconnect from an server.  

	@param oppc OPPC Session Handle.
	
	OPPC_DISCONNECT_CFM message will be received by the application. 
*/
void OppcDisconnect(OPPC *oppc);


/*!
	@brief Abort the current multi-packet operation.

	@param oppc OPPC Session Handle.

	@param size_description Length of the description parameter.

	@param description This is an optional description of why the transfer was
	aborted.
	
	The relevant COMPLETE_IND message will be received with the status code of ftpc_aborted on success.
*/
void OppcAbort(OPPC *oppc, const uint16 size_description, const uint8* description);


/*!
	@brief Push the first packet of an object to the server.

	@param oppc OPPC Session Handle.
	@param size_name Length of the name field.
	@param name The object name.
	@param size_type Length of the type field.
	@param type The object type.
	@param size_packet Length of the packet supplied with this request.
	@param packet First packet to send.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).

	The ownership of all pointers remains with the caller.
	
	A OPPC_PUSH_MORE_DATA_IND message will be received for each packet.
	A OPPC_PUSH_COMPLETE_IND message will be received after the last packet.
*/
void OppcPushObject(OPPC *oppc, const uint16 size_name, const uint8* name, const uint16 size_type, const uint8* type, const uint16 size_packet, const uint8 *packet, const uint32 totalLen, const bool onlyPacket);


/*!
	@brief Push the next packet of the current object to the server.

	@param oppc OPPC Session Handle.
	@param size_packet Length of the packet supplied with this request.
	@param packet Packet to send.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).

	The ownership of the pointer 'packet' remains with the caller.
	
	A OPPC_PUSH_MORE_DATA_IND message will be received for each packet.
	A OPPC_PUSH_COMPLETE_IND message will be received after the last packet.
*/
void OppcPushNextPacket(OPPC *oppc, const uint16 size_packet, const uint8 *packet, const bool lastPacket);


/*!
	@brief Push the first packet of an object to the server.

	@param oppc OPPC Session Handle.
	@param size_name Length of the name field.
	@param name The object name.
	@param size_type Length of the type field.
	@param type The object type.
	@param size_packet Length of the packet supplied with this request.
	@param src Source containing the packet to send.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).

	The ownership of all pointers remains with the caller.
	
	A OPPC_PUSH_MORE_DATA_IND message will be received for each packet.
	A OPPC_PUSH_COMPLETE_IND message will be received after the last packet.
*/
void OppcPushObjectSource(OPPC *oppc, const uint16 size_name, const uint8* name, const uint16 size_type, const uint8* type, const uint16 size_packet, Source src, const uint32 totalLen, const bool onlyPacket);


/*!
	@brief Push the next packet of the current object to the server.

	@param oppc OPPC Session Handle.
	@param size_packet Length of the packet supplied with this request.
	@param src Source containing the packet to send.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
	
	A OPPC_PUSH_MORE_DATA_IND message will be received for each packet.
	A OPPC_PUSH_COMPLETE_IND message will be received after the last packet.
*/
void OppcPushNextPacketSource(OPPC *oppc, const uint16 size_packet, Source src, const bool lastPacket);


/*!
	@brief Push the first packet of a vCard (business card) to the server.

	@param oppc OPPC Session Handle.
	@param size_name Length of the name field.
	@param name The object name.
	@param size_packet Length of the packet supplied with this request.
	@param packet First packet to send.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).

	The ownership of all pointers remains with the caller.
	
	A OPPC_PUSH_MORE_DATA_IND message will be received for each packet.
	A OPPC_PUSH_COMPLETE_IND message will be received after the last packet.
*/
void OppcPushBusinessCard(OPPC *oppc, const uint16 size_name, const uint8* name, const uint16 size_packet, const uint8 *packet, const uint32 totalLen, const bool onlyPacket);


/*!
	@brief Push the first packet of a vCard (business card) to the server.

	@param oppc OPPC Session Handle.
	@param size_name Length of the name field.
	@param name The object name.
	@param size_packet Length of the packet supplied with this request.
	@param src Source containing the packet to send.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).

	The ownership of all pointers remains with the caller.
	
	A OPPC_PUSH_MORE_DATA_IND message will be received for each packet.
	A OPPC_PUSH_COMPLETE_IND message will be received after the last packet.
*/
void OppcPushBusinessCardSource(OPPC *oppc, const uint16 size_name, const uint8* name, const uint16 size_packet, Source src, const uint32 totalLen, const bool onlyPacket);


/*!
	@brief Request the owner's business card from the OPP Server.

	@param oppc OPPC Session Handle.
	
	A OPPC_PULL_BC_START_IND message will be received for the first packet.
	A OPPC_PULL_BC_DATA_IND message will be received for each packet after the first.
	A OPPC_PULL_BC_COMPLETE_IND message will be received after the last packet.
*/
void OppcPullBusinessCard(OPPC *oppc);


/*!
	@brief Pull the next packet of the current object from the server.

	@param oppc OPPC Session Handle.
	
	A OPPC_PULL_BC_DATA_IND message will be received for each packet after the first.
	A OPPC_PULL_BC_COMPLETE_IND message will be received after the last packet.
*/
void OppcPullNextBcPacket(OPPC *oppc);


/*!
	@brief The packet received has been processed and is no longer needed.
	@param oppc OPPC Session Handle (as returned in OPPC_INIT_CFM).
	
	Every packet send to the client that contains a source must be declared
	complete before the next function is called.  e.g. When a
	OPPC_PULL_BC_START_IND has been received, OppcPacketComplete must be called
	before calling OppcPullNextBcPacket.
	
	No message is received on completion.
*/
void OppcPacketComplete(OPPC *oppc);


#endif /* OPPC_H_ */
