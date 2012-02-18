/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    oppc_interface.c
    
DESCRIPTION
	Interface source for the OPP Client library

*/

#include <message.h>
#include <panic.h>
#include <string.h>
#include <print.h>

#include <goep.h>

#include "oppc.h"
#include "oppc_private.h"

/*!
	@brief Initialise the OPPC Library.
	@param theAppTask The current application task.
	@param priority The profile instance low power mode priority. For a set of 
	profile instances with connections on a given ACL the instance with the highest 
	priority value wins and the low power mode on the ACL is set according to it's power table.
	
	OPPC_INIT_CFM message will be received by the application. 
*/
void OppcInit(Task theAppTask, uint16 priority)
{
	Task oppcTask;
	oppcState *state;
	MAKE_OPPC_MESSAGE(OPPC_INIT_CFM);
	
	PRINT(("OPPC Init\n"));
	
	oppcTask = oppcGetTheTask(); /* Can't fail since oppcGetTheTask will Panic on no memory */
	state = (oppcState*)oppcTask;
	
	state->theAppTask=theAppTask;
	state->rfcChan = 0; /* Ensure channel is invalid so we can see if we need to get one later */
	state->handle = NULL; /* Ensure handle is invalid so we can see if we need to get one later */
		
	message->oppc = state;
	message->status = goep_success;
	MessageSend(theAppTask, OPPC_INIT_CFM, message);
}

/*!
	@brief Open an OPPC Connection with a server.
	@param oppc OPPC Session Handle.
	@param bd_addr The Bluetooth address of the device being replied to.
	@param maxPacketSize Maximum packet size supported by this client.
	
	This will make a Bluetooth connection with the server.
	OPPC_CONNECT_CFM message will be received by the application. 
*/
void OppcConnect(OPPC *oppc, const bdaddr *bdAddr, const uint16 maxPacketSize)
{
	Task state;
	
#ifdef FTPC_LIBRARY_DEBUG
	if(bdAddr == NULL)
    {
       OPPC_DEBUG(("Null Bluetooth address pointer\n")); 
    }
#endif

    state = (Task)( &oppc->task );
	
	/* Send an internal message */
	{
		MAKE_OPPC_MESSAGE(OPPC_INT_CONNECT);        
		message->bdAddr = *bdAddr;
		message->maxPacketSize=maxPacketSize;        
		MessageSend(state, OPPC_INT_CONNECT, message);
	}
}

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
void oppcConnectAuthResponse(OPPC *oppc, const uint8 *digest, uint16 size_userid, const uint8 *userid, const uint8 *nonce)
{
	Task state;
	
    state = (Task)( &oppc->task );
	
	/* Send an internal message */
	{
		MAKE_OPPC_MESSAGE(OPPC_INT_AUTH_RESP);
		message->digest = digest;
		message->size_userid = size_userid;
		message->userid = userid;
		message->nonce = nonce;
		MessageSend(state, OPPC_INT_AUTH_RESP, message);
	}
}

/*!
	@brief Disconnect from an server.  
	@param oppc OPPC Session Handle.
	
	OPPC_DISCONNECT_CFM message will be received by the application. 
*/
void OppcDisconnect(OPPC *oppc)
{
    Task state;
    
    state = (Task)( &oppc->task );
    
	/* Send an internal message */
	MessageSend(state, OPPC_INT_DISCONNECT, OPPC_NO_PAYLOAD);
}

/****************************************************************************
NAME	
	oppcAbort

DESCRIPTION
	Abort the current multi-packet operation.
	
	The description field is an optional description of why the transfer was aborted.
	
RETURNS
    void
*/
void OppcAbort(OPPC *oppc, const uint16 descLen, const uint8* description)
{
    Task state;
    
    state = (Task)( &oppc->task );
    
	/* Send an internal message */
	{
		MAKE_OPPC_MESSAGE(OPPC_INT_ABORT);
        
        message->descLen = descLen;
        message->desc = description;
        
		MessageSend(state, OPPC_INT_ABORT, message);
	}
}

/****************************************************************************
NAME	
	oppcPushObject

DESCRIPTION
	Push the first packet of an object to the server.
	The ownership of all pointers remains with the caller.

RETURNS
    void
*/
void OppcPushObject(OPPC *oppc, 
						const uint16 nameLen, const uint8* name, 
						const uint16 typeLen, const uint8* type, 
						const uint16 length, const uint8 *packet, 
						const uint32 totalLen, const bool onlyPacket)
{
    Task state;
    
    state = (Task)( &oppc->task );
    
	/* Send an internal message */
	{
		MAKE_OPPC_MESSAGE(OPPC_INT_PUSH_OBJECT);
        
        message->nameLen = nameLen;
        message->name = name;
        message->typeLen = typeLen;
        message->type = type;
        message->length = length;
        message->packet = packet;
        message->totalLen = totalLen;
        message->onlyPacket = onlyPacket;
        
		MessageSend(state, OPPC_INT_PUSH_OBJECT, message);
	}
}

