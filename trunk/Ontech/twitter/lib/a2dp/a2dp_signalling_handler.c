/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_connect_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp_close_handler.h"
#include "a2dp_connect_handler.h"
#include "a2dp_current_sep_handler.h"
#include "a2dp_delete.h"
#include "a2dp_free.h"
#include "a2dp_open_handler.h"
#include "a2dp_send_command_handler.h"
#include "a2dp_signalling_handler.h"

#include <print.h>
#include <stdlib.h>


/*****************************************************************************/
static void startWatchdogTimer(Task my_task, uint32 t)
{
	MessageSendLater(my_task, A2DP_INTERNAL_WATCHDOG_IND, 0, t);
}


/*****************************************************************************/
static void setSignallingToIdle(A2DP* a2dp)
{
	a2dpFreeConfiguredCapsMemory(a2dp);

	a2dpFreeSeidListMemory(a2dp);

	/* Cancel the watchdog timeout */
	(void) MessageCancelAll((Task)&a2dp->task, A2DP_INTERNAL_WATCHDOG_IND);

	a2dp->signal_conn.pending_transaction_label = 0;

	a2dpSetSignallingState(a2dp, avdtp_state_idle);
}


/*****************************************************************************/
static void sendCloseInd(A2DP *a2dp, a2dp_status_code status)
{
	MAKE_A2DP_MESSAGE(A2DP_CLOSE_IND);
	message->media_sink = a2dp->media_conn.sink;
	message->status = status;
	message->a2dp = a2dp;
	MessageSend(a2dp->clientTask, A2DP_CLOSE_IND, message);
}


/*****************************************************************************/
void a2dpHandleConnectSignallingReq(A2DP *a2dp, const A2DP_INTERNAL_CONNECT_SIGNALLING_REQ_T *req)
{
	if (a2dp->signal_conn.connection_state == avdtp_connection_idle)
	{
		a2dp->signal_conn.connect_then_open_media = FALSE;
		a2dpOpenSignallingChannel(a2dp, &req->addr);
	}
	else
	{
		a2dpSendSignallingConnectCfm(a2dp, a2dp->clientTask, a2dp_wrong_state, 0);
	}
}


/*****************************************************************************/
void a2dpHandleConnectSignallingRes(A2DP *a2dp, const A2DP_INTERNAL_CONNECT_SIGNALLING_RES_T *res)
{
	/* Process the response if the library was waiting for one. No validation is done on the connection id returned. */
	if (a2dp->signal_conn.waiting_response)
	{
		a2dp->signal_conn.waiting_response--;
		/* Update client task */
		a2dp->clientTask = res->client;
		
		/* Send the connect response to the connection lib */
		ConnectionL2capConnectResponse(&a2dp->task, res->accept, AVDTP_PSM, res->connection_id, NULL);

		if (!res->accept)
        {
            if ((a2dp->signal_conn.connection_state == avdtp_connection_connecting) ||
				(a2dp->signal_conn.connection_state == avdtp_connection_disconnecting)
			)
			{
				/* Check that no other connections exist */
				if ((a2dp->signal_conn.sink == 0) && (a2dp->media_conn.media_connected == 0))
				{
                	/* The signalling connection was rejected so must delete the A2DP instance */
			     	a2dpDeleteTask(a2dp);
				}
			}
            else if (a2dp->signal_conn.connection_state == avdtp_connection_connecting_crossover)
			{
                /* One of the two signalling connections is no longer active, update to indicate 
                    one signalling channel left active */
                a2dpSetSignallingConnectionState(a2dp, avdtp_connection_connecting);
			}
        }
	}
}


/****************************************************************************/
void a2dpSetSignallingState(A2DP *a2dp, avdtp_state state)
{
	PRINT(("a2dpSetSignallingState: O:%d N:%d\n", a2dp->signal_conn.signalling_state, state));

	if (((state == avdtp_state_local_aborting) ||
		(state == avdtp_state_remote_aborting)) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_local_aborting) &&
		(a2dp->signal_conn.signalling_state != avdtp_state_remote_aborting))
		/* on first entering abort we need to store the state
		   we came from */
	{
		PRINT(("store preabort state : %d\n",state));
		a2dp->signal_conn.preabort_state = a2dp->signal_conn.signalling_state;
	}

	a2dp->signal_conn.signalling_state = state;

	/* Cancel the watchdog timeout*/
	(void) MessageCancelAll((Task)&a2dp->task, A2DP_INTERNAL_WATCHDOG_IND);
	
	/* do we need to restart the watchdog? */
	switch (state)
	{
		case avdtp_state_idle:
		case avdtp_state_configured:
		case avdtp_state_open:
		case avdtp_state_streaming:
			/* These states are stable and therefore
			   do not require the watchdog. */
			break;
	
		/* 
		   From Generic Audio/Video Distribution Profile, Table 4-1
		   Some signals use TGAVDP100 and some don't.  For those
		   that don't we apply our own timeout to prevent lock-up.
		*/
		case avdtp_state_configuring:
		case avdtp_state_local_opening:
		case avdtp_state_local_starting:
		case avdtp_state_local_suspending:
		case avdtp_state_local_closing:
		case avdtp_state_remote_closing:
		case avdtp_state_reconfiguring:
		case avdtp_state_local_aborting:
		case avdtp_state_remote_aborting:
			startWatchdogTimer((Task)&a2dp->task, WATCHDOG_TGAVDP100);
			break;

		case avdtp_state_discovering:
		case avdtp_state_reading_caps:
		case avdtp_state_processing_caps:
		case avdtp_state_reconfig_reading_caps:
		case avdtp_state_remote_opening:
			startWatchdogTimer((Task)&a2dp->task, WATCHDOG_GENERAL);
			break;
	}
}


