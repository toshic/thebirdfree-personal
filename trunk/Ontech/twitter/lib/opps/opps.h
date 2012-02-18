/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    opps.h
    
DESCRIPTION
	Interface definition for the OPP Server library

*/
/*!
@file	opps.h
@brief	Interface to the OPP Server library


Library Dependecies : connection,region,service, goep, bdaddr

Library variants:-
		opps - opps with no debug
		opps_debug - opps with debug checks
		opps_debug_print - opps with debug checks and print output
*/

#ifndef	OPPS_H_
#define	OPPS_H_

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
	opps_success,		
	/*! Last operation failed.*/
    opps_failure,		
	/*! Could not open a session due to having too many sessions open. */
	opps_maxsessions,   
	/*! A command has been attempted while another command is already
	  running. */
    opps_not_idle,      
	/*! A command has been attempted while a different multi-packet command is
	  running. */
    opps_wrong_command, 
	/*! Operation failed due to being in the wrong state.*/
	opps_wrong_state,
	opps_invalid_sdp,
	
	opps_badrequest,
	opps_forbidden,
	opps_notfound,
	
	opps_aborted,
	
	opps_end_of_status_list
} opps_lib_status;

/*! 
	@brief Handle of the current OPP Server Session.
   
	For client-side session management, this value can be used in comparisons,
	but it's meaning cannot be relied upon.
 */
struct __oppsState;
typedef struct __oppsState OPPS; 

/*
   @brief Upstream Messages for the OPP Client Library   
*/
#define OPPS_MESSAGE_BASE	0x6600

/*
	Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
	/* Session Control */
	OPPS_INIT_CFM = OPPS_MESSAGE_BASE,
	OPPS_CONNECT_IND,
	OPPS_CONNECT_CFM,
	OPPS_AUTH_RESULT_IND,
	OPPS_DISCONNECT_IND,
	
	/* Object transfer */
	OPPS_PUSH_OBJ_START_IND,
	OPPS_PUSH_OBJ_DATA_IND,
	OPPS_PUSH_OBJ_COMPLETE_IND,
	OPPS_PUSH_BC_START_IND,
	OPPS_PUSH_BC_DATA_IND,
	OPPS_PUSH_BC_COMPLETE_IND,
	
	/* There is no data associated with this message */
	OPPS_PULL_BC_START_IND, 
	/* There is no data associated with this message */
	OPPS_PULL_BC_MOREDATA_IND, 
	OPPS_PULL_BC_COMPLETE_IND,

	OPPS_MESSAGE_TOP
} OppsMessageId;
#endif


/*!
	@brief This message returns the result of an OppsInit attempt.
*/
typedef struct
{
	/*! OPPS Session Handle.*/
	OPPS            *opps;	    
	/*! The current status of the OPPS library. */
    opps_lib_status status;			
} OPPS_INIT_CFM_T;


/*!
	@brief This message is sent when an remote client attempts to make a
	connection.
*/
typedef struct
{
	/*! OPPS Session Handle.*/
	OPPS    *opps;	    
	/*! Bluetooth address of the remote client. */
	bdaddr  bd_addr;		
	/*! Maximum size of packet supported by this GOEP connection */
	uint16  maxPacketLen;	
} OPPS_CONNECT_IND_T;


/*!
	@brief This message is sent when a remote connection completes.
*/
typedef struct
{
	/*! OPPS Session Handle.*/
	OPPS            *opps;	        
	/*! The current status of the OPPS library. */
    opps_lib_status status;		        
	/*! Maximum size of packet supported by this GOEP connection */
	uint16          maxPacketLen;		
} OPPS_CONNECT_CFM_T;

/*!
     @brief This message is sent when an authentication challenge completes during a connect.
*/
typedef struct
{
	/*! OPPS Session Handle.*/
	OPPS *opps;
	uint8 digest[GOEP_SIZE_DIGEST];
	uint8 nonce[GOEP_SIZE_DIGEST];
	uint16 size_userid;
	uint8 userid[1];
} OPPS_AUTH_RESULT_IND_T;

/*!
	@brief This message indicates that a disconnect operation has completed.
*/
typedef struct
{
	OPPS    *opps;	    /*!< OPPS Session Handle.*/
} OPPS_DISCONNECT_IND_T;


/*!
	@brief This message is sent when a remote object push starts.
*/
typedef struct 
{
	/*! OPPS Session Handle.*/
	OPPS    *opps;	    
	/*! Source containing the data. */
    Source  src;			
	/*! Offset to name. */
    uint16  nameOffset;		
	/*! Length of name. */
    uint16  nameLength;		
	/*! Offset to type. */
    uint16  typeOffset;		
	/*! Length of type. */
    uint16  typeLength;		
	/*! Total length of the object. */
    uint32  totalLength;	
	/*! Length of the packet.*/
	uint16  packetLen;		
	/*! Offset into the stream of the packet start.*/
	uint16  packetOffset;	
	/*! Is this is the last packet? Yes(TRUE) or No(FALSE).*/
    bool    moreData;		
} OPPS_PUSH_OBJ_START_IND_T;


