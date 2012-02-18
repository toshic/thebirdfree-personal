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

#include "a2dp_codec_handler.h"
#include "a2dp_delete.h"
#include "a2dp_connect_handler.h"
#include "a2dp_open_handler.h"
#include "a2dp_receive_packet_handler.h"
#include "a2dp_send_command_handler.h"
#include "a2dp_signalling_handler.h"

#include <print.h>
#include <sink.h>
#include <stream.h>


/*****************************************************************************/
static void sendOpenInd(A2DP *a2dp)
{
	MAKE_A2DP_MESSAGE(A2DP_OPEN_IND);
	message->seid = a2dp->sep.current_sep->sep_config->seid;
	message->media_sink = a2dp->media_conn.sink;
	message->a2dp = a2dp;
	MessageSend(a2dp->clientTask, A2DP_OPEN_IND, message);
}


/*****************************************************************************/
void a2dpSendSignallingConnectInd(A2DP *a2dp, uint16 connection_id, bdaddr addr)
{
    MAKE_A2DP_MESSAGE(A2DP_SIGNALLING_CHANNEL_CONNECT_IND);
    message->a2dp = a2dp;
    message->connection_id = connection_id;
	message->addr = addr;
    MessageSend(a2dp->clientTask, A2DP_SIGNALLING_CHANNEL_CONNECT_IND, message);
}

/*****************************************************************************/
void a2dpSendSignallingConnectCfm(A2DP *a2dp, Task client, a2dp_status_code status, Sink sink)
{
    MAKE_A2DP_MESSAGE(A2DP_SIGNALLING_CHANNEL_CONNECT_CFM);

	if (status != a2dp_success)
		message->a2dp = 0;
	else
		message->a2dp = a2dp;
    message->status = status;
    message->sink = sink;

    MessageSend(client, A2DP_SIGNALLING_CHANNEL_CONNECT_CFM, message);

    if (status != a2dp_success)
    {
		a2dp->signal_conn.connect_then_open_media = FALSE;
        if (a2dp->signal_conn.connection_state == avdtp_connection_connecting_crossover)
        {
            a2dpSetSignallingConnectionState(a2dp, avdtp_connection_connecting);
        }
        else if (a2dp)
		    a2dpDeleteTask(a2dp);
    }
}


/****************************************************************************/
void a2dpSetSignallingConnectionState(A2DP *a2dp, avdtp_connection state)
{
	PRINT(("a2dpSetSignallingConnectionState: O:%d N:%d\n", a2dp->signal_conn.connection_state, state));
	a2dp->signal_conn.connection_state = state;
}


/****************************************************************************/
void a2dpOpenSignallingChannel(A2DP *a2dp, const bdaddr *addr)
{
	/* Initiate the L2CAP connection */

	a2dpSetSignallingConnectionState(a2dp, avdtp_connection_connecting);
	
	PRINT(("a2dpOpenSignallingChannel: %x:%x:%lx\n", addr->nap, addr->uap, addr->lap));

    ConnectionL2capConnectRequest(&a2dp->task, addr, AVDTP_PSM, AVDTP_PSM, 0);
}


/****************************************************************************/
void a2dpSignallingConnectSuccess(A2DP *a2dp, Sink sink, uint16 mtu_remote)
{
	/* This side was waiting for the signalling connection, so it is now connected */
	a2dpSetSignallingConnectionState(a2dp, avdtp_connection_connected);

	a2dp->signal_conn.sink = sink;
	a2dp->signal_conn.mtu = mtu_remote;

	if (!a2dp->signal_conn.connect_then_open_media)
	{
		a2dpSendSignallingConnectCfm(a2dp, a2dp->clientTask, a2dp_success, sink);
	}

	a2dpSignalChannelOpen(a2dp, a2dp->signal_conn.connect_then_open_media);
}


/****************************************************************************/
void a2dpSignallingConnectFailure(A2DP *a2dp, bool key_missing)
{
    a2dp_status_code status = key_missing ? a2dp_key_missing : a2dp_operation_fail;
	if (a2dp->signal_conn.connect_then_open_media)
	{
		a2dpSendConnectOpenCfm(a2dp, a2dp->clientTask, status);
        
        if (a2dp->signal_conn.connection_state == avdtp_connection_connecting_crossover)
            a2dpSetSignallingConnectionState(a2dp, avdtp_connection_connecting);
        else
		    a2dpDeleteTask(a2dp);
	}
	else
	{
		a2dpSendSignallingConnectCfm(a2dp, a2dp->clientTask, status, 0);
	}
}


