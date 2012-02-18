/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    opps_interface.c
    
DESCRIPTION
	Interface source for the OPP Server library

*/

#include <message.h>
#include <panic.h>
#include <string.h>
#include <print.h>

#include <goep.h>

#include "opps.h"
#include "opps_private.h"


/*!
	@brief Initialise the OPPS Library.
	@param theAppTask The current application task.
	@param priority The profile instance low power mode priority. For a set of 
	profile instances with connections on a given ACL the instance with the highest 
	priority value wins and the low power mode on the ACL is set according to it's power table.
	
	OPPS_INIT_CFM message will be received by the application. 
*/
void OppsInit(Task theAppTask, uint16 priority)
{
	Task oppsTask;
	oppsState *state;
	
	PRINT(("OPPS Init\n"));
	
	oppsTask = oppsGetTheTask(); /* Can't fail since ftpcGetTheTask will Panic on no memory */
	state = (oppsState*)oppsTask;
	
	state->theAppTask=theAppTask;
		
	GoepInit(&state->task, goep_Server, goep_OBEX);
}


/****************************************************************************
NAME	
	OppsConnectResponse

DESCRIPTION
	Accept or refuse a connection.
	
RETURNS
    void
*/
void OppsConnectResponse(OPPS *opps, bool accept, uint16 pktSize)
{
    Task state;
    
    state = (Task)(&opps->task);
	
	/* Send an internal message */
	{
		MAKE_OPPS_MESSAGE(OPPS_INT_ACCEPT_CONNECTION);
        
        message->accept = accept;
        message->pktSize = pktSize;
        
		MessageSend(state, OPPS_INT_ACCEPT_CONNECTION, message);
	}
}

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
void OppsConnectAuthChallenge(OPPS *opps, const uint8 *nonce, uint8 options, uint16 size_realm, const uint8 *realm)
{
    Task state;
    
    state = (Task)(&opps->task);
	
	/* Send an internal message */
	{
		MAKE_OPPS_MESSAGE(OPPS_INT_AUTH_CHALLENGE);
        
        message->nonce = nonce;
        message->options = options;
        message->size_realm = size_realm;
        message->realm = realm;
        
		MessageSend(state, OPPS_INT_AUTH_CHALLENGE, message);
	}
}

/*!
	@brief Abort the current multi-packet operation.

	@param opps OPPS Session Handle.
	
	The relevant COMPLETE_IND message will be received with the status code of opps_aborted on success.
*/
void OppsAbort(OPPS *opps)
{
    Task state;
    
    state = (Task)( &opps->task );
    
	MessageSend(state, OPPS_INT_ABORT, NULL);
}

/****************************************************************************
NAME	
	oppsGetNextPutPacket

DESCRIPTION
	Request the next packet during a Remote PUT operation.  
	
RETURNS
    void
*/
void OppsGetNextPutPacket(OPPS *opps, bool moreData)
{
    Task state;
    
    state = (Task)(&opps->task);
	
	/* Send an internal message */
	{
		MAKE_OPPS_MESSAGE(OPPS_INT_GET_NEXT_PACKET);
        
        message->moreData = moreData;
        
		MessageSend(state, OPPS_INT_GET_NEXT_PACKET, message);
	}
}

/****************************************************************************
NAME	
	oppsPushBusinessCardStart

DESCRIPTION
	Push the first packet of a vCard (business card) to the server.
RETURNS
    void
*/
void OppsPushBusinessCardStart(OPPS *opps, 
						const uint16 nameLen, const uint8* name, 
						const uint16 length, const uint8 *packet, 
						const uint32 totalLen, const bool onlyPacket)
{
    Task state;
    
    state = (Task)(&opps->task);
	
	/* Send an internal message */
	{
		MAKE_OPPS_MESSAGE(OPPS_INT_PUSH_VCARD);
        
		message->nameLen = nameLen;
		message->name = name;
		message->length = length;
		message->packet = packet;
		message->totalLen = totalLen;
		message->onlyPacket = onlyPacket;
        
		MessageSend(state, OPPS_INT_PUSH_VCARD, message);
	}
}

/****************************************************************************
NAME	
	oppsPushBusinessCardNext

DESCRIPTION
	Push the next packet of the current vCard to the server.
RETURNS
    void
*/
void OppsPushBusinessCardNext(OPPS *opps, 
							const uint16 length, const uint8 *packet, const bool lastPacket)
{
    Task state;
    
    state = (Task)(&opps->task);
	
	/* Send an internal message */
	{
		MAKE_OPPS_MESSAGE(OPPS_INT_PUSH_NEXT_VCARD);
        
		message->length = length;
		message->packet = packet;
		message->lastPacket = lastPacket;
        
		MessageSend(state, OPPS_INT_PUSH_NEXT_VCARD, message);
	}
}

/*!
	@brief Push the first packet of a vCard (business card) to the server.
	@param session OPPS Session Handle.
	@param size_name Length of the name field.
	@param name The object name.
	@param size_packet Length of the packet supplied with this request.
	@param src Source containing the data packet.
	@param totalLen Total length of the object being sent.
	@param onlyPacket Is this the only packet? Yes(TRUE) or No(FALSE).
*/
void OppsPushBusinessCardStartSource(OPPS *opps, 
						const uint16 size_name, const uint8* name, 
						const uint16 size_packet, Source src, 
						const uint32 totalLen, const bool onlyPacket)
{
    Task state;
    
    state = (Task)(&opps->task);
	
	/* Send an internal message */
	{
		MAKE_OPPS_MESSAGE(OPPS_INT_PUSH_VCARD_SRC);
        
		message->nameLen = size_name;
		message->name = name;
		message->length = size_packet;
		message->src = src;
		message->totalLen = totalLen;
		message->onlyPacket = onlyPacket;
        
		MessageSend(state, OPPS_INT_PUSH_VCARD_SRC, message);
	}
}

/*!
	@brief Push the next packet of the current vCard to the server.
	@param session OPPS Session Handle.
	@param size_packet Length of the packet supplied with this request.
	@param src Source containing the data packet.
	@param lastPacket Is this the last packet? Yes(TRUE) or No(FALSE).
*/
void OppsPushBusinessCardNextSource(OPPS *opps, 
							const uint16 size_packet, Source src, const bool lastPacket)
{
    Task state;
    
    state = (Task)(&opps->task);
	
	/* Send an internal message */
	{
		MAKE_OPPS_MESSAGE(OPPS_INT_PUSH_NEXT_VCARD_SRC);
        
		message->length = size_packet;
		message->src = src;
		message->lastPacket = lastPacket;
        
		MessageSend(state, OPPS_INT_PUSH_NEXT_VCARD_SRC, message);
	}
}
		
/*!
	@brief The packet received has been processed and is no longer needed.
	@param session OPPS Session Handle (as returned in OPPS_INIT_CFM).
	
	Every packet send to the client that contains a source must be declared complete before the next
	function is called.
	e.g. When a OPPS_PUSH_OBJ_START_IND has been received, OppsPacketComplete must be called before calling
	OppsGetNextPutPacket.
*/
void OppsPacketComplete(OPPS *opps)
{
	GoepPacketComplete(opps->handle);
}
