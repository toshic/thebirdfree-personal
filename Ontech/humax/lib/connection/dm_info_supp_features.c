/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    dm_info_supp_features.c        

DESCRIPTION
    This file contains the management entity responsible for arbitrating 
    access to functionality in BlueStack that provides information on
    the setup of the local device or about the link to the remote device.

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"


/*****************************************************************************/
void ConnectionReadRemoteSuppFeatures(Task theAppTask, Sink sink)
{
	/* All requests are sent through the internal state handler */    
	MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ);
	message->theAppTask = theAppTask;
	message->sink = sink;
	MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ, message, &theCm.infoState.stateInfoLock);
}