/****************************************************************************/
void a2dpSignalChannelOpen(A2DP *a2dp, bool connect_media)
{
	a2dpHandleNewSignalPacket(a2dp);	

	PRINT(("a2dpSignalChannelOpen conn media=%d\n",a2dp->signal_conn.connect_then_open_media));

	if (connect_media && (a2dp->signal_conn.signalling_state == avdtp_state_idle))
	{
		a2dpSetSignallingState(a2dp, avdtp_state_discovering);
	
		/* Signalling channel open - start discovery */
		if(!a2dpSendDiscover(a2dp))
			a2dpAbortSignalling(a2dp, FALSE, FALSE);
	}
}


/****************************************************************************/
void a2dpOpenTransportChannel(A2DP *a2dp, uint16 flush_timeout)
{
	bdaddr addr;
	Sink sink = a2dp->signal_conn.sink;

	if (!SinkGetBdAddr(sink, &addr))
		return;

	a2dp->media_conn.media_connecting = 1;

    /* Initiate the connection */
    if (flush_timeout == 0)
    {
        ConnectionL2capConnectRequest(&a2dp->task, &addr, AVDTP_PSM, AVDTP_PSM ,0);
    }
    else
    {
        l2cap_config_params p;
        /*
            Configure reasonable settings and specified flush
        */
        p.mtu_local = 895;
        p.mtu_remote_min = 48;
        p.flush_timeout = flush_timeout;   
        p.accept_non_default_flush = TRUE; 
        p.qos.service_type = 1; /* BEST EFFORT */
        p.qos.token_rate = 0;
        p.qos.token_bucket = 0;
        p.qos.peak_bw = 0;        
        p.qos.latency = 0xFFFFFFFF;        
        p.qos.delay_var = 0xFFFFFFFF;
        p.accept_qos_settings = TRUE;
        p.timeout = DEFAULT_L2CAP_CONNECTION_TIMEOUT;
        ConnectionL2capConnectRequest(&a2dp->task, &addr, AVDTP_PSM, AVDTP_PSM ,&p);
    }
}


/****************************************************************************/
void a2dpMediaConnectSuccess(A2DP *a2dp, l2cap_connect_status status, Sink sink, uint16 remote_mtu)
{
	a2dp->media_conn.media_connecting = 0;

	/* Deal with the case where the signalling has been disconnected or no Stream
		End Point has been configured. In both cases disconnect the media channel. */
	if (!a2dp->signal_conn.sink || !a2dp->sep.configured_service_caps)
	{
		ConnectionL2capDisconnectRequest(&a2dp->task, sink);
		return;
	}

	a2dp->media_conn.sink = sink;
	a2dp->media_conn.mtu = remote_mtu;
	a2dp->media_conn.media_connected = 1;

	PRINT(("connect=%x\n",(uint16)a2dp->media_conn.sink));
	
	/* Start disposing of data coming on the source */
	(void) StreamConnectDispose(StreamSourceFromSink(sink));
	
	/* Choose the codec params that the app needs to know about and send it this information */
	a2dpSendCodecAudioParams(a2dp);

	if (a2dp->signal_conn.signalling_state == avdtp_state_remote_opening)
	{					
		/* Send A2DP_OPEN_IND to client */
		sendOpenInd(a2dp);
	}
	else
	{
		if (a2dp->signal_conn.connect_then_open_media)
			/* Send A2DP_CONNECT_OPEN_CFM to client */
			a2dpSendConnectOpenCfm(a2dp, a2dp->clientTask, a2dp_success);
		else
			/* Send A2DP_OPEN_CFM to client */
			a2dpSendOpenCfm(a2dp, a2dp->clientTask, a2dp_success);
	}
	
	/* Move to the open state */
	a2dpSetSignallingState(a2dp, avdtp_state_open);
	
	MessageSend((Task)&a2dp->task, A2DP_INTERNAL_SIGNAL_PACKET_IND, 0);
}


/****************************************************************************/
void a2dpMediaConnectFailure(A2DP *a2dp, bool key_missing)
{
	a2dp->media_conn.media_connecting = 0;

	if ((a2dp->signal_conn.sink == 0) && (!a2dp->signal_conn.waiting_response))
	{
		a2dpDeleteTask(a2dp);
		return;
	}

	a2dpAbortSignalling(a2dp, FALSE, key_missing);
}


/*****************************************************************************/
void a2dpCloseMediaConnection(A2DP *a2dp)
{
	PRINT(("a2dpCloseMediaConnection\n"));
	if (a2dp->media_conn.media_connected)
		/* If we have a media channel close it */
		ConnectionL2capDisconnectRequest(&a2dp->task, a2dp->media_conn.sink);
	else
		a2dp->media_conn.sink = 0;
}
