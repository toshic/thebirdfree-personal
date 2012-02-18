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
#include "a2dp_send_command_handler.h"
#include "a2dp_signalling_handler.h"
#include "a2dp_suspend_handler.h"
#include "a2dp_private.h"


void sendSuspendCfm(A2DP *a2dp, Sink sink, a2dp_status_code status)
{
	MAKE_A2DP_MESSAGE(A2DP_SUSPEND_CFM);
	message->media_sink	= sink;
	message->status = status;
	message->a2dp = a2dp;
	MessageSend(a2dp->clientTask, A2DP_SUSPEND_CFM, message);
}


/*****************************************************************************/
void a2dpHandleSuspendReq(A2DP *a2dp)
{
	if (!a2dp->signal_conn.sink)
	{
		sendSuspendCfm(a2dp, a2dp->media_conn.sink, a2dp_no_signalling_connection);
	}
	else if (!a2dp->media_conn.media_connected)
	{
		sendSuspendCfm(a2dp, 0, a2dp_no_media_connection);
	}
	else if (a2dp->signal_conn.signalling_state != avdtp_state_streaming)
	{
		sendSuspendCfm(a2dp, a2dp->media_conn.sink, a2dp_wrong_state);		
	}
	else
	{		
		a2dpSetSignallingState(a2dp, avdtp_state_local_suspending);

		if (!a2dpSendSuspend(a2dp))		
			a2dpAbortSignalling(a2dp, FALSE, FALSE);
	}
}
