/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_audio.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"

#include <panic.h>


/* Default parameters for an eSCO connection.  These are the "S1" safe settings 
   for an EV3 packet type as defined in the HFP spec.  All HFP v1.5 devices should
   support these parameters for an EV3 packet type.                                */
static const hfp_audio_params default_esco_audio_params =
{
    8000,                    /* Bandwidth for both Tx and Rx */
    0x000a,                  /* Max Latency                  */
    sync_air_coding_cvsd,    /* Voice Settings               */
    sync_retx_power_usage    /* Retransmission Effort        */
};

/* Default parameters for a SCO connection.  The max latency is sufficient to 
   support all SCO packet types - HV1, HV2 and HV3.                           */
static const hfp_audio_params default_sco_audio_params =
{
    8000,                    /* Bandwidth for both Tx and Rx */
    0x0005,                  /* Max Latency                  */
    sync_air_coding_cvsd,    /* Voice Settings               */
    sync_retx_disabled       /* Retransmission Effort        */
};

/****************************************************************************
NAME	
	HfpAudioTransferConnection

DESCRIPTION
    

MESSAGE RETURNED
    none.

RETURNS
	void
*/
void HfpAudioTransferConnection(HFP *hfp, hfp_audio_transfer_direction direction, sync_pkt_type packet_type, const hfp_audio_params *audio_params)
{
#ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
    if ( direction > hfp_audio_transfer )
    {
        HFP_DEBUG(("Invalid direction passed in.\n"));
    }
#endif

    {
        /* Send an internal message */
        MAKE_HFP_MESSAGE(HFP_INTERNAL_AUDIO_TRANSFER_REQ);
        message->direction = direction;
        message->packet_type = packet_type;
        if ( audio_params )
        {
            message->audio_params = *audio_params;
        }
        else
        {
            if ( packet_type & sync_all_esco )
            {
                message->audio_params = default_esco_audio_params;
            }
            else
            {
                message->audio_params = default_sco_audio_params;
            }
        }    
    
        MessageSend(&hfp->task, HFP_INTERNAL_AUDIO_TRANSFER_REQ, message);
    }
}


/****************************************************************************
NAME	
	HfpAudioConnect

DESCRIPTION
	Create an audio connection to the AG for the specified profile instance. 
	
    The audio_params structure is used to specify the connection parameters for
    setting up either a SCO or eSCO link. It is the application's responsibility
    to ensure that the remote device supports the requested parameters.

MESSAGE RETURNED
	HFP_AUDIO_CONNECT_CFM.

RETURNS
	void
*/
void HfpAudioConnect(HFP *hfp, sync_pkt_type packet_type, const hfp_audio_params *audio_params)
{
#ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif

    {
        /* Send an internal message */
        MAKE_HFP_MESSAGE(HFP_INTERNAL_AUDIO_CONNECT_REQ);
        message->packet_type = packet_type;
        if ( audio_params )
        {
            message->audio_params = *audio_params;
        }
        else
        {
            if ( packet_type & sync_all_esco )
            {
                message->audio_params = default_esco_audio_params;
            }
            else
            {
                message->audio_params = default_sco_audio_params;
            }
        }    
    
        MessageSend(&hfp->task, HFP_INTERNAL_AUDIO_CONNECT_REQ, message);
    }
}


/****************************************************************************
NAME	
	HfpAudioConnectResponse

DESCRIPTION
    Used by the application to respond to an incoming connection request from
    the AG.  The application will receive a HFP_AUDIO_CONNECT_IND to indicate
    that the AG is attempting to initiate a connection.
    
	The response is used to accept or reject the incoming connection request.

    The audio_params structure is used to specify the acceptable range of
    connection parameters when accepting an incoming request. It is the 
    application's responsibility to ensure that the remote device supports the 
    specified parameters.

MESSAGE RETURNED
	HFP_AUDIO_CONNECT_CFM.

RETURNS
	void
*/
void HfpAudioConnectResponse(HFP *hfp, bool response, sync_pkt_type packet_type, const hfp_audio_params *audio_params, bdaddr bd_addr)
{
#ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif

    {
        /* Send an internal message */
        MAKE_HFP_MESSAGE(HFP_INTERNAL_AUDIO_CONNECT_RES);
        message->response = response;
        message->packet_type = packet_type;
        message->bd_addr = bd_addr;
        if ( audio_params )
        {
            message->audio_params = *audio_params;
        }
        else
        {
            if ( packet_type & sync_all_esco)
            {
                message->audio_params = default_esco_audio_params;
            }
            else
            {
                message->audio_params = default_sco_audio_params;
            }
        }
    
        MessageSend(&hfp->task, HFP_INTERNAL_AUDIO_CONNECT_RES, message);
    }
}


/****************************************************************************
NAME	
	HfpAudioDisconnect

DESCRIPTION
	Disconnect the audio connection to the AG for the specified profile instance.
	
MESSAGE RETURNED
	HFP_AUDIO_DISCONNECT_IND.

RETURNS
	void
*/
void HfpAudioDisconnect(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif

    /* Send an internal message */
    MessageSend(&hfp->task, HFP_INTERNAL_AUDIO_DISCONNECT_REQ, 0);
}


