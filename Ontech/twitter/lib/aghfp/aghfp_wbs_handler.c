/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	aghfp_wbs_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_common.h"
#include "aghfp_wbs.h"
#include "aghfp_wbs_handler.h"
#include "aghfp_send_data.h"
#include "aghfp_ok.h"
#include "aghfp_call_manager.h"
#include "aghfp_audio_handler.h"

#include <panic.h>
#include <stdio.h>
#include <string.h>

/* WB-Speech parameters for an eSCO connection.  */
static const aghfp_audio_params wb_speech_esco_audio_params_sbc =
{
	8000,						/* Bandwidth for both Tx and Rx */
	0x000e, 					/* Max Latency					*/
	sync_air_coding_transparent,/* Voice Settings				*/
	sync_retx_link_quality,		/* Retransmission Effort		*/
	FALSE						/* Use WB-Speech if available	*/
};

static const aghfp_audio_params wb_speech_esco_audio_params_amr =
{
    3000,                    	/* Bandwidth for both Tx and Rx */
    0x000e,                  	/* Max Latency                  */
    sync_air_coding_transparent,/* Voice Settings               */
    sync_retx_link_quality,    	/* Retransmission Effort        */
    FALSE				     	/* Use WB-Speech if available   */
};


/*
	Handle Codec Connection request from the HF (AT+BCC).
	This function performs the the actual actions of handling the AT+BCC (once the audio params have been
	obtained from the app, i.e. after aghfpHandleWbsCodecConReq() and ).
*/
static void aghfpHandleWbsCodecConReqProcessing(AGHFP *aghfp)
{
	/* Are we supporting WBS */
	if (aghfp->use_wbs)
	{
		/* OK to initiate WBS Codec Negotiation. */
	
		/* Send OK */
		aghfpSendOk(aghfp);
	
		/* If we haven't negotiatied a codec we need to do so.
		   If we have, we're gonna go ahead and use it here. */
		if (aghfp->use_codec == 0)
		{
			/* Start codec negotiation with HF. */
			if (aghfpWbsStartCodecNegotiation(aghfp, aghfp_negotiate_audio_at_hf))
			{
				aghfp->audio_connection_state = aghfp_audio_codec_connect;
			}
		}
		else
		{
			if (!aghfpCallManagerActiveNotComplete(aghfp))
			{
				MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AUDIO_CONNECT_REQ);
				AGHFP_DEBUG(("	  Normal Audio Connection\n"));
				
				message->audio_params = aghfp->audio_params;
				message->packet_type = aghfp->audio_packet_type;
				MessageSend(&aghfp->task, AGHFP_INTERNAL_AUDIO_CONNECT_REQ, message);
			}
			else
			{
				/* Save the audio packet type for Call Manager */
				aghfpStoreAudioParams(aghfp, aghfp->audio_packet_type, &aghfp->audio_params);
				
				/* Answer the call again now that the WBS negotiation is complete. */
				aghfpManageCall(aghfp, CallEventAnswer, CallFlagOpenAudio);
			}
		}
	}
	else
	{
		/* WBS not supported, send ERROR. */
		aghfpSendError(aghfp);
	}
}

/*
	Start Codec negotiation with HF
*/
bool aghfpWbsStartCodecNegotiation(AGHFP *aghfp, aghfp_wbs_negotiate_action wbs_negotiate_action)
{
	char bcs[15];
	uint16 codec; 		/* Codec bitmap used internally */
	uint16 codecUUID16; /* Used to store the actual codec UUID16 */
	
	/*	find highest codec supported by both ends*/
	codec = aghfp->codecs_info.ag_codecs & aghfp->hf_codecs;

	AGHFP_DEBUG(("aghfpWbsStartCodecNegotiation\n"));
	
	if ((codec & aghfp->codec_to_negotiate) == 0)
		return FALSE;

	/* If we get here then the selected codec to negotiate is ok. */
 	
	/* Remember the codec we wish to use */
	aghfp->use_codec = aghfp->codec_to_negotiate;
	
	/* Translate from the internal bitmap format (only one bit should be set in 'aghfp->use_codec') into codec UUID16. */
	codecUUID16 = WbsCodecToUuid16((codecs_info*)(&aghfp->codecs_info), aghfp->use_codec);

	/* Create the AT cmd we're sending */
	aghfpAtCmdBegin(aghfp);
	aghfpAtCmdString(aghfp, "+BCS=");

	sprintf(bcs, "%d", codecUUID16);

	aghfpAtCmdString(aghfp, bcs);
	aghfpAtCmdEnd(aghfp);
	
	aghfp->wbs_negotiate_action = wbs_negotiate_action;
	
	return TRUE;
}

