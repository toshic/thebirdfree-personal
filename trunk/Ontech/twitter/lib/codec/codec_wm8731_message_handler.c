/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_wm8731_message_handler.c
    
DESCRIPTION

*/
#include "codec.h"
#ifndef CODEC_EXCLUDE_WOLFSON

#include "codec_private.h"
#include "codec_wm8731_init_handler.h"
#include "codec_wm8731_config_handler.h"
#include "codec_wm8731_power_handler.h"
#include "codec_wm8731_volume_handler.h"
#include "codec_wm8731_message_handler.h"


/****************************************************************************
NAME	
	wolfsonMessageHandler

DESCRIPTION
	All messages for the Wolfson codec are handled by this function

RETURNS
	void
*/
void wolfsonMessageHandler(Task task, MessageId id, Message message)
{
	WolfsonCodecTaskData *codec = (WolfsonCodecTaskData *) task;
	codec = codec;

	/* Check the message id */
	switch (id)
	{
	case CODEC_INTERNAL_INIT_REQ:
		handleWm8731CodecInitReq(codec);
		break;
	case CODEC_INTERNAL_CONFIG_REQ:
		handleWm8731CodecConfigureReq(codec, (CODEC_INTERNAL_CONFIG_REQ_T *) message);		
		break;
	case CODEC_INTERNAL_INPUT_GAIN_REQ:
		handleWm8731InputGainReq(codec, (CODEC_INTERNAL_INPUT_GAIN_REQ_T *) message);
		break;
	case CODEC_INTERNAL_OUTPUT_GAIN_REQ:
		handleWm8731OutputGainReq(codec, (CODEC_INTERNAL_OUTPUT_GAIN_REQ_T *) message);
		break;
	case CODEC_INTERNAL_CODEC_ENABLE_REQ:
		handleWm8731CodecEnableReq(codec);
		break;
	case CODEC_INTERNAL_CODEC_DISABLE_REQ:
		handleWm8731CodecDisableReq(codec);
		break;
	case CODEC_INTERNAL_POWER_DOWN_REQ:
		handleWm8731CodecPowerDownReq(codec);
		break;
	default:
		break;
	}
}


#endif /* CODEC_EXCLUDE_WOLFSON */
