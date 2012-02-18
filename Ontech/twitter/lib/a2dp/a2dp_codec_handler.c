/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_sep_handler.c        

DESCRIPTION
	This file contains 

NOTES

*/


/****************************************************************************
	Header files
*/

#include "a2dp_codec_handler.h"

#include "a2dp_caps_parse.h"
#include "a2dp_aac.h"
#include "a2dp_atrac.h"
#include "a2dp_csr_faststream.h"
#include "a2dp_mp3.h"
#include "a2dp_reconfigure_handler.h"
#include "a2dp_sbc.h"
#include "a2dp_send_command_handler.h"
#include "a2dp_signalling_handler.h"

#include <string.h>
#include <stdlib.h>


/*****************************************************************************/
static bool getCodecFromCaps(const uint8 *local_caps, uint8 *codec_type)
{
	const uint8 *local_codec = local_caps;

	/* find the codec specific info in caps */
	if (!a2dpFindCodecSpecificInformation(&local_codec,0))
		return FALSE;

	/* return the codec type (SBC, MP3, etc) */
	*codec_type = local_codec[3];
	return TRUE;
}


static bool isCodecCsrFaststream(const uint8 *local_caps)
{
	const uint8 *local_codec = local_caps;
	uint32 local_vendor_id;
    uint16 local_codec_id;

	/* find the codec specific info in caps */
	if (!a2dpFindCodecSpecificInformation(&local_codec,0))
		return FALSE;

	local_vendor_id = a2dpConvertUint8ValuesToUint32(&local_codec[4]);
	local_codec_id = (local_codec[8] << 8) | local_codec[9];

	if ((local_vendor_id == A2DP_CSR_VENDOR_ID) && (local_codec_id == A2DP_CSR_FASTSTREAM_CODEC_ID))
		return TRUE;

	return FALSE;
}


/*****************************************************************************/
void processCodecInfo(A2DP *a2dp, bool accept, uint16 size_codec_service_caps, const uint8 *codec_service_caps)
{
	if (accept)
	{
		bool response = FALSE;

		/* Update the SEP state */
		a2dpSetSignallingState(a2dp, avdtp_state_configuring);
		
		response = a2dpSendSetConfiguration(a2dp, codec_service_caps, size_codec_service_caps);

		if (!response)
			/* Failed to start - abort */
			a2dpAbortSignalling(a2dp, FALSE, FALSE);
	}
	else
	{
		/* Failed initial configuration - reset SEP. */
		a2dpResetSignalling(a2dp, FALSE, FALSE);
	}		
}


/*****************************************************************************/
bool a2dpHandleSelectingCodecSettings(A2DP *a2dp, uint16 size_service_caps, uint8 *service_caps)
{
	uint8 codec_type;
    bool accept = FALSE;

	if (getCodecFromCaps(a2dp->sep.current_sep->sep_config->caps, &codec_type))
	{
		/* If this is a sink we need to inform the client of the codec settings */
		if (a2dp->sep.current_sep->sep_config->role == a2dp_sink)
		{
			/* Determine the optimal codec settings */
			switch (codec_type)
			{
			case AVDTP_MEDIA_CODEC_SBC:
				selectOptimalSbcCapsSink(a2dp->sep.current_sep->sep_config->caps, service_caps);
				accept = TRUE;
				break;
			case AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO:
				selectOptimalMp3CapsSink(a2dp->sep.current_sep->sep_config->caps, service_caps);
				accept = TRUE;
				break;
			case AVDTP_MEDIA_CODEC_MPEG2_4_AAC:
				selectOptimalAacCapsSink(service_caps);
				accept = TRUE;
				break;
#ifdef A2DP_ATRAC				
			case AVDTP_MEDIA_CODEC_ATRAC:
				selectOptimalAtracCapsSink(a2dp->sep.current_sep->sep_config->caps, service_caps);
				accept = TRUE;
				break;
#endif /* A2DP_ATRAC */				
			case AVDTP_MEDIA_CODEC_NONA2DP:
				if (isCodecCsrFaststream(a2dp->sep.current_sep->sep_config->caps))
				{
					selectOptimalCsrFastStreamCapsSink(a2dp->sep.current_sep->sep_config->caps, service_caps);
					accept = TRUE;
				}
				break;
			default:
				break;
			}
		}
		else
		{
			/* Local device is a source of one type or another */
			switch (codec_type)
			{
			case AVDTP_MEDIA_CODEC_SBC:
				selectOptimalSbcCapsSource(a2dp->sep.current_sep->sep_config->caps, service_caps);
				accept = TRUE;
				break;
			case AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO:
				selectOptimalMp3CapsSource(a2dp->sep.current_sep->sep_config->caps, service_caps);
				accept = TRUE;
				break;
			case AVDTP_MEDIA_CODEC_MPEG2_4_AAC:
				/* Not Yet Implemented */
				/*
				selectOptimalAacCapsSource(a2dp->sep.current_sep->sep_config->caps, service_caps);
				accept = TRUE;
				*/
				accept = FALSE;
				break;
			case AVDTP_MEDIA_CODEC_ATRAC:
				/* Not Yet Implemented */
				/*
				selectOptimalAtracCapsSource(a2dp->sep.current_sep->sep_config->caps, service_caps);
				accept = TRUE;
				*/
				accept = FALSE;
				break;
			case AVDTP_MEDIA_CODEC_NONA2DP:
				if (isCodecCsrFaststream(a2dp->sep.current_sep->sep_config->caps))
				{
					selectOptimalCsrFastStreamCapsSource(a2dp->sep.current_sep->sep_config->caps, service_caps);
					accept = TRUE;
				}
				break;
			default:
				break;
			}
		}
	}
    
    return accept;
}