/*!
	@brief This message is sent when a remote business card push starts.
*/
typedef struct 
{
	/*! OPPS Session Handle.*/
	OPPS    *opps;	    
	/*! Source containing the data. */
    Source  src;			
	/*! Offset to name. */
    uint16  nameOffset;		
	/*! Length of name. */
    uint16  nameLength;		
	/*! Total length of the object. */
    uint32  totalLength;	
	/*! Length of the packet.*/
	uint16  packetLen;		
	/*! Offset into the stream of the packet start.*/
	uint16  packetOffset;	
	/*! Is this is the last packet? Yes(TRUE) or No(FALSE).*/
    bool    moreData;		
} OPPS_PUSH_BC_START_IND_T;


/*!
	@brief This message is sent when a remote object push completes.
*/
typedef struct 
{
	OPPS            *opps;	/*!< OPPS Session Handle.*/
    opps_lib_status status;	    /*!< The current status of the OPPS library. */
} OPPS_PUSH_OBJ_COMPLETE_IND_T;


/*!
	@brief This message is sent when a remote object push completes.
*/
typedef struct 
{
	OPPS            *opps;	/*!< OPPS Session Handle.*/
    opps_lib_status status; 	/*!< The current status of the OPPS library. */
} OPPS_PUSH_BC_COMPLETE_IND_T;


/*! 
	@brief This message is sent when data associated with a remote object push
	arrives.
*/
typedef struct 
{
	/*! OPPS Session Handle.*/
	OPPS    *opps;	    
	/*! Source containing the data.*/
    Source  src;			
	/*! Length of the packet.*/
	uint16  packetLen;		
	/*! Offset into the stream of the packet start.*/
	uint16  packetOffset;	
	/*! Is this is the last packet? Yes(TRUE) or No(FALSE).*/
    bool    moreData;		
} OPPS_PUSH_OBJ_DATA_IND_T;


/*! 
	@brief This message is sent when data associated with a remote business
	card push arrives.
*/
typedef struct 
{
	/*! OPPS Session Handle.*/
	OPPS    *opps;	    
	/*! Source containing the data.*/
    Source  src;			
	/*! Length of the packet.*/
	uint16  packetLen;		
	/*! Offset into the stream of the packet start.*/
	uint16  packetOffset;	
	/*! Is this is the last packet? Yes(TRUE) or No(FALSE).*/
    bool    moreData;		
} OPPS_PUSH_BC_DATA_IND_T;


/*!
	@brief This message indicates that a pull business card operation has
	started.
*/
typedef struct
{
	OPPS    *opps;   	/*!< OPPS Session Handle.*/
} OPPS_PULL_BC_START_IND_T;


/*!
	@brief This message indicates that a pull business card operation needs
	more data.
*/
typedef struct
{
	OPPS    *opps;   	/*!< OPPS Session Handle.*/
} OPPS_PULL_BC_MOREDATA_IND_T;


/*!
	@brief This message is sent when a PULL Business Card request completes.
*/
typedef struct
{
	OPPS            *opps;	/*!< OPPS Session Handle.*/
    opps_lib_status status;		/*!< The current status of the OPPS library. */
} OPPS_PULL_BC_COMPLETE_IND_T;




/*   Downstream API for the OPPS Library   */

/*!
	@brief Initialise the OPPS Library.

	@param theAppTask The current application task.

	@param priority The profile instance low power mode priority. For a set of
	profile instances with connections on a given ACL the instance with the
	highest priority value wins and the low power mode on the ACL is set
	according to it's power table.
	
	OPPS_INIT_CFM message will be received by the application. 
*/
void OppsInit(Task theAppTask, uint16 priority);


/*!
	@brief Accept or refuse a connection.  

	@param opps OPPS Session Handle.
	@param accept TRUE to accept the connection, or FALSE to refuse it.
	@param pktSize Maximum packet size that can be accepted on this connection.
	
	This function should be called on recept of a OPPS_CONNECT_IND message.
	A OPPS_CONNECT_CFM message will be received on completion.
*/
void OppsConnectResponse(OPPS *opps, bool accept, uint16 pktSize);

/*
	@brief Start an authentication challenge during connect.
    @param opps OPPS Session Handle.
	@param nonce MD5 Digest Nonce to use. Must be a 16byte string.
	@param options Optional options to use.
	@param size_realm Length of the optional Realm string.
	@param realm Option realm string.
	
	This function can be called by a OPP Server on receipt of a OPPS_CONNECT_IND message to start
	and authentication challenge.
	A OPPS_AUTH_RESULT_IND message will be received to indicate the result of the authentication challenge.
*/
void OppsConnectAuthChallenge(OPPS *opps, const uint8 *nonce, uint8 options, uint16 size_realm, const uint8 *realm);

