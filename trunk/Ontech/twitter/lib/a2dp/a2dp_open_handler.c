/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_open_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp_connect_handler.h"
#include "a2dp_free.h"
#include "a2dp_open_handler.h"
#include "a2dp_private.h"

#include <print.h>


/*****************************************************************************/
void a2dpSendConnectOpenCfm(A2DP *a2dp, Task client, a2dp_status_code status)
{
    MAKE_A2DP_MESSAGE(A2DP_CONNECT_OPEN_CFM);

	if (status != a2dp_success)
	{
		message->a2dp = 0;
		message->signalling_sink = 0;
		message->media_sink = 0;
		message->seid = 0;

		/* If the operation has failed but the signalling channel is open, then it must be disconnected here. */
		if (a2dp && a2dp->signal_conn.sink)
		{
			ConnectionL2capDisconnectRequest(&a2dp->task, a2dp->signal_conn.sink);
		}
		else
		{
			a2dp->signal_conn.connect_then_open_media = FALSE;
		}
	}
	else
	{
		message->a2dp = a2dp;
		message->signalling_sink = a2dp->signal_conn.sink;
		message->media_sink = a2dp->media_conn.sink;
		message->seid = a2dp->sep.current_sep->sep_config->seid;

		a2dp->signal_conn.connect_then_open_media = FALSE;
	}

    message->status = status;
   
    MessageSend(a2dp->clientTask, A2DP_CONNECT_OPEN_CFM, message);
}


/*****************************************************************************/
void a2dpSendOpenCfm(A2DP *a2dp, Task client, a2dp_status_code status)
{
    MAKE_A2DP_MESSAGE(A2DP_OPEN_CFM);
    message->a2dp = a2dp;
    message->status = status;
	if (status == a2dp_success)
	{
		message->media_sink = a2dp->media_conn.sink;
		message->seid = a2dp->sep.current_sep->sep_config->seid;
	}
	else
	{
		message->media_sink = 0;
		message->seid = 0;
	}
    MessageSend(client, A2DP_OPEN_CFM, message);

	if (a2dp)
		a2dpFreeSeidListMemory(a2dp);
}


/*****************************************************************************/
void a2dpHandleOpenReq(A2DP *a2dp, const A2DP_INTERNAL_OPEN_REQ_T *req)
{
	PRINT(("a2dpHandleOpenReq\n"));
	if (a2dp->signal_conn.signalling_state != avdtp_state_idle)
	{
		if (a2dp->signal_conn.connect_then_open_media)
			a2dpSendConnectOpenCfm(a2dp, a2dp->clientTask, a2dp_wrong_state);
		else
			a2dpSendOpenCfm(a2dp, a2dp->clientTask, a2dp_wrong_state);
	}
	else if (a2dp->signal_conn.connection_state == avdtp_connection_idle)
	{
		/* Signalling channel is not connected, so connect it now. */
		a2dpOpenSignallingChannel(a2dp, &req->addr);
	}
	else if (a2dp->signal_conn.connection_state == avdtp_connection_connected)
	{
		PRINT(("signalling already open\n"));
		a2dpSignalChannelOpen(a2dp, TRUE);
	}
	else
	{
		if (a2dp->signal_conn.connect_then_open_media)
			a2dpSendConnectOpenCfm(a2dp, a2dp->clientTask, a2dp_wrong_state);
		else
			a2dpSendOpenCfm(a2dp, a2dp->clientTask, a2dp_wrong_state);
	}
}
