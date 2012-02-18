/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_l2cap_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_connect_handler.h"
#include "a2dp_delete.h"
#include "a2dp_init.h"
#include "a2dp_l2cap_handler.h"
#include "a2dp_profile_handler.h"
#include "a2dp_receive_packet_handler.h"
#include "a2dp_signalling_handler.h"

#include <print.h>
#include <stdlib.h>


/*****************************************************************************/
const profile_task_recipe a2dp_recipe = {
    sizeof(A2DP),
    A2DP_INTERNAL_TASK_INIT_REQ,
    {a2dpProfileHandler},
	2,
    0
};


static a2dp_status_code convertDisconnectStatusCode(l2cap_disconnect_status l2cap_status)
{
	switch (l2cap_status)
	{
	case l2cap_disconnect_successful:
		return a2dp_success;	
	case l2cap_disconnect_no_connection:
		return a2dp_no_signalling_connection;
	case l2cap_disconnect_link_loss:
		return a2dp_disconnect_link_loss;
	case l2cap_disconnect_timed_out:
	case l2cap_disconnect_error:	
	default:
		return a2dp_operation_fail;
	}
}


static void sendSignallingCloseInd(A2DP* a2dp, Sink sink, a2dp_status_code status)
{
	MAKE_A2DP_MESSAGE(A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND);
	message->a2dp = a2dp;
	message->sink = sink;
	message->status = status;
	MessageSend(a2dp->clientTask, A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND, message);
}		


/*****************************************************************************/
void a2dpRegisterL2cap(A2DP* a2dp)
{
	ConnectionL2capRegisterRequestLazy(&a2dp->task, a2dp->clientTask, AVDTP_PSM, &a2dp_recipe);
}


/*****************************************************************************/
void a2dpHandleL2capRegisterCfm(A2DP *a2dp, const CL_L2CAP_REGISTER_CFM_T *cfm)
{
    /* Send a confirmation message to the client regardless of the outcome */
    a2dpSendInitCfmToClient(a2dp, (cfm->status == success) ? a2dp_success : a2dp_l2cap_fail, a2dp->sep.sep_list);
}


/*****************************************************************************/
void a2dpHandleL2capConnectInd(A2DP *a2dp, const CL_L2CAP_CONNECT_IND_T *ind)
{
	bool accept = FALSE;

	if(ind->psm == AVDTP_PSM)
	{
		if (a2dp->signal_conn.connection_state == avdtp_connection_connected)
		{
			/* Must be the media channel, always accept this immediately as long as not
                already in the process of connecting it */
            if (!a2dp->media_conn.media_connecting)
            {
			    a2dp->media_conn.media_connecting = 1;
			    accept = TRUE;
            }
            else
            {
                accept = FALSE;
            }
		}
		else
		{
			if ((a2dp->signal_conn.connection_state != avdtp_connection_idle) &&
                (a2dp->signal_conn.connection_state != avdtp_connection_connecting))
			{
				accept = FALSE;
			}
			else
			{
                if (a2dp->signal_conn.connection_state == avdtp_connection_connecting)
                    a2dpSetSignallingConnectionState(a2dp, avdtp_connection_connecting_crossover);
                else
				    a2dpSetSignallingConnectionState(a2dp, avdtp_connection_connecting);
				a2dp->signal_conn.waiting_response++;
				/* This must be the signalling channel, so the app must decide if to accept this connection */
				a2dpSendSignallingConnectInd(a2dp, ind->connection_id, ind->bd_addr);
				return;
			}
		}
	}

	/* Send the connect response to the connection lib */
	ConnectionL2capConnectResponse(&a2dp->task, accept, ind->psm, ind->connection_id, NULL);
}


