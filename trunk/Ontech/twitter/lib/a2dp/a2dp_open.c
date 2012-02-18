/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_open.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_free.h"
#include "a2dp_init.h"
#include "a2dp_get_sep.h"
#include "a2dp_open_handler.h"
#include "a2dp_private.h"
#include "a2dp_delete.h"

#include <sink.h>
#include <stdlib.h>
#include <string.h>


static void initiateOpen(A2DP *a2dp, Task clientTask, const bdaddr *addr, uint16 size_local_seids, uint8 *local_seids)
{
	sep_type *pSeps = NULL;

	a2dp->sep.list_preferred_local_seids = (uint8 *) malloc(sizeof(uint8) * size_local_seids);
	memmove(a2dp->sep.list_preferred_local_seids, local_seids, size_local_seids);
	a2dp->sep.max_preferred_local_seids = size_local_seids;
	a2dp->sep.current_preferred_local_seid = 0;

	pSeps = getSepInstanceBySeid(a2dp, a2dp->sep.list_preferred_local_seids[0]);

	if (pSeps == NULL)
	{
		/* If the SEID is not valid then can't proceed. */
		if (a2dp->signal_conn.connect_then_open_media)
                {
			a2dpSendConnectOpenCfm(a2dp, clientTask, a2dp_invalid_parameters);
                        /* ensure we delete the a2dp instance */
                        a2dpDeleteTask(a2dp);
                }
		else
			a2dpSendOpenCfm(a2dp, clientTask, a2dp_invalid_parameters);
                        /* don't delete a2dp instance, we have a signalling channel on the go */
		return;
	}

	a2dp->sep.current_sep = pSeps;

	{
		/* Send an internal message to initiate the connection. */
		MAKE_A2DP_MESSAGE(A2DP_INTERNAL_OPEN_REQ);
		message->client = a2dp->clientTask;
		message->addr = *addr;
		MessageSend(&a2dp->task, A2DP_INTERNAL_OPEN_REQ, message);    
	}
}


/*****************************************************************************/
void A2dpOpen(A2DP *a2dp, uint16 size_local_seids, uint8 *local_seids)
{
	bdaddr addr;

#ifdef A2DP_DEBUG_LIB
	if (!a2dp)
		A2DP_DEBUG(("A2dpOpen NULL instance\n"));
#endif

	if (!local_seids || !size_local_seids)
    {
        /* If the user failed to supply connect params go no further. */
        a2dpSendOpenCfm(a2dp, a2dp->clientTask, a2dp_invalid_parameters);
		return;
    }

	if ((a2dp->signal_conn.connection_state != avdtp_connection_connected) || !SinkGetBdAddr(a2dp->signal_conn.sink, &addr))
	{
		/* There must be a signalling channel open first. */
        a2dpSendOpenCfm(a2dp, a2dp->clientTask, a2dp_no_signalling_connection);
		return;
	}

	a2dpFreeSeidListMemory(a2dp);

	a2dp->signal_conn.connect_then_open_media = FALSE;

	initiateOpen(a2dp, a2dp->clientTask, &addr, size_local_seids, local_seids);
}


/*****************************************************************************/
void A2dpConnectOpen(Task clientTask, const bdaddr *addr, uint16 size_local_seids, uint8 *local_seids, device_sep_list *sep_list)
{
	A2DP *new_a2dp = NULL;

	if (!addr || !local_seids || !size_local_seids || !sep_list)
    {
        /* If the user failed to supply connect params go no further. */
        a2dpSendConnectOpenCfm(0, clientTask, a2dp_invalid_parameters);
		return;
    }

	/* Create the a2dp task */
	new_a2dp = (A2DP *) malloc(sizeof(A2DP));

	if (new_a2dp)
	{
		a2dpInitTask(new_a2dp, clientTask);

		new_a2dp->sep.sep_list = sep_list;
	}
	else
	{
		/* Failed to create the task so cannot proceed further */
		a2dpSendConnectOpenCfm(0, clientTask, a2dp_insufficient_memory);
		return;
	}

	new_a2dp->signal_conn.connect_then_open_media = TRUE;

	initiateOpen(new_a2dp, clientTask, addr, size_local_seids, local_seids);
}


/*****************************************************************************/
Sink A2dpGetMediaSink(A2DP *a2dp)
{
    if (!a2dp)
    {
#ifdef A2DP_DEBUG_LIB
        A2DP_DEBUG(("A2dpGetMediaSink NULL instance\n"));
#endif
        return (Sink)0;
    }

	if (!a2dp->media_conn.media_connected)
		return (Sink)0;

    return a2dp->media_conn.sink;
}