/*
	Handle set audio parameters request from the app.
*/
void aghfpHandleSetAudioParamsReq(AGHFP *aghfp, const AGHFP_INTERNAL_SET_AUDIO_PARAMS_REQ_T *req)
{
	/* Only act upon the apps request if in the correct state. */
	switch ( aghfp->audio_connection_state )
	{
	case aghfp_audio_disconnected:
	case aghfp_audio_codec_connect:
		aghfpStoreAudioParams(aghfp, req->packet_type, &req->audio_params);

		/* Continue the Codec Connection */
		aghfpHandleWbsCodecConReqProcessing(aghfp);
		break;
	case aghfp_audio_connecting_esco:
	case aghfp_audio_connecting_sco:
	case aghfp_audio_accepting:
	case aghfp_audio_disconnecting:
	case aghfp_audio_connected:
	default:
		AGHFP_DEBUG(("aghfpHandleAudioConnectReq invalid state %d\n",aghfp->audio_connection_state));
		break;
	}
}

/*
	Handle Codec Negotiation resopnse from the HF (AT+BCS)
*/
void aghfpHandleWbsCodecNegReq(AGHFP *aghfp, uint16 codecUUID16)
{
    sync_pkt_type		audio_packet_type = aghfp->audio_packet_type;
    aghfp_audio_params	audio_params = aghfp->audio_params;
	uint16 codec; 		/* Codec bitmap used internally */
	
    AGHFP_DEBUG(("aghfpHandleWbsCodecNegReq : "));
    
	/* Translate from codec UUID16 format to internal bitmap. */
	codec = WbsUuid16ToCodec((codecs_info*)&(aghfp->codecs_info), codecUUID16);

	/* Note that one bit should be set in the bitmaps and it must be the same bit for success. */
	if(aghfp->use_wbs && (aghfp->use_codec == codec))
	{
		AGHFP_DEBUG(("Starting WB-Speech eSCO\n"));
		
		/* Send OK */
		aghfpSendOk(aghfp);

		/* audio_packet_type and audio_params already set to WBS values. */
		
		/* Force connection create.
		   WBS SCO connection are always initiated by the AG. */
		if(aghfp->wbs_negotiate_action != aghfp_negotiate_no_audio)
		{
			aghfp->wbs_negotiate_action = aghfp_negotiate_audio_at_ag;
		}
	}
	else
	{
		AGHFP_DEBUG(("Falling back to request eSCO type\n"));
		/* send Error */
		aghfpSendError(aghfp);

		/* Reset any saved codecs; Codec Negotiation aborted. */
		aghfp->use_codec = 0;
		
		/* For normal SCO we only set up the SCO here if the AG initiated the connection. */
		if (aghfp->wbs_negotiate_action == aghfp_negotiate_audio_at_ag)
		{ /* Start eSCO Connection */
	        audio_packet_type = aghfp->audio_packet_type;
	        audio_params = aghfp->audio_params;
	        audio_params.override_wbs = TRUE;
        }
	}
	
	if (aghfp->wbs_negotiate_action == aghfp_negotiate_audio_at_ag)
	{ /* Start eSCO Connection. We get here if we are setting up a WBS link or an AG initiated normal SCO. */
		if (!aghfpCallManagerActiveNotComplete(aghfp))
		{
		    MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AUDIO_CONNECT_REQ);
		    AGHFP_DEBUG(("    Normal Audio Connection\n"));
		    
			message->audio_params = audio_params;
			message->packet_type = audio_packet_type;
	        MessageSend(&aghfp->task, AGHFP_INTERNAL_AUDIO_CONNECT_REQ, message);
		}
		else
		{
			/* Save the audio packet type for Call Manager */
			aghfpStoreAudioParams(aghfp, audio_packet_type, &audio_params);
			
			/* Answer the call again now that the WBS negotiation is complete. */
			aghfpManageCall(aghfp, CallEventAnswer, CallFlagOpenAudio);
        }
	}
	
	aghfp->wbs_negotiate_action = aghfp_negotiate_undefined; /* Reset flag to ensure it is correct next time */
}

