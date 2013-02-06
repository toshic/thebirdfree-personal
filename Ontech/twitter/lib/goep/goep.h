/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep.h
    
DESCRIPTION
	Interface definition for the Generic Object Exchange Profile (GOEP) library

	This is the base profile for FTP Server, FTP Client and the Object Push
	Profile (OPP) Libraries.
*/
/*!
@file	goep.h
@brief	Interface to the Generic Object Exchange Profile (GOEP) library.

		This is the base profile for FTP Server, FTP Client and the Object Push
		Profile (OPP) Libraries.

Library Dependecies : connection,region,service, bdaddr
		
	Library variants:-
		goep - GOEP with no debug output
		goep_print - GOEP with debug output
		goep_print_decode - GOEP with debug output and packet decoding
*/

#ifndef	GOEP_H_
#define	GOEP_H_

#include <bdaddr_.h>
#include <message.h>
#include <source.h>

/*!
	@brief Size of a MD5 Digest String
*/
#define GOEP_SIZE_DIGEST 16

/*!
	@brief Maximum size of a MD5 String.
	Used for the Realm string in the request and the UserID in the response.
	Longer realms will be truncated to this.
*/
#define GOEP_SIZE_MD5_STRING 32

/*! 
	@brief The GOEP service role. 
*/
typedef enum
{
	goep_Client,
	goep_Server
} goep_serv_role;

/*!
	@brief The GOEP service classes. 
*/
typedef enum
{
	goep_OBEX  		= 0x1105,
	goep_FTP   		= 0x1106,
	goep_PBAP_PCE   = 0x112e,
	goep_PBAP_PSE   = 0x112f,
	goep_SYNC_PSE   = 0x1104
} goep_serv_class;


/*! \name Set Path flag bits
*/
/*! \{ */
/*!
	@brief Instruct the GOEP library to change folder to the parent of the
	current folder.
*/
#define GOEP_PATH_PARENT	(1<<0)
/*!
	@brief Don't create a folder if the required SETPATH folder does not exist.
*/
#define GOEP_PATH_NOCREATE	(1<<1)
/*! \} */


/*!
	@brief The GOEP status. 
*/
typedef enum 
{
	/*! Last operation was successful.*/
	goep_success,			   
	/*! Last operation failed.*/
	goep_failure,			   
	/*! Client is the wrong state for this command. */
    goep_invalid_command,      
	/*! Parameters sent in the command are invalid. */
    goep_invalid_parameters,   
	/*! Connection has failed due to being unauthorised. */
    goep_connect_unauthorised,           
	/*! Remote Host has aborted our current multi-packet command. */
    goep_host_abort,           
	/*! Local Client has aborted our current multi-packet command. */
    goep_local_abort,          
	/*! Remote Client has disconnected */
	goep_remote_disconnect,	   
	/*! Server doesn't support this type of GOEP. */
	goep_server_unsupported,   
	/*! Server has accepted the connection on a service that wasn't
	  requested. */
	goep_server_invalid_serv,  
	/*! Server has refused the connection */
    goep_connection_refused,   
    /*! Folder not found and GOEP_PATH_NOCREATE specified */
    goep_setpath_notfound,     
	/*! Not authorised to access the requested folder */
    goep_setpath_unauthorised, 
    /*! Server couldn't understand the request. */
    goep_get_badrequest,       
	/*! Server reported that the request was understood but forbidden. */
    goep_get_forbidden,        
	/*! Requested file was not found. */
    goep_get_notfound,         	
	goep_transport_failure,
	/*! Connect was cancelled by the client during authentication */
	goep_connect_cancelled,
	goep_end_of_status_list
} goep_lib_status;

/*!
	@brief Response codes sent back to a remote client from a local server
*/
typedef enum
{
	goep_svr_resp_OK,
	goep_svr_resp_Continue,
			
	goep_svr_resp_BadRequest,
	goep_svr_resp_Forbidden,
	goep_svr_resp_Unauthorized,
	goep_svr_resp_NotFound,
	/*! Pre condition failed */
	goep_svr_resp_PreConFail,
	goep_svr_resp_ServUnavailable,
	
	goep_svr_resp_end_of_list
} goep_svr_resp_codes;

