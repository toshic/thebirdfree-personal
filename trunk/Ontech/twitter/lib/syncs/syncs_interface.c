/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbaps_interface.h
    
DESCRIPTION
	PhoneBook Access Profile Server Library - public interface.

*/

#include <vm.h>
#include <print.h>
#include <memory.h>

#include "syncs.h"
#include "syncs_private.h"


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
void SyncsInit(Task theAppTask, uint16 priority, uint8 stores)
{
	syncsState *state = PanicUnlessNew(syncsState);
	
	PRINT(("SYNCS Init\n"));
		
	memset(state, 0, sizeof(syncsState));
	
	state->task.handler = syncsIntHandler;
	state->theAppTask = theAppTask;
	state->stores = stores;
		
	GoepInit(&state->task, goep_Server, goep_SYNC_PSE);
}

/*!
 @brief Respond to a connection request.  
 @param pbaps PBAPS Session Handle.
 @param accept TRUE to accept the connection, or FALSE to refuse it.
 @param pktSize Maximum packet size that can be accepted on this connection.
*/
void SyncsConnectResponse(SYNCS *syncs, bool accept, uint16 pktSize)
{
    Task state;
    
    state = (Task)(&syncs->task);
	
	/* Send an internal message */
	{
		MAKE_SYNCS_MESSAGE(SYNCS_INT_CONNECTION_RESP);
        
        message->accept = accept;
        message->pktSize = pktSize;
        
		MessageSend(state, SYNCS_INT_CONNECTION_RESP, message);
	}
}

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
void SyncsConnectAuthChallenge(SYNCS *syncs, const uint8 *nonce, uint8 options, uint16 size_realm, const uint8 *realm)
{
    Task state;
    
    state = (Task)(&syncs->task);
	
	/* Send an internal message */
	{
		MAKE_SYNCS_MESSAGE(SYNCS_INT_AUTH_CHALLENGE);
        
        message->nonce = nonce;
        message->options = options;
        message->size_realm = size_realm;
        message->realm = realm;
        
		MessageSend(state, SYNCS_INT_AUTH_CHALLENGE, message);
	}
}