/*
	Get pointer to WB-Speech eSCO Parameters
*/
aghfp_audio_params *aghfpGetWbsParameters(AGHFP *aghfp)
{
	switch(aghfp->use_codec)
	{
		case(wbs_codec_mask_sbc):
			return((aghfp_audio_params*)&wb_speech_esco_audio_params_sbc);
			break;
		case(wbs_codec_mask_amr_wb):
			return((aghfp_audio_params*)&wb_speech_esco_audio_params_amr);
			break;
		case(wbs_codec_mask_cvsd):
			/* This relies on audio parameters having been stored previously using aghfpStoreAudioParams() */
			return(&aghfp->audio_params);
		default:
		case(wbs_codec_mask_evrc_wb):
			AGHFP_DEBUG(("Codec not supported.\n"));
			Panic();
			return(NULL);
			break;
	}
}

/*
	Get WB-Speech packet type
*/
sync_pkt_type aghfpGetWbsPacketType(AGHFP *aghfp)
{
	switch(aghfp->use_codec)
	{
		case(wbs_codec_mask_sbc):
		case(wbs_codec_mask_amr_wb):
			return(WBS_PACKET_TYPE);
			break;
		case(wbs_codec_mask_cvsd):
			/* This relies on audio parameters having been stored previously using aghfpStoreAudioParams() */
			return(aghfp->audio_packet_type);
		default:
		case(wbs_codec_mask_evrc_wb):
			AGHFP_DEBUG(("Codec not supported.\n"));
			Panic();
			return(0);
			break;
	}
}

/****************************************************************************
NAME	
	aghfpHandleCodecNegotiationReq

DESCRIPTION
	Update the AGHFP's information regarding the HF's codecs and send an OK AT command.
	This is a response to the AT+BAC command.

RETURNS
	void
*/

void aghfpHandleCodecNegotiationReq(AGHFP *aghfp, AGHFP_INTERNAL_CODEC_NEGOTIATION_REQ_T *codecs)
{
	uint16 counter_1;

	aghfp->hf_codecs = 0;

	/* Convert from a list of UUID16s to a codec mask. */
	for(counter_1 = 0; counter_1 < codecs->num_codecs; counter_1++)
	{
		aghfp->hf_codecs |= WbsUuid16ToCodec((codecs_info*)(&aghfp->codecs_info), codecs->codec_uuids[counter_1]);
	}

	/* Check audio (and AGHFP state?)
	   Restart codec negotiation if in the middle of WBS connect. */
	switch(aghfp->audio_connection_state)
	{
		case(aghfp_audio_disconnected):
			aghfpSendOk(aghfp);
			break;
		case(aghfp_audio_codec_connect):
			/* Start codec negotiation with HF. */
			if (aghfpWbsStartCodecNegotiation(aghfp, aghfp_negotiate_audio_at_ag))
			{
				aghfp->audio_connection_state = aghfp_audio_codec_connect;
			} /* Ignore error case for now. */
			break;
		case(aghfp_audio_connecting_esco):
		case(aghfp_audio_connecting_sco):
		case(aghfp_audio_accepting):
		case(aghfp_audio_disconnecting):
		case(aghfp_audio_connected):
			break;
	}
}
/*
	Handle Codec Connection request from the HF (AT+BCC).
	Ask the app for its configured audio parameters.
*/
void aghfpHandleWbsCodecConReq(AGHFP *aghfp)
{
	MessageSend(aghfp->client_task, AGHFP_APP_AUDIO_PARAMS_REQUIRED_IND, 0);
}

void aghfpEnableWbs(AGHFP *aghfp)
{
	if (aghfp->codecs_info.ag_codecs)
	{ /* WB-Speech Supported */
		AGHFP_DEBUG(("WB-Speech Local Codecs [0x%x]\n", aghfp->codecs_info.ag_codecs));
		aghfp->use_wbs = TRUE;
	}
	else
	{
		AGHFP_DEBUG(("Wbs - Not Supported, Disabling WB-Speech\n"));
		aghfp->use_wbs = FALSE;
	}
}