/*!
	@brief Handle of the current GOEP Client Session.

   	For client-side session management, this value can be used in comparisons,
   	but it's meaning cannot be relied upon.
 */
struct __goepState;
typedef struct __goepState GOEP;

/*!
	@brief Upstream Messages for the GOEP Library.   
*/
#define GOEP_MESSAGE_BASE	0x6a00

/*
	Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
	/* Session Control */
	GOEP_INIT_CFM = GOEP_MESSAGE_BASE,
    GOEP_CHANNEL_IND,
	
	GOEP_CONNECT_CFM,		/*!< Connection with server complete */
	GOEP_CONNECT_IND,
	GOEP_AUTH_REQUEST_IND, /*!< Remote Server has requested authentication */
	GOEP_AUTH_RESULT_IND,  /*!< Remote Client has replied to an authentication request */
	GOEP_DISCONNECT_IND,
	
	GOEP_SET_PATH_CFM,
	GOEP_SET_PATH_IND,
	
	GOEP_DELETE_CFM,
	GOEP_DELETE_IND,
	
	/* Local initiated messages */
    /* Local end doing a PUT - Sending Data */
    GOEP_LOCAL_PUT_DATA_REQUEST_IND,
    GOEP_LOCAL_PUT_COMPLETE_IND,

	/* Local end doing a GET - requesting data */
	GOEP_LOCAL_GET_START_IND,
    GOEP_LOCAL_GET_DATA_IND,
    GOEP_LOCAL_GET_COMPLETE_IND,

	/* Remote initiated messages */
    /* Remote end doing a PUT - Sending data to us */
    GOEP_REMOTE_PUT_START_IND,
    GOEP_REMOTE_PUT_DATA_IND,
    GOEP_REMOTE_PUT_COMPLETE_IND,
	
    /* Remote end doing a GET - requesting data from us */
    GOEP_REMOTE_GET_START_IND,
    GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND,
    GOEP_REMOTE_GET_COMPLETE_IND,


	/* Application want to use App. specific headers, request them */
	GOEP_GET_APP_HEADERS_IND,
	/* Local Get has started which contains App. Specific Headers */
	GOEP_LOCAL_GET_START_HDRS_IND,
	/* Remote Get has started which contains App. Specific Headers */
    GOEP_REMOTE_GET_START_HDRS_IND,

    GOEP_END_OF_LIST,
    GOEP_MESSAGE_TOP = GOEP_END_OF_LIST    
} GoepMessageId;
#endif


/*!
	@brief This message returns the result of a geopInit attempt.
*/
typedef struct
{
	GOEP            *goep;	/*!< Handle to GOEP session of this client. */
    goep_lib_status status;		/*!< The current status of the GOEP library. */
} GOEP_INIT_CFM_T;


/*!
	@brief This message returns the result of a GoepGetChannel call.
*/
typedef struct
{
	/*! Handle to GOEP session of this client. */
	GOEP            *goep;	
	/*! The current status of the GOEP library. */
    goep_lib_status status;		
	/*! RFCOMM channel to use for this GOEP Session. */
    uint8           channel;    
} GOEP_CHANNEL_IND_T;


/*!
	@brief This message indicated that a remote GOEP client is attempting to
	connect to a server.
	
	The maximum length of the target is 32 bytes.
*/
typedef struct
{
	/*! Handle to GOEP session of this client. */
	GOEP    *goep;		
	/*! Bluetooth address of the remote client.  Ownership passes to the
	  client. */
	bdaddr  bd_addr;		
	/*! Maximum size of packet supported by this GOEP connection */
	uint16  maxPacketLen;	
	/*! Length of the target header if present */
	uint16  size_target;	
	/*! Contents of the target header if present. The client should not attempt
	  to free this pointer, the memory will be freed when the message is
	  destroyed. If the client needs access to this data after the message has
	  been destroyed it is the client's responsibility to copy it. */
	uint8   target[1];		
} GOEP_CONNECT_IND_T;


/*!
	@brief This message returns the result of a GoepConnect attempt.
*/
typedef struct
{
	/*! Handle to GOEP session of this client. */
	GOEP            *goep;	    
	/*! The current status of the GOEP library. */
    goep_lib_status status;	        
	/*! Maximum size of packet supported by this GOEP connection */
    uint16          maxPacketLen;	
} GOEP_CONNECT_CFM_T;


