/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_delete.c        

DESCRIPTION

NOTES

*/



/****************************************************************************
	Header files
*/

#include "a2dp_connect_handler.h"
#include "a2dp_delete.h"
#include "a2dp_free.h"
#include "a2dp_private.h"

#include <stdlib.h>


/*****************************************************************************/
void a2dpDeleteTask(A2DP *a2dp)
{
    /* Set the state so that a remote device will not be able to connect before the task is deleted */
    a2dpSetSignallingConnectionState(a2dp, avdtp_connection_connecting_crossover);
    
    /* Send the message */
    MessageSend(&a2dp->task, A2DP_INTERNAL_TASK_DELETE_REQ, 0);
}


/*****************************************************************************/
void a2dpHandleDeleteTask(A2DP *a2dp)
{
	a2dpFreeConfiguredCapsMemory(a2dp);

	a2dpFreeSeidListMemory(a2dp);

    /* Discard all messages for the task and free it. */
	(void) MessageFlushTask(&a2dp->task);
    free(a2dp);
}

