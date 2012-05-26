/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    ConnectionSyncRenegotiate.c        

DESCRIPTION
    This file contains the functions responsible for managing the setting up 
    and tearing down of Synchronous connections.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"

extern const sync_config_params default_sync_config_params;

/*****************************************************************************/
void ConnectionSyncRenegotiate(Task theAppTask, Sink sink, const sync_config_params *config_params)
{
	/* Send an internal change Synchronous packet type request */
	MAKE_CL_MESSAGE(CL_INTERNAL_SYNC_RENEGOTIATE_REQ);
	message->theAppTask = theAppTask;
	message->audio_sink = sink;
    if ( config_params )
    {
        message->config_params = *config_params;
    }
    else
    {
        message->config_params = default_sync_config_params;
    }
    
	MessageSend(connectionGetCmTask(), CL_INTERNAL_SYNC_RENEGOTIATE_REQ, message);
}