/*****************************************************************************/
void a2dpHandleConfigureCodecRes(A2DP *a2dp, const A2DP_INTERNAL_CONFIGURE_CODEC_RSP_T *res)
{
	if (a2dp->signal_conn.signalling_state == avdtp_state_processing_caps)
	{
		processCodecInfo(a2dp, res->accept, res->size_codec_service_caps, res->codec_service_caps);
	}
}


/*****************************************************************************/
void a2dpSendCodecAudioParams(A2DP *a2dp)
{
	uint8 codec_type;
	uint32 rate = 0, voice_rate = 0;
    a2dp_channel_mode mode = a2dp_mono;
	uint16 packet_size = 0;
	uint8 bitpool = 0, bitpool_min = 0, bitpool_max = 0, format = 0;
	codec_data_type codecData;
	uint8 *returned_caps = NULL;
	
	uint16 size_service_caps = a2dp->sep.configured_service_caps_size;
    const uint8 *service_caps = a2dp->sep.configured_service_caps;

	a2dp_content_protection content_protection = a2dpGetContentProtection(service_caps, size_service_caps, returned_caps);
    if (!a2dpFindCodecSpecificInformation(&service_caps, &size_service_caps))
		return;

	if (getCodecFromCaps(service_caps, &codec_type))
	{
		switch (codec_type)
		{
		case AVDTP_MEDIA_CODEC_SBC:			
			if (a2dp->sep.current_sep->sep_config->role != a2dp_sink)
			{
				/* 
				As we support the full SBC range, we limit our bit pool range 
				to the values passed by the other side. 
				*/
				bitpool_min = service_caps[6];
				bitpool_max = service_caps[7];
    
				/* Store configuration in SBC format */
				format = a2dpSbcFormatFromConfig(service_caps);
				
				/* Calculate the optimal bitpool to use for the required data rate */
				if ((format & 0x0c) == 0)
					/* Mono mode - 1 channel */
					bitpool = a2dpSbcSelectBitpoolAndPacketSize(format,
										SBC_ONE_CHANNEL_RATE, a2dp->media_conn.mtu, &packet_size);
				else
					/* All other modes are 2 channel */
					bitpool = a2dpSbcSelectBitpoolAndPacketSize(format,
										SBC_TWO_CHANNEL_RATE, a2dp->media_conn.mtu, &packet_size);

				/* Clamp bitpool to remote device's limits. TODO: B-4407 we could try and use a lower data rate. */
				if (bitpool < bitpool_min)
					bitpool = bitpool_min;

				if (bitpool > bitpool_max)
					bitpool = bitpool_max;
			}
			getSbcConfigSettings(service_caps, &rate, &mode);
			break;
		case AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO:
			getMp3ConfigSettings(service_caps, &rate, &mode);
			break;
		case AVDTP_MEDIA_CODEC_MPEG2_4_AAC:
			getAacConfigSettings(service_caps, &rate, &mode);
			break;
#ifdef A2DP_ATRAC			
		case AVDTP_MEDIA_CODEC_ATRAC:
			getAtracConfigSettings(service_caps, &rate, &mode);
			break;
#endif /* A2DP_ATRAC */			
		case AVDTP_MEDIA_CODEC_NONA2DP:
			if (isCodecCsrFaststream(service_caps))
			{
				/* Get the config settings so they can be sent to the client */
				getCsrFastStreamConfigSettings(service_caps, a2dp->sep.current_sep->sep_config->role, &rate, &mode, &voice_rate, &bitpool, &format);
			}
			break;
		default:
			return;
			break;
		}

		/* Tell the client so it can configure the codec */
		codecData.bitpool = bitpool;
		codecData.packet_size = packet_size;
		codecData.format = format;
		codecData.content_protection = content_protection;
		codecData.voice_rate = voice_rate;

		{
			MAKE_A2DP_MESSAGE_WITH_LEN(A2DP_CODEC_SETTINGS_IND, a2dp->sep.configured_service_caps_size);
			message->rate = rate;
			message->channel_mode = mode;
			message->seid = a2dp->sep.current_sep->sep_config->seid;
			message->a2dp = a2dp;
			message->codecData = codecData;
			message->configured_codec_caps_size = a2dp->sep.configured_service_caps_size;
			memmove(message->configured_codec_caps, a2dp->sep.configured_service_caps, a2dp->sep.configured_service_caps_size);
			MessageSend(a2dp->clientTask, (MessageId) A2DP_CODEC_SETTINGS_IND, message);
		}
	}
}