/*!
	@brief This message is received when the remote server requests authentication during connection.
*/
typedef struct
{
	/*! Handle to GOEP session of this client. */
	GOEP *goep;
	uint8 nonce[GOEP_SIZE_DIGEST];
	uint8 options;
	uint16 size_realm;
	uint8 realm[1];
} GOEP_AUTH_REQUEST_IND_T;

/*!
	@brief This message is received when the remote client returns the result of authentication during connection.
*/
typedef struct
{
	/*! Handle to GOEP session of this client. */
	GOEP *goep;	    
	uint8 request[GOEP_SIZE_DIGEST];
	uint8 nonce[GOEP_SIZE_DIGEST];
	uint16 size_userid;
	uint8 *userid;
} GOEP_AUTH_RESULT_IND_T;
/*!
	@brief This message returns the result of a GoepDisconnect attempt.
*/
typedef struct
{
	GOEP            *goep;	/*!< Handle to GOEP session of this client. */
    goep_lib_status status;		/*!< The current status of the GOEP library. */
} GOEP_DISCONNECT_IND_T;


/*!
	@brief This message is received when the final packet has been sent.
*/
typedef struct
{
	GOEP            *goep;	/*!< Handle to GOEP session of this client. */
    goep_lib_status status;		/*!< The current status of the GOEP library. */
} GOEP_LOCAL_PUT_COMPLETE_IND_T;			


/*!
	@brief This message is received when more data has been requested.
*/
typedef struct
{
	GOEP    *goep;	/*!< Handle to GOEP session of this client. */
} GOEP_LOCAL_PUT_DATA_REQUEST_IND_T;			


/*!
	@brief This message is receieved when a GET request starts.
*/
typedef struct 
{
	GOEP    *goep;	    /*!< Handle to GOEP session of this client. */
    Source  src;			/*!< Source. */
    uint16  nameOffset;		/*!< Offset to name. */
    uint16  nameLength;		/*!< Length of name. */
    uint16  typeOffset;		/*!< Offset to type. */
    uint16  typeLength;		/*!< Length of type. */
    uint32  totalLength;	/*!< Total length of the object. */
    uint16  dataOffset;		/*!< Offset to data. */
    uint16  dataLength;		/*!< Length of data. */
    bool    moreData;		/*!< More data? Yes(TRUE) or No(FALSE). */
} GOEP_LOCAL_GET_START_IND_T;


/*!
	@brief This message is received when more of the data requested has
	arrived.
*/
typedef struct 
{
	GOEP    *goep;	    /*!< Handle to GOEP session of this client. */
    Source  src;			/*!< Source. */
    uint16  dataOffset;		/*!< Offset to data. */
    uint16  dataLength;		/*!< Length of data. */
    bool    moreData;		/*!< More data? Yes(TRUE) or No(FALSE). */
} GOEP_LOCAL_GET_DATA_IND_T;


/*!
	@brief This message is received when a GET requested has completed.
*/
typedef struct 
{
	GOEP            *goep;	/*!< Handle to GOEP session of this client. */
    goep_lib_status status; 	/*!< The current status of the GOEP library. */
} GOEP_LOCAL_GET_COMPLETE_IND_T;


/*!
	@brief This message is receieved when a Remote PUT request starts.
*/
typedef struct
{
	GOEP    *goep;	    /*!< Handle to GOEP session of this client. */
    Source  src;			/*!< Source. */
    uint16  nameOffset;		/*!< Offset to name. */
    uint16  nameLength;		/*!< Length of name. */
    uint16  typeOffset;		/*!< Offset to type. */
    uint16  typeLength;		/*!< Length of type. */
    uint32  totalLength;	/*!< Total length of the object. */
    uint16  dataOffset;		/*!< Offset to data. */
    uint16  dataLength;		/*!< Length of data. */
    bool    moreData;		/*!< More data? Yes(TRUE) or No(FALSE). */
} GOEP_REMOTE_PUT_START_IND_T;


