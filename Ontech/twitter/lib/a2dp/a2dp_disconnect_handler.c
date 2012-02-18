/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_suspend_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp_disconnect_handler.h"

#include "a2dp_signalling_handler.h"


/*****************************************************************************/
void a2dpHandleDisconnectAllReq(A2DP *a2dp)
{
	if ((a2dp->signal_conn.signalling_state == avdtp_state_open) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_streaming) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_local_starting) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_local_suspending) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_reconfig_reading_caps) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_reconfiguring)
		)
	{
		a2dpAbortSignalling(a2dp, FALSE, FALSE);
	}

	/* Start signalling connection timer. When it fires the signalling channel will be disconnected. */
	MessageSendLater((Task)&a2dp->task, A2DP_INTERNAL_SIGNAL_CONNECTION_TIMEOUT_IND, 0, SIGNAL_TIMER);
}