void aghfpDisableWbs(AGHFP *aghfp)
{
	AGHFP_DEBUG(("Wbs - Disabling WB-Speech\n"));
	aghfp->use_wbs = FALSE;

	/* Clear any negotiated codec. */
	aghfp->use_codec = 0;
}

/*
	Sets the codec to use.
*/
void AghfpSetCodecType(AGHFP *aghfp, aghfp_wbs_codec codec)
{
	wbs_codec_mask codec_mask;

	aghfp->app_pending_codec_negotiation = FALSE;

	/* Convert to internal codec mask format. */
	switch(codec)
	{
		case(aghfp_wbs_codec_cvsd):
			codec_mask = wbs_codec_mask_cvsd;
			/* CVSD is a special case. It may be negotiated by the app. A check of this flag
			   in combination with aghfp->negotiation_type will determine which negotiation method to use. */
			aghfp->app_pending_codec_negotiation = TRUE;
			break;
		case(aghfp_wbs_codec_sbc):
			codec_mask = wbs_codec_mask_sbc;
			break;
		case(aghfp_wbs_codec_amr):
			codec_mask = wbs_codec_mask_amr_wb;
			break;
		case(aghfp_wbs_codec_evrc):
			codec_mask = wbs_codec_mask_evrc_wb;
			break;
		default:
			aghfp->app_pending_codec_negotiation = TRUE;
			codec_mask = 0;
			break;
	}

	/* Always re-negotiate the codec even if it has not changed. Rely on callers of this function
	   to protect against needlessly re-negotiating a codec. This has been left in to cope with
	   CVSD which can be noegitiated either by AGHFP or the app. */
	aghfp->codec_to_negotiate = codec_mask;

	/* Signify that we need to negotiate a new codec at the AGHFP level. */
	aghfp->use_codec = 0;
}

/*
	Sets the codec negotiation type to use.
*/
void AghfpSetNegotiationType(AGHFP *aghfp, aghfp_codec_negotiation_type negotiation_type)
{
	aghfp->negotiation_type = negotiation_type;
}

/*
	Dynamically enables WBS codec negotiation. This function can only has an affect
	if the AGHFP's state is 'aghfp_ready', i.e. no SLC connection.
*/
void AghfpEnableWbsCodecNegotiation(AGHFP *aghfp)
{
	if(aghfp->state == aghfp_ready)
	{
		aghfpEnableWbs(aghfp);

		/* Enable WBS codec negotiation in the AGHFP's supported features */
		aghfp->supported_features |= aghfp_codec_negotiation;
	}
}

/*
	Dynamically dsiables WBS codec negotiation. This function can only has an affect
	if the AGHFP's state is 'aghfp_ready', i.e. no SLC connection.
*/
void AghfpDisableWbsCodecNegotiation(AGHFP *aghfp)
{
	if(aghfp->state == aghfp_ready)
	{
		aghfpDisableWbs(aghfp);

		/* Disable WBS codec negotiation in the AGHFP's supported features */
		aghfp->supported_features &= ~(aghfp_codec_negotiation);
	}
}

/*
	Start WBS negotiation if appropriate.
*/
void AghfpStartWbsCodecNegotiation(AGHFP *aghfp)
{
	if(aghfp->state >= aghfp_slc_connected)
	{
		aghfpWbsStartCodecNegotiation(aghfp, aghfp_negotiate_no_audio);
	}
}

/*
	Gets the WBS codecs supported by the connected HF.
*/
unsigned int AghfpGetHfWbsCodecsSupported(AGHFP * aghfp)
{
	if(aghfp->state >= aghfp_slc_connected)
	{
		return(aghfp->hf_codecs);
	}
	else
	{
		return(0);
	}
}

/*
	Indicates whether a Wbs has been negotiated.
		TRUE - WBS codec has been negotiated
		FALSE - WBS codec has NOT been negotiated
*/
bool AghfpCodecHasBeenNegotiated(AGHFP * aghfp, uint8 *wbs_codec)
{
	return(aghfpCodecBitmapToAghfpEnum(aghfp->use_codec, wbs_codec));
}