/*!
	@brief This message is received when more of the data requested has
	arrived.
*/
typedef struct
{
	GOEP    *goep;	    /*!< Handle to GOEP session of this client. */
    Source  src;			/*!< Source. */
    uint16  dataOffset;		/*!< Offset to data. */
    uint16  dataLength;		/*!< Length of data. */
    bool    moreData;		/*!< More data? Yes(TRUE) or No(FALSE). */
} GOEP_REMOTE_PUT_DATA_IND_T;


/*!
	@brief This message is received when a Remote PUT requested has completed.
*/
typedef struct
{
	GOEP            *goep;	/*!< Handle to GOEP session of this client. */
    goep_lib_status status; 	/*!< The current status of the GOEP library. */
} GOEP_REMOTE_PUT_COMPLETE_IND_T;


/*!
	@brief This message is received when a remote client initiates a GET
	request.
*/
typedef struct
{
	GOEP    *goep;   	/*!< Handle to GOEP session of this client. */
    Source  src;			/*!< Source. */
    uint16  nameOffset;		/*!< Offset to name. */
    uint16  nameLength;		/*!< Length of name. */
    uint16  typeOffset;		/*!< Offset to type. */
    uint16  typeLength;		/*!< Length of type. */
} GOEP_REMOTE_GET_START_IND_T;


/*!
	@brief This message is received when the remote host has requested more
	data.
*/
typedef struct
{
	GOEP    *goep;	/*!< Handle to GOEP session of this client. */
} GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND_T;


/*!
	@brief This message is received when GET completes.
*/
typedef struct
{
	GOEP            *goep;	/*!< Handle to GOEP session of this client. */
    goep_lib_status status; 	/*!< The current status of the GOEP library. */
} GOEP_REMOTE_GET_COMPLETE_IND_T;


/*!
	@brief This message is received when a Delete completes.
*/
typedef struct
{
	GOEP            *goep;	/*!< Handle to GOEP session of this client. */
    goep_lib_status status;	    /*!< The current status of the GOEP library. */
} GOEP_DELETE_CFM_T;


/*!
	@brief This message is receieved when a remote client starts a PUT request
	to delete an object.
*/
typedef struct
{
	GOEP    *goep;   	/*!< Handle to GOEP session of this client. */
    Source  src;			/*!< Source. */
    uint16  nameOffset;		/*!< Offset to name. */
    uint16  nameLength;		/*!< Length of name. */
} GOEP_DELETE_IND_T;


/*!
	@brief This message returns the result of a GoepSetPath attempt.
*/
typedef struct
{
	GOEP            *goep;	/*!< Handle to GOEP session of this client. */
    goep_lib_status status;	    /*!< The current status of the GOEP library. */
} GOEP_SET_PATH_CFM_T;


/*!
	@brief This message is received when a remote client issues a SET PATH
	request.
*/
typedef struct
{
	GOEP    *goep;   	/*!< Handle to GOEP session of this client. */
    Source  src;			/*!< Source. */
	uint8   flags;			/*!< SET PATH flags. */
    uint16  nameOffset;		/*!< Offset to name. */
    uint16  nameLength;		/*!< Length of name. */
} GOEP_SET_PATH_IND_T;


/*!
	@brief This message is received when a remote client issues a SET PATH
	request.
*/
typedef struct
{
	GOEP    *goep;   	/*!< Handle to GOEP session of this client. */
    Sink  	sink;			/*!< Sink to place the headers in. */
    uint16  length;			/*!< Maximum length of the header */
} GOEP_GET_APP_HEADERS_IND_T;

/*!
	@brief This message is receieved when a GET request starts which 
		contains App. Specific Headers.
*/
typedef struct 
{
	GOEP    *goep;	    /*!< Handle to GOEP session of this client. */
    Source  src;			/*!< Source. */
    uint16  nameOffset;		/*!< Offset to name. */
    uint16  nameLength;		/*!< Length of name. */
    uint16  typeOffset;		/*!< Offset to type. */
    uint16  typeLength;		/*!< Length of type. */
    uint32  totalLength;	/*!< Total length of the object. */
    uint16  dataOffset;		/*!< Offset to data. */
    uint16  dataLength;		/*!< Length of data. */
    uint16  headerOffset;	/*!< Offset to Application Specific Headers. */
    uint16  headerLength;	/*!< Length of Application Specific Headers. */
    bool    moreData;		/*!< More data? Yes(TRUE) or No(FALSE). */
} GOEP_LOCAL_GET_START_HDRS_IND_T;

