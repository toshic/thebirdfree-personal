/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    sdp_register.c        

DESCRIPTION
		

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"

#include <panic.h>
#include <string.h>


/*****************************************************************************/
void ConnectionRegisterServiceRecord(Task appTask, uint16 num_rec_bytes, const uint8 *service_record)
{
	/* Check some record has been supplied */
#ifdef CONNECTION_DEBUG_LIB	
	if (num_rec_bytes == 0)
	{
		CL_DEBUG(("sdp - Zero length record supplied\n"));
	}
#endif

	{
		/* Send an internal message to the state machine */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_REGISTER_RECORD_REQ);
		message->theAppTask = appTask;
		message->record_length = num_rec_bytes;

		if (num_rec_bytes)
			message->record = (uint8 *) service_record;
		else
			message->record = 0;

		MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_REGISTER_RECORD_REQ, message);
	}
}


/*****************************************************************************/
void ConnectionUnregisterServiceRecord(Task appTask, uint32 service_record_hdl)
{
	/* Create an internal message and send it to the state machine */
	MAKE_CL_MESSAGE(CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ);
	message->theAppTask = appTask;
	message->service_handle = service_record_hdl;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ, message);
}