/****************************************************************************/
void a2dpHandleWatchdogInd(A2DP *a2dp)
{
	if ((a2dp->signal_conn.signalling_state == avdtp_state_local_aborting) ||
			(a2dp->signal_conn.signalling_state == avdtp_state_remote_aborting))
	{
		/* The watchdog fired while we were aborting, kill. */
		a2dpResetSignalling(a2dp, FALSE, FALSE);
	}
	else
	{
		/* Abort */
		a2dpAbortSignalling(a2dp, FALSE, FALSE);
	}
}


/*****************************************************************************/
void a2dpAbortSignalling(A2DP *a2dp, bool link_loss, bool key_missing)
{
	PRINT(("a2dpAbortSignalling\n"));

	if ((a2dp->signal_conn.signalling_state == avdtp_state_local_aborting) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_remote_aborting))
		/* already aborting */
		return;

	if (a2dp->signal_conn.connection_state == avdtp_connection_connected)
	{
		/* change to aborting */
		a2dpSetSignallingState(a2dp, avdtp_state_local_aborting);
		
		/* send the abort command */
		if (!a2dpSendAbort(a2dp))
			a2dpResetSignalling(a2dp, FALSE, key_missing);
	}
	else
		/* can't signal, so just reset */
		a2dpResetSignalling(a2dp, FALSE, key_missing);	
}


/*****************************************************************************/
void a2dpResetSignalling(A2DP *a2dp, bool link_loss, bool key_missing)
{
	avdtp_state state = a2dp->signal_conn.signalling_state;

	bool report_close = FALSE;

	PRINT(("a2dpResetSignalling\n"));

	/* If we aborted, we want to process based on the state
	   we were in before the abort */
	if ((state == avdtp_state_local_aborting) ||
		(state == avdtp_state_remote_aborting))
	{
		state = a2dp->signal_conn.preabort_state;
		PRINT(("use preabort state : %d\n",state));
	}

	/* Return reasonable primitive based on state. */
	switch (state)
	{
		case avdtp_state_local_aborting:
		case avdtp_state_remote_aborting:
			return;

		case avdtp_state_idle:
		case avdtp_state_remote_opening:
        case avdtp_state_configured:
			/* Nothing to report */
			break;
		
		case avdtp_state_discovering:
		case avdtp_state_reading_caps:
		case avdtp_state_processing_caps:
		case avdtp_state_configuring:
		case avdtp_state_local_opening:
		{
			/* 
				Initiator has failed to open a remote SEP - report failure of
				initial request. 
			*/
            a2dp_status_code status = key_missing ? a2dp_key_missing : 
                                                    a2dp_operation_fail;
            
			if (a2dp->signal_conn.connect_then_open_media)
				a2dpSendConnectOpenCfm(a2dp, a2dp->clientTask, status);
			else
				a2dpSendOpenCfm(a2dp, a2dp->clientTask, status);
		}
		break;

		case avdtp_state_local_starting:
		case avdtp_state_local_suspending:
		{
			report_close = TRUE; 
		}
		break;

		case avdtp_state_reconfig_reading_caps:
		case avdtp_state_reconfiguring:
		{
			report_close = TRUE; 
		}
		break;

		case avdtp_state_open:
		case avdtp_state_streaming:
			report_close = TRUE;
			break;

		case avdtp_state_remote_closing:
		{
            if (link_loss)
                sendCloseInd(a2dp, a2dp_disconnect_link_loss);
            else
			    sendCloseInd(a2dp, a2dp_closed_by_remote_device);          
		}
		break;

		case avdtp_state_local_closing:
		{	
            if (link_loss)
                sendCloseCfm(a2dp, a2dp_disconnect_link_loss);
            else
			    sendCloseCfm(a2dp, a2dp_success);
		}
		break;
	}

	/* has the reset resulted in an unexpected close? */
	if (report_close)
    {
        if (link_loss)
            sendCloseInd(a2dp, a2dp_disconnect_link_loss);
        else
    		sendCloseInd(a2dp, a2dp_aborted);
    }

	/* Mark this SEP as not in use anymore */
	if (a2dp->sep.current_sep)
	{
		a2dp->sep.current_sep->configured = 0;
		a2dp->sep.current_sep = NULL;
	}
	
	/* 
		Close any open transport channels.
		This is slightly out of spec if we are the acceptor,
		but the other device isn't working too well anyway! 
	*/
	a2dpCloseMediaConnection(a2dp);

	/* 
		Move to idle state.
	*/
	setSignallingToIdle(a2dp);
}


/*****************************************************************************/
void a2dpHandleSignalConnectionTimeoutInd(A2DP* a2dp)
{
	if (a2dp->signal_conn.connection_state == avdtp_connection_connected)
	{
		/* Close the signalling channel */
		ConnectionL2capDisconnectRequest(&a2dp->task, a2dp->signal_conn.sink);

		/* Update the connection state */
		a2dpSetSignallingConnectionState(a2dp, avdtp_connection_disconnecting);
	}
}


/*****************************************************************************/
void a2dpGetCapsTimeout(A2DP* a2dp)
{
	if ((a2dp->signal_conn.signalling_state == avdtp_state_open) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_streaming) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_local_starting) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_local_suspending) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_reconfig_reading_caps) ||
		(a2dp->signal_conn.signalling_state == avdtp_state_reconfiguring)
		)
	{
		sendGetCurrentSepCapabilitiesCfm(a2dp, a2dp_operation_fail, 0, 0);
	}
}