/*!
	@brief This message is received when a remote client initiates a GET
		request which contains App. Specific Headers..
*/
typedef struct
{
	GOEP    *goep;   	/*!< Handle to GOEP session of this client. */
    Source  src;			/*!< Source. */
    uint16  nameOffset;		/*!< Offset to name. */
    uint16  nameLength;		/*!< Length of name. */
    uint16  typeOffset;		/*!< Offset to type. */
    uint16  typeLength;		/*!< Length of type. */
    uint16  headerOffset;	/*!< Offset to Application Specific Headers. */
    uint16  headerLength;	/*!< Length of Application Specific Headers. */
} GOEP_REMOTE_GET_START_HDRS_IND_T;


/*   Downstream API for the GOEP Library   */

/*!
	@brief Initialise the GOEP Library.
	@param theAppTask The current application task.
	@param role Client or Server
	@param servClass Type of GOEP required (e.g. FTP or OPP)

	This should be the first function called.
	
	A GOEP_INIT_CFM message will be received on completion.
*/
void GoepInit(Task theAppTask, const goep_serv_role role, const goep_serv_class servClass);


/*!
	@brief Get an RFCOMM channel to use with this GOEP Instance.
	@param goep GOEP Session Instance.
	
	A GOEP_CHANNEL_IND will be received on completion.
*/
void GoepGetChannel(GOEP *goep);


/*
	@brief Open an GOEP Connection with a server.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param bd_addr The Bluetooth address of the device being replied to.
	@param rfc_channel RFCOMM Server channel to use for the connection
	@param maxPacketSize Maximum packet size supported by this client.
	@param size_target Length of the target parameter.
	@param target GOEP target header.
	
	A GOEP_CONNECT_CFM message will be received to indicate that the connection process
	has completed.
*/
void GoepConnect(GOEP *goep, const bdaddr *bdAddr, uint8 rfc_channel, uint16 maxPacketSize, uint16 size_target, const uint8 *target);

/*
	@brief Start an authentication challenge during connect.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param nonce MD5 Digest Nonce to use
	@param options Optional options to use.
	@param size_realm Length of the optional Realm string.
	@param realm Option realm string.
	
	This function can be called by a GOEP Server on receipt of a GOEP_CONNECT_IND message to start
	and authentication challenge.
	A GOEP_AUTH_RESULT_IND message will be received to indicate the result of the authentication challenge.
*/
void GoepConnectAuthChallenge(GOEP *goep, const uint8 *nonce, uint8 options, uint16 size_realm, const uint8 *realm);

/*
	@brief Start an authentication challenge during connect.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param digest MD5 Digest response. Must be a 16byte string.
	@param size_userid Length of the optional User Id string.
	@param userid Option User Id string.
	@param nonce Optional MD5 Digest Nonce as sent in the Challenge. Must be a 16byte string or NULL.
	
	This function can be called by a GOEP Client on receipt of a GOEP_AUTH_REQUEST_IND message to reply
	to an authentication challenge.
	A GOEP_CONNECT_CFM message will be received to indicate that the connection process
	has completed.
*/
void GoepConnectAuthResponse(GOEP *goep, const uint8 *digest, uint16 size_userid, const uint8 *userid, const uint8 *nonce);

/*
	@brief Respond to a GOEP connection attempt from a client.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param result Response code for the connection.
	@param maxPacketSize Maximum packet size supported by this client.
	
	This function should be called on receipt of a GEOP_CONNECT_IND message.
	A GOEP_CONNECT_CFM message will be received to indicate that the connection process
	has completed.
*/
void GoepConnectResponse(GOEP *goep, goep_svr_resp_codes result, uint16 maxPacketSize);


/*!
	@brief Close an GOEP Connection with a server. 
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	
	All transfer operations associated with this session MUST either be
	completed or aborted before this function is called.  After
	GOEP_DISCONNECT_IND has been received with a 'success' state, the only valid
	function to call is GoepDeRegister or GoepConnect.
	
	A GOEP_DISCONNECT_IND message will be received when the connection has been
	diconnected.
*/
void GoepDisconnect(GOEP *goep);