/*!
	@brief Abort the current multi-packet operation.

	@param opps OPPS Session Handle.
	
	The relevant COMPLETE_IND message will be received with the status code of opps_aborted on success.
*/
void OppsAbort(OPPS *opps);

/*!
	@brief Request the next packet during a Remote PUT operation.  

	@param opps OPPS Session Handle.

	@param moreData TRUE if expecting more data, else False not expecting more
	data.
	
	This function should be called on receipt of a OPPS_PUSH_OBJ_START_IND, OPPS_PUSH_OBJ_DATA_IND,
	OPPS_PUSH_BC_START_IND or a OPPS_PUSH_BC_DATA_IND message.
	A OPPS_PUSH_OBJ_DATA_IND or OPPS_PUSH_BC_DATA_IND message is recieved following each packet after
	the first.
	A OPPS_PUSH_OBJ_COMPLETE_IND or OPPS_PUSH_BC_COMPLETE_IND message is recieved following the 
	last packet.
*/
void OppsGetNextPutPacket(OPPS *opps, bool moreData);

/*!
	@brief Push the first packet of a vCard (business card) to the server.

	@param opps OPPS Session Handle.
	@param size_name Length of the name field.
	@param name The object name.
	@param size_packet Length of the packet supplied with this request.
	@param packet First packet to send.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).

	The ownership of all pointers remains with the caller.
	
	If data is in a VM Source (e.g. from the filesystem),
	OppsPushBusinessCardStartSource should be used instead of
	OppsPushBusinessCardStart.
	
	This function should be called on receipt of a OPPS_PULL_BC_START_IND message.
	A OPPS_PULL_BC_DATA_IND message will be received to request the next packet.
	A OPPS_PULL_BC_COMPLETE_IND message will be received after sending the final packet.
*/
void OppsPushBusinessCardStart(OPPS *opps, const uint16 size_name, const uint8* name, const uint16 size_packet, const uint8 *packet, const uint32 totalLen, const bool onlyPacket);


/*!
	@brief Push the next packet of the current vCard to the server.

	@param opps OPPS Session Handle.
	@param size_packet Length of the packet supplied with this request.
	@param packet Packet to send.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).

	The ownership of the pointer 'packet' remains with the caller.
	
	If data is in a VM Source (e.g. from the filesystem),
	OppsPushBusinessCardNextSource should be used instead of
	OppsPushBusinessCardNext.
	
	This function should be called on receipt of a OPPS_PULL_BC_DATA_IND message.
	A OPPS_PULL_BC_DATA_IND message will be received to request the next packet.
	A OPPS_PULL_BC_COMPLETE_IND message will be received after sending the final packet.
*/
void OppsPushBusinessCardNext(OPPS *opps, const uint16 size_packet, const uint8 *packet, const bool lastPacket);


/*!
	@brief Push the first packet of a vCard (business card) to the server.

	@param opps OPPS Session Handle.
	@param size_name Length of the name field.
	@param name The object name.
	@param size_packet Length of the packet supplied with this request.
	@param src Source containing the data packet.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).
	
	This function should be called on receipt of a OPPS_PULL_BC_START_IND message.
	A OPPS_PULL_BC_DATA_IND message will be received to request the next packet.
	A OPPS_PULL_BC_COMPLETE_IND message will be received after sending the final packet.
*/
void OppsPushBusinessCardStartSource(OPPS *opps, const uint16 size_name, const uint8* name, const uint16 size_packet, Source src, const uint32 totalLen, const bool onlyPacket);


/*!
	@brief Push the next packet of the current vCard to the server.

	@param opps OPPS Session Handle.
	@param size_packet Length of the packet supplied with this request.
	@param src Source containing the data packet.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
	
	This function should be called on receipt of a OPPS_PULL_BC_DATA_IND message.
	A OPPS_PULL_BC_DATA_IND message will be received to request the next packet.
	A OPPS_PULL_BC_COMPLETE_IND message will be received after sending the final packet.
*/
void OppsPushBusinessCardNextSource(OPPS *opps, const uint16 size_packet, Source src, const bool lastPacket);


/*!
	@brief The packet received has been processed and is no longer needed.

	@param opps OPPS Session Handle (as returned in OPPS_INIT_CFM).
	
	Every packet send to the client that contains a source must be declared
	complete before the next function is called.  e.g. When a
	OPPS_PUSH_OBJ_START_IND has been received, OppsPacketComplete must be
	called before calling OppsGetNextPutPacket.
	
	No message is received on completion.
*/
void OppsPacketComplete(OPPS *opps);


#endif /* OPPS_H_ */
