/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_signalling.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_connect_handler.h"
#include "a2dp_init.h"
#include "a2dp_open_handler.h"
#include "a2dp_private.h"

#include <stdlib.h>


/*****************************************************************************/
void A2dpConnectSignallingChannel(Task clientTask, const bdaddr *addr, device_sep_list *sep_list)
{
	A2DP *new_a2dp = NULL;

	if (!sep_list || !addr)
	{
		/* The app must supply the details of the available SEPs, and the address of the device to connect to. */
		a2dpSendSignallingConnectCfm(0, clientTask, a2dp_invalid_parameters, 0);
		return;
	}

	/* Create the a2dp task */
	new_a2dp = (A2DP *) malloc(sizeof(A2DP));

	if (new_a2dp)
	{
		/* Send an internal message to initiate the connection. */
		MAKE_A2DP_MESSAGE(A2DP_INTERNAL_CONNECT_SIGNALLING_REQ);
		message->client = clientTask;
		message->addr = *addr;
		
		a2dpInitTask(new_a2dp, clientTask);

		new_a2dp->sep.sep_list = sep_list;
		
		MessageSend(&new_a2dp->task, A2DP_INTERNAL_CONNECT_SIGNALLING_REQ, message);   
	}
	else
	{
		/* Failed to create the task so cannot proceed further */
		a2dpSendSignallingConnectCfm(0, clientTask, a2dp_insufficient_memory, 0);
	}
}

/*****************************************************************************/
void A2dpConnectSignallingChannelResponse(Task clientTask, A2DP *a2dp, bool accept, uint16 connection_id, device_sep_list *sep_list)
{
	MAKE_A2DP_MESSAGE(A2DP_INTERNAL_CONNECT_SIGNALLING_RES);

#ifdef A2DP_DEBUG_LIB
	if (!a2dp)
		A2DP_DEBUG(("A2dpConnectSignallingChannelResponse NULL instance\n"));
#endif

	if (!sep_list)
	{
		accept = FALSE;
	}
	else
	{
		a2dp->sep.sep_list = sep_list;
	}

	message->client = clientTask;
	message->accept = accept;
	message->connection_id = connection_id;
	
	MessageSend(&a2dp->task, A2DP_INTERNAL_CONNECT_SIGNALLING_RES, message);   
}


/*****************************************************************************/
Sink A2dpGetSignallingSink(A2DP *a2dp)
{
    if (!a2dp)
    {
#ifdef A2DP_DEBUG_LIB
        A2DP_DEBUG(("A2dpGetSignallingSink NULL instance\n"));
#endif
        return (Sink)0;
    }

    return a2dp->signal_conn.sink;
}