/*!
	@brief Set the current folder
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param flags GOEP Setpath flags.  (Zero, GOEP_PATH_PARENT or
	GOEP_PATH_NOCREATE)
	@param size_folder Length of the folder name.
	@param folder Folder name.
	
	A GOEP_SET_PATH_CFM message is received on completion.
*/
void GoepSetPath(GOEP *goep, const uint8 flags, uint16 size_folder, const uint8* folder);


/*
	@brief Respond to a Remote Set Path packet from a client.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param result Response code for the connection.
	
	This function should be called on receipt of a GOEP_SET_PATH_IND message from
	a remote client.
*/
void GoepRemoteSetPathResponse(GOEP *goep, goep_svr_resp_codes result);

/*!
	@brief Delete an Object.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_name Length of the object name.
	@param name The object name.
	
	A GOEP_DELETE_CFM message is received on completion.
*/
void GoepDelete(GOEP *goep, uint16 size_name, const uint8* name);


/*!
	@brief Respond to a Delete request from a client.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param result Response code for the connection.
	
	This function should be called on receipt of a GOEP_DELETE_IND message from
	a remote client.
*/
void GoepRemoteDeleteResponse(GOEP *goep, goep_svr_resp_codes result);


/*!
	@brief Put the first packet of an object.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_name Length of the object name.
	@param name The object name.
	@param size_packet Length of the packet supplied with this request.
	@param packet First packet to send.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).
	
	If data is in a VM Source (e.g. from the filesystem),
	GoepLocalPutFirstSource should be used instead of GoepLocalPutFirst.
	
	A GOEP_LOCAL_PUT_DATA_REQUEST_IND message will be received for each packet.
	A GOEP_LOCAL_PUT_COMPLETE_IND message will be received after the last packet.
*/
void GoepLocalPutFirst(GOEP *goep, uint16 size_name, const uint8* name, uint16 size_packet, const uint8 *packet, uint32 totalLen, bool onlyPacket);


/*!
	@brief Put the first packet of an object.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_name Length of the object name.
	@param name The object name.
	@param size_type Length of the object type.
	@param type The object type.
	@param size_packet Length of the packet supplied with this request.
	@param packet First packet to send.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).
	
	If data is in a VM Source (e.g. from the filesystem),
	GoepLocalPutFirstTypeSource should be used instead of
	GoepLocalPutFirstType.
*/
void GoepLocalPutFirstType(GOEP *goep, uint16 size_name, const uint8* name, uint16 size_type, const uint8* type, uint16 size_packet, const uint8 *packet, uint32 totalLen, bool onlyPacket);


/*!
	@brief Put the next packet of a multi-packet object.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_packet Length of packet in this request.
	@param packet Packet to send.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
	
	If data is in a VM Source (e.g. from the filesystem),
	GoepLocalPutNextPacketSource should be used instead of
	GoepLocalPutNextPacket.
	
	A GOEP_LOCAL_PUT_DATA_REQUEST_IND message will be received for each packet.
	A GOEP_LOCAL_PUT_COMPLETE_IND message will be received after the last packet.
*/
void GoepLocalPutNextPacket(GOEP *goep, uint16 size_packet, const uint8 *packet, bool lastPacket);


/*!
	@brief Put the first packet of an object.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_name Length of the object name.
	@param name The object name.
	@param size_packet Length of the packet supplied with this request.
	@param src Source which contains the data.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).
	
	A GOEP_LOCAL_PUT_DATA_REQUEST_IND message will be received for each packet.
	A GOEP_LOCAL_PUT_COMPLETE_IND message will be received after the last packet.
*/
void GoepLocalPutFirstSource(GOEP *goep, uint16 size_name, const uint8* name, uint16 size_packet, Source src, uint32 totalLen, bool onlyPacket);


/*!
	@brief Put the first packet of an object.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_name Length of the object name.
	@param name The object name.
	@param size_type Length of the object type.
	@param type The object type.
	@param size_packet Length of the packet supplied with this request.
	@param src Source which contains the data.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).
	
	A GOEP_LOCAL_PUT_DATA_REQUEST_IND message will be received for each packet.
	A GOEP_LOCAL_PUT_COMPLETE_IND message will be received after the last packet.
*/
void GoepLocalPutFirstTypeSource(GOEP *goep, uint16 size_name, const uint8* name, uint16 size_type, const uint8* type, uint16 size_packet, Source src, uint32 totalLen, bool onlyPacket);


