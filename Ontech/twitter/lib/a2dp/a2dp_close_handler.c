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
#include "a2dp_close_handler.h"
#include "a2dp_private.h"
#include "a2dp_send_command_handler.h"
#include "a2dp_signalling_handler.h"

#include <print.h>

/*****************************************************************************/
void sendCloseCfm(A2DP *a2dp, a2dp_status_code status)
{
	MAKE_A2DP_MESSAGE(A2DP_CLOSE_CFM);
	message->media_sink = a2dp->media_conn.sink;
	message->status = status;
	message->a2dp = a2dp;
	MessageSend(a2dp->clientTask, A2DP_CLOSE_CFM, message);
}


/*****************************************************************************/
void a2dpHandleCloseReq(A2DP *a2dp)
{
	if (!a2dp->signal_conn.sink)
	{
		sendCloseCfm(a2dp, a2dp_no_signalling_connection);
	}
	else if (!a2dp->media_conn.media_connected)
	{
		sendCloseCfm(a2dp, a2dp_no_media_connection);
	}
	else
	{		
		if ((a2dp->signal_conn.signalling_state == avdtp_state_open) ||
			(a2dp->signal_conn.signalling_state == avdtp_state_streaming) ||
			(a2dp->signal_conn.signalling_state == avdtp_state_local_starting) ||
			(a2dp->signal_conn.signalling_state == avdtp_state_local_suspending) ||
			(a2dp->signal_conn.signalling_state == avdtp_state_reconfig_reading_caps) ||
			(a2dp->signal_conn.signalling_state == avdtp_state_reconfiguring)
			)
		{
			{
				a2dpSetSignallingState(a2dp, avdtp_state_local_closing);

				if (!a2dpSendClose(a2dp))		
					a2dpAbortSignalling(a2dp, FALSE, FALSE);
			}
		}
		else
			sendCloseCfm(a2dp, a2dp_wrong_state);
	}
}


