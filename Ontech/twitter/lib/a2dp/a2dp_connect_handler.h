/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_connect_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_CONNECT_HANDLER_H_
#define A2DP_CONNECT_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME	
    a2dpSendSignallingConnectInd

DESCRIPTION
    Send an A2DP_SIGNALLING_CHANNEL_CONNECT_IND message to the client task informing it of an 
    incoming AVDTP signalling connection. The client must respond to say whether it wants to accept this connection or not.

*/
void a2dpSendSignallingConnectInd(A2DP *a2dp, uint16 connection_id, bdaddr addr);


/****************************************************************************
NAME	
    a2dpSendSignallingConnectCfm

DESCRIPTION
    Send an A2DP_SIGNALLING_CHANNEL_CONNECT_CFM message to the client task informing it of the 
    outcome of the request to open an AVDTP signalling connection to a remote device.

*/
void a2dpSendSignallingConnectCfm(A2DP *a2dp, Task client, a2dp_status_code status, Sink sink);


/****************************************************************************
NAME	
    a2dpSetSignallingConnectionState

DESCRIPTION
    Changes the signalling connection state.

*/
void a2dpSetSignallingConnectionState(A2DP *a2dp, avdtp_connection state);


/****************************************************************************
NAME	
    a2dpOpenSignallingChannel

DESCRIPTION
    Request to open a signalling channel to the specified device 

*/
void a2dpOpenSignallingChannel(A2DP *a2dp, const bdaddr *addr);


/****************************************************************************
NAME	
    a2dpSignallingConnectSuccess

DESCRIPTION
    The signalling channel has been successfully connected.

*/
void a2dpSignallingConnectSuccess(A2DP *a2dp, Sink sink, uint16 mtu_remote);


/****************************************************************************
NAME	
    a2dpSignallingConnectFailure

DESCRIPTION
    The signalling channel has failed to connect.

*/
void a2dpSignallingConnectFailure(A2DP *a2dp, bool key_missing);


/****************************************************************************
NAME	
    a2dpSignalChannelOpen

DESCRIPTION
    This function is called when a signalling channel opens so we can perform
	a SEP discovery procedure.

*/
void a2dpSignalChannelOpen(A2DP *a2dp, bool connect_media);


/****************************************************************************
NAME	
    a2dpOpenTransportChannel

DESCRIPTION
    Request to open a transport channel to the specified device 

*/
void a2dpOpenTransportChannel(A2DP *a2dp, uint16 flush_timeout);


/****************************************************************************
NAME	
    a2dpMediaConnectSuccess

DESCRIPTION
    Media channel has been successfully connected.

*/
void a2dpMediaConnectSuccess(A2DP *a2dp, l2cap_connect_status status, Sink sink, uint16 remote_mtu);


/****************************************************************************
NAME	
    a2dpMediaConnectFailure

DESCRIPTION
    Media channel has failed to connect.

*/
void a2dpMediaConnectFailure(A2DP *a2dp, bool key_missing);


/****************************************************************************
NAME	
    a2dpCloseMediaConnection

DESCRIPTION
    Media channel should be closed.

*/
void a2dpCloseMediaConnection(A2DP *a2dp);


#endif /* A2DP_CONNECT_HANDLER_H_ */