/*****************************************************************************/
void a2dpHandleL2capConnectCfm(A2DP *a2dp, const CL_L2CAP_CONNECT_CFM_T *cfm)
{
	if (cfm->status == l2cap_connect_success)
	{
		switch (a2dp->signal_conn.connection_state)
		{
		case avdtp_connection_connecting:
        case avdtp_connection_connecting_crossover:
			a2dpSignallingConnectSuccess(a2dp, cfm->sink, cfm->mtu_remote);
			break;
		case avdtp_connection_idle:
		case avdtp_connection_connected:
		case avdtp_connection_disconnecting:
		default:
			/* Signalling already connected so this is the media channel connected */
			a2dpMediaConnectSuccess(a2dp, cfm->status, cfm->sink, cfm->mtu_remote);
			break;
		}

		/* Check for data in the buffer */
		a2dpHandleSignalPacket(a2dp, StreamSourceFromSink(cfm->sink));
	}
	else
	{
        bool key_missing = (bool)(cfm->status == l2cap_connect_failed_key_missing);
        
		switch (a2dp->signal_conn.connection_state)
		{
		case avdtp_connection_connecting:
        case avdtp_connection_connecting_crossover:
			a2dpSignallingConnectFailure(a2dp, key_missing);
			break;
		case avdtp_connection_idle:
		case avdtp_connection_connected:
		case avdtp_connection_disconnecting:
		default:
			/* Signalling already connected so this is the media channel connect failure */
			a2dpMediaConnectFailure(a2dp, key_missing);
			break;
		}
	}
}


/****************************************************************************/
void a2dpHandleL2capDisconnectInd(A2DP* a2dp, const CL_L2CAP_DISCONNECT_IND_T* ind)
{
	if (ind->sink == a2dp->signal_conn.sink)
	{
		/* Signalling disconnected */

		bool link_loss = FALSE;

        if (ind->status == l2cap_disconnect_link_loss)
            link_loss = TRUE;
		
		PRINT(("Signalling disconnect %d\n",ind->status));

		if (!a2dp->signal_conn.connect_then_open_media)
			sendSignallingCloseInd(a2dp, ind->sink, convertDisconnectStatusCode(ind->status));

		/* Update the connection state to disconnecting so no other devices can connect while this A2DP instance exists (it will soon be deleted) */
		a2dpSetSignallingConnectionState(a2dp, avdtp_connection_disconnecting);

		a2dp->signal_conn.sink = 0;

        /* SEP is useless without a signalling channel, so Reset it */
		a2dpResetSignalling(a2dp, link_loss, FALSE);
	}
	else if (ind->sink == a2dp->media_conn.sink)
	{
		/* Media disconnected */
		
		PRINT(("Media disconnect %d\n",ind->status));

		a2dp->media_conn.media_connected = 0;

		if(ind->status == l2cap_disconnect_successful)
		{
			if((a2dp->signal_conn.signalling_state == avdtp_state_local_closing) ||
				(a2dp->signal_conn.signalling_state == avdtp_state_remote_closing) ||
				(a2dp->signal_conn.signalling_state == avdtp_state_local_aborting) ||
				(a2dp->signal_conn.signalling_state == avdtp_state_remote_aborting))
			{
				/* SEP is now released, so reset it */
				a2dpResetSignalling(a2dp, FALSE, FALSE);	
			}
		}
		else
		{        
			bool link_loss = FALSE;

			if (ind->status == l2cap_disconnect_link_loss)
				link_loss = TRUE;

			/* The disconnect was not signalled so kill the SEP to Abort */
			a2dpAbortSignalling(a2dp, link_loss, FALSE);	
		}
		a2dp->media_conn.media_connecting = 0;
	}
	else
	{
		/* Unknown sink, but could be the media if the lib has already reset its states due to a signalling disconnect */
		a2dp->media_conn.media_connecting = 0;
	}
	
	if (a2dp->signal_conn.connection_state == avdtp_connection_connecting_crossover)
	{
		/* If two signalling connections ongoing and one of them disconnects,
		   then only update the signalling state. The A2DP instance shouldn't be deleted. */
		a2dpSetSignallingConnectionState(a2dp, avdtp_connection_connecting);
	}
	else
	{
		/* Remove A2DP instance if no connections. Don't delete if it's in the middle of opening a stream though. */
		if ((a2dp->signal_conn.sink == 0) && 
			(!a2dp->media_conn.media_connected) && 
			(!a2dp->media_conn.media_connecting) &&
			(!a2dp->signal_conn.waiting_response)
		)
		{
			a2dpDeleteTask(a2dp);
		}
	}
}
