/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_volume.c
    
DESCRIPTION
	
*/

#include "codec.h"
#include "codec_private.h"
#include "codec_wm8731_message_handler.h"
#include "codec_csr_internal_message_handler.h"
#include "codec_wm8731_volume_handler.h"

#include <codec.h>


void CodecSetInputGain(Task codecTask, uint16 volume, codec_channel channel)
{
	MAKE_CODEC_MESSAGE(CODEC_INTERNAL_INPUT_GAIN_REQ);
	message->volume = volume;
    message->channel = channel;
	MessageSend(codecTask, CODEC_INTERNAL_INPUT_GAIN_REQ, message);
}


void CodecSetInputGainNow(Task codecTask, uint16 volume, codec_channel channel)
{
    if (codecTask->handler == csrInternalMessageHandler)
    {
        if (channel != right_ch)
    	    CodecSetInputGainA(volume);
        if (channel != left_ch)
    	    CodecSetInputGainB(volume);
    }
#ifndef CODEC_EXCLUDE_WOLFSON
    else if (codecTask->handler == wolfsonMessageHandler)
    {
        Wm8731SetInputGain((WolfsonCodecTaskData *) codecTask, volume, channel);
    }
#endif /* CODEC_EXCLUDE_WOLFSON */
    else
    {
        CodecSetInputGain(codecTask, volume, channel);
    }
}


void CodecSetOutputGain(Task codecTask, uint16 volume, codec_channel channel)
{
	MAKE_CODEC_MESSAGE(CODEC_INTERNAL_OUTPUT_GAIN_REQ);
    message->volume = volume;
	message->channel = channel;
	MessageSend(codecTask, CODEC_INTERNAL_OUTPUT_GAIN_REQ, message);
}


void CodecSetOutputGainNow(Task codecTask, uint16 volume, codec_channel channel)
{
    if (codecTask->handler == csrInternalMessageHandler)
    {
        if (channel != right_ch)
    	    CodecSetOutputGainA(volume);
        if (channel != left_ch)
    	    CodecSetOutputGainB(volume);	
    }
#ifndef CODEC_EXCLUDE_WOLFSON
    else if (codecTask->handler == wolfsonMessageHandler)
    {
        Wm8731SetOutputGain((WolfsonCodecTaskData *) codecTask, volume, channel);
    }
#endif /* CODEC_EXCLUDE_WOLFSON */
    else
    {
        CodecSetOutputGain(codecTask, volume, channel);
    }
}