/****************************************************************************
NAME	
	oppcPushNextPacket

DESCRIPTION
	Push the next packet of the current object to the server.
	The ownership of the pointer 'packet' remains with the caller.
	
RETURNS
    void
*/
void OppcPushNextPacket(OPPC *oppc, 
                        const uint16 length, const uint8 *packet, const bool lastPacket)
{
    Task state;
    
    state = (Task)( &oppc->task );
    
	/* Send an internal message */
	{
		MAKE_OPPC_MESSAGE(OPPC_INT_PUSH_NEXT_PACKET);
        
        message->length = length;
        message->packet = packet;
        message->lastPacket = lastPacket;
        
		MessageSend(state, OPPC_INT_PUSH_NEXT_PACKET, message);
	}
}

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
*/
void OppcPushObjectSource(OPPC *oppc, 
						const uint16 size_name, const uint8* name, 
						const uint16 size_type, const uint8* type, 
						const uint16 size_packet, Source src, 
						const uint32 totalLen, const bool onlyPacket)
{
    Task state;
    
    state = (Task)( &oppc->task );
    
	/* Send an internal message */
	{
		MAKE_OPPC_MESSAGE(OPPC_INT_PUSH_OBJECT_SRC);
        
        message->nameLen = size_name;
        message->name = name;
        message->typeLen = size_type;
        message->type = type;
        message->length = size_packet;
        message->src = src;
        message->totalLen = totalLen;
        message->onlyPacket = onlyPacket;
        
		MessageSend(state, OPPC_INT_PUSH_OBJECT_SRC, message);
	}
}

/*!
	@brief Push the next packet of the current object to the server.
	@param oppc OPPC Session Handle.
	@param size_packet Length of the packet supplied with this request.
	@param src Source containing the packet to send.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).

	The ownership of the pointer 'packet' remains with the caller.
*/
void OppcPushNextPacketSource(OPPC *oppc, 
							const uint16 size_packet, Source src, const bool lastPacket)
{
    Task state;
    
    state = (Task)( &oppc->task );
    
	/* Send an internal message */
	{
		MAKE_OPPC_MESSAGE(OPPC_INT_PUSH_NEXT_PACKET_SRC);
        
        message->length = size_packet;
        message->src = src;
        message->lastPacket = lastPacket;
        
		MessageSend(state, OPPC_INT_PUSH_NEXT_PACKET_SRC, message);
	}
}

/****************************************************************************
NAME	
	oppcPushBusinessCard

DESCRIPTION
	Push the first packet of a vCard (business card) to the server.
	The ownership of all pointers remains with the caller.

RETURNS
    void
*/
void OppcPushBusinessCard(OPPC *oppc, 
						const uint16 size_name, const uint8* name, 
						const uint16 size_packet, const uint8 *packet, 
						const uint32 totalLen, const bool onlyPacket)
{
    Task state;
    
    state = (Task)( &oppc->task );
    
	/* Send an internal message */
	{
		MAKE_OPPC_MESSAGE(OPPC_INT_PUSH_OBJECT);
        
        message->nameLen = size_name;
        message->name = name;
        message->typeLen = sizeof(vcardType);
        message->type = &vcardType[0];
        message->length = size_packet;
        message->packet = packet;
        message->totalLen = totalLen;
        message->onlyPacket = onlyPacket;
        
		MessageSend(state, OPPC_INT_PUSH_OBJECT, message);
	}
}

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
*/
void OppcPushBusinessCardSource(OPPC *oppc, 
						const uint16 size_name, const uint8* name, 
						const uint16 size_packet, Source src, 
						const uint32 totalLen, const bool onlyPacket)
{
    Task state;
    
    state = (Task)( &oppc->task );
    
	/* Send an internal message */
	{
		MAKE_OPPC_MESSAGE(OPPC_INT_PUSH_OBJECT_SRC);
        
        message->nameLen = size_name;
        message->name = name;
        message->typeLen = sizeof(vcardType);
        message->type = &vcardType[0];
        message->length = size_packet;
        message->src = src;
        message->totalLen = totalLen;
        message->onlyPacket = onlyPacket;
        
		MessageSend(state, OPPC_INT_PUSH_OBJECT_SRC, message);
	}
}

/****************************************************************************
NAME	
	oppcPullBusinessCard

DESCRIPTION
	Request the owner's business card from the OPP Server.

RETURNS
    void
*/
void OppcPullBusinessCard(OPPC *oppc)
{
    Task state;
    
    state = (Task)( &oppc->task );
    
	/* Send an internal message */
	MessageSend(state, OPPC_INT_PULL_VCARD, OPPC_NO_PAYLOAD);
}

/****************************************************************************
NAME	
	oppcPullNextBcPacket

DESCRIPTION
	Pull the next packet of the current object from the server.
	
RETURNS
    void
*/
void OppcPullNextBcPacket(OPPC *oppc)
{
    Task state;
    
    state = (Task)( &oppc->task );
    
	/* Send an internal message */
	MessageSend(state, OPPC_INT_PULL_NEXT_VCARD, OPPC_NO_PAYLOAD);
}

/*!
	@brief The packet received has been processed and is no longer needed.
	@param session OPPC Session Handle (as returned in OPPC_INIT_CFM).
	
	Every packet send to the client that contains a source must be declared complete before the next
	function is called.
	e.g. When a OPPC_PULL_BC_START_IND has been received, OppcPacketComplete must be called before calling
	OppcPullNextBcPacket.
*/
void OppcPacketComplete(OPPC *oppc)
{
	GoepPacketComplete(oppc->handle);
}
