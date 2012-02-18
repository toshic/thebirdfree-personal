/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_start_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp_send_command_handler.h"
#include "a2dp_signalling_handler.h"
#include "a2dp_start_handler.h"
#include "a2dp_private.h"


void sendStartCfm(A2DP *a2dp, Sink sink, a2dp_status_code status)
{
	MAKE_A2DP_MESSAGE(A2DP_START_CFM);
	message->media_sink	= sink;
	message->status = status;
	message->a2dp = a2dp;
	MessageSend(a2dp->clientTask, A2DP_START_CFM, message);
}


/*****************************************************************************/
void a2dpHandleStartReq(A2DP *a2dp)
{
	if (!a2dp->signal_conn.sink)
	{
		sendStartCfm(a2dp, a2dp->media_conn.sink, a2dp_no_signalling_connection);
	}
	else if (!a2dp->media_conn.media_connected)
	{
		sendStartCfm(a2dp, 0, a2dp_no_media_connection);
	}
	else if (a2dp->signal_conn.signalling_state != avdtp_state_open)
	{
		sendStartCfm(a2dp, a2dp->media_conn.sink, a2dp_wrong_state);		
	}
	else
	{		
		a2dpSetSignallingState(a2dp, avdtp_state_local_starting);

		if (!a2dpSendStart(a2dp))		
			a2dpAbortSignalling(a2dp, FALSE, FALSE);
	}
}
