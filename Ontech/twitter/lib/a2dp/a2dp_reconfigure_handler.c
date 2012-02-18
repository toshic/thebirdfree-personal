/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_reconfigure_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/

#include "a2dp_reconfigure_handler.h"
#include "a2dp_private.h"
#include "a2dp_send_command_handler.h"
#include "a2dp_signalling_handler.h"


/*****************************************************************************/
void a2dpSendReconfigureCfm(A2DP *a2dp, a2dp_status_code status)
{
    MAKE_A2DP_MESSAGE(A2DP_RECONFIGURE_CFM);
    message->a2dp = a2dp;
    message->status = status;
    MessageSend(a2dp->clientTask, A2DP_RECONFIGURE_CFM, message);
}


/*****************************************************************************/
void a2dpHandleReconfigureReq(A2DP *a2dp, const A2DP_INTERNAL_RECONFIGURE_REQ_T *req)
{
	if ((a2dp->signal_conn.signalling_state != avdtp_state_open) || !a2dp->sep.remote_seid)
	{
		a2dpSendReconfigureCfm(a2dp, a2dp_wrong_state);
	}
	else
	{
		a2dp->sep.reconfigure_caps_size = req->size_sep_caps;
		a2dp->sep.reconfigure_caps = req->sep_caps;

		/* TODO more checks on caps passed in */

		a2dpSetSignallingState(a2dp, avdtp_state_reconfig_reading_caps);
		(void)a2dpSendGetCapabilities(a2dp, a2dp->sep.remote_seid);
	}
}