/*!
	@brief Put the next packet of a multi-packet object.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_packet Length of packet in this request.
	@param src Source which contains the data.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
	
	A GOEP_LOCAL_PUT_DATA_REQUEST_IND message will be received for each packet.
	A GOEP_LOCAL_PUT_COMPLETE_IND message will be received after the last packet.
*/
void GoepLocalPutNextPacketSource(GOEP *goep, uint16 size_packet, Source src, bool lastPacket);


/*!
	@brief Start a GET transaction and wait for the first packet.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_name Length of the object name.
	@param name Object name.
	@param size_type Length of the type.
	@param type Type of object to get.
	
	A GOEP_LOCAL_GET_START_IND message will be received for the first packet.
	A GOEP_LOCAL_GET_COMPLETE_IND message will be received after the last packet.
	If size_name is non-zero while the name is NULL, an empty name header will be included in the packet.
*/
void GoepLocalGetFirstPacket(GOEP *goep, uint16 size_name, const uint8* name, uint16 size_type, const uint8* type);


/*!
	@brief Acknowledge the reception if this packet and get the next one.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	
	This function should only be called if the last packet received from a
	local GET operation had the moreData flag set to TRUE.
	
	A GOEP_LOCAL_GET_DATA_IND message will be received after each packet.
	A GOEP_LOCAL_GET_COMPLETE_IND message will be received after the last packet.
*/
void GoepLocalGetAck(GOEP *goep);


/*
	@brief Respond to a Remote Put start packet from a client.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param result Response code for the connection.
	
	This function should be called on receipt of a GOEP_REMOTE_PUT_START_IND to the client that 
	the packet has been received and the server is ready for the next.
	A GOEP_REMOTE_PUT_COMPLETE_IND message will be received after the last packet has arrived.
*/
void GoepRemotePutResponse(GOEP *goep, goep_svr_resp_codes result);


/*!
	@brief Acknowledge the receiption of a Remote PUT packet and request the
	next one.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	
	This function should only be called if the last packet received from a
	Remote PUT operation had the moreData flag set to TRUE.
	
	This function should be called on receipt of a GOEP_REMOTE_PUT_DATA_IND to the client that 
	the packet has been received and the server is ready for the next. A GOEP_REMOTE_PUT_DATA_IND
	message is received for each packet after the first.
	A GOEP_REMOTE_PUT_COMPLETE_IND message will be received after the last packet has arrived.
*/
void GoepRemotePutResponseAck(GOEP *goep);


/*!
	@brief Send the first response to a Remote Get request.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param result Response code for the connection.
	@param totLen Total length of the object.
	@param size_name Length of the object name.
	@param name Object name.
	@param size_type Length of the object type.
	@param type Object type.
	@param size_data Length of the data packet.
	@param data Data to send for the get.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
	
	If data is in a VM Source (e.g. from the filesystem),
	GoepRemoteGetResponseSource should be used instead of
	GoepRemoteGetResponse.
	
	This function should be called on receipt of a GOEP_REMOTE_GET_START_IND message.
	A GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND message will be received to request the next packet.
	A GOEP_REMOTE_GET_COMPLETE_IND message will be received after sending the final packet.
*/
void GoepRemoteGetResponse(GOEP *goep, goep_svr_resp_codes result, uint32 totLen, uint16 size_name, const uint8* name, uint16 size_type, const uint8* type, uint16 size_data, const uint8* data, bool lastPacket);

void GoepRemoteGetResponseHdr(GOEP *goep, goep_svr_resp_codes result, uint32 totLen, uint16 size_name, const uint8* name, uint16 size_type, const uint8* type, uint16 size_hdr, uint8* hdr, uint16 size_data, const uint8* data, bool lastPacket);

/*!
	@brief Send a subsequent response to a Remote Get request.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_data Length of the data packet.
	@param data Data to send for the get.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
	
	If data is in a VM Source (e.g. from the filesystem),
	GoepRemoteGetResponsePktSource should be used instead of
	GoepRemoteGetResponsePkt.
	
	This function should be called on receipt of a GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND message.
	A GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND message will be received to request the next packet.
	A GOEP_REMOTE_GET_COMPLETE_IND message will be received after sending the final packet.
*/
void GoepRemoteGetResponsePkt(GOEP *goep,  uint16 size_data, const uint8* data, bool lastPacket);


/*!
	@brief Send the first response to a Remote Get request.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param result Response code for the connection.
	@param totLen Total length of the object.
	@param size_name Length of the object name.
	@param name Object name.
	@param size_type Length of the object type.
	@param type Object type.
	@param size_data Length of the data packet.
	@param src Source which contains the data.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
	
	This function should be called on receipt of a GOEP_REMOTE_GET_START_IND message.
	A GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND message will be received to request the next packet.
	A GOEP_REMOTE_GET_COMPLETE_IND message will be received after sending the final packet.
*/
void GoepRemoteGetResponseSource(GOEP *goep, goep_svr_resp_codes result, uint32 totLen, uint16 size_name, const uint8* name, uint16 size_type, const uint8* type, uint16 size_data,  Source src, bool lastPacket);


/*!
	@brief Send a subsequent response to a Remote Get request.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_data Length of the data packet.
	@param src Source which contains the data.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
	
	This function should be called on receipt of a GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND message.
	A GOEP_REMOTE_GET_MORE_DATA_REQUEST_IND message will be received to request the next packet.
	A GOEP_REMOTE_GET_COMPLETE_IND message will be received after sending the final packet.
*/
void GoepRemoteGetResponsePktSource(GOEP *goep,  uint16 size_data, Source src, bool lastPacket);


/*!
	@brief Abort the current multi-packet transaction.
	@param session GOEP Session Handle (as returned in GOEP_INIT_CFM).
	
	The relevant COMPLETE_IND message will be received with the status code of goep_local_abort on success.
*/
void GoepAbort(GOEP *goep);


/*!
	@brief The packet received has been processed and is no longer needed.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	
	Every packet send to the client that contains a source must be declared
	complete before the next function is called.  e.g. When a
	GOEP_REMOTE_PUT_START_IND has been received, GoepPacketComplete must be
	called before calling GoepRemotePutResponse.
	
	No message is received on completion.
*/
void GoepPacketComplete(GOEP *goep);

/*!
	@brief Send a packet containing Application Specific Headers.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param length The total length of the paramters added.

	This function should be called on receipt of a GOEP_GET_APP_HEADERS_IND message
	when the parameters have been added to the packet.
*/
void GoepSendAppSpecificPacket(GOEP *goep, uint16 len_used);

/*!
	@brief Start a GET transaction using application specific headers.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param size_name Length of the object name.
	@param name Object name.
	@param size_type Length of the type.
	@param type Type of object to get.
	
	A GOEP_GET_APP_HEADERS_IND message will be received to indicate that the application
	should add it's custom parameters.  When complete, GoepSendAppSpecificPacket should
	be called to actually send the packet.
	If size_name is non-zero while the name is NULL, an empty name header will be included in the packet.
*/
void GoepLocalGetFirstPacketHeaders(GOEP *goep, uint16 size_name, const uint8* name, uint16 size_type, const uint8* type);

/*!
	@brief Send the first response to a Get, including application specific headers.
	@param goep GOEP Session Handle (as returned in GOEP_INIT_CFM).
	@param result Response code for the connection.
	@param totLen Total length of the object.
	@param size_name Length of the object name.
	@param name Object name.
	@param size_type Length of the object type.
	@param type Object type.
	
	A GOEP_GET_APP_HEADERS_IND message will be received to indicate that the application
	should add it's custom parameters.  When complete, GoepSendAppSpecificPacket should
	be called to actually send the packet.
*/
void GoepRemoteGetResponseHeaders(GOEP *goep, goep_svr_resp_codes result, uint32 totLen, uint16 size_name, const uint8* name, uint16 size_type, const uint8* type);
void GoepRemoteGetResponseAppHeaders(GOEP *goep, goep_svr_resp_codes result, uint32 totLen, uint16 size_header, uint8* header, uint16 size_type, const uint8* type);

#endif /* GOEP_H_ */

