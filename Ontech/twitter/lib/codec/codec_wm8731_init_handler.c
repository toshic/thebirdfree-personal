/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_wm8731_init_handler.c
    
DESCRIPTION
	
*/

#include "codec.h"
#ifndef CODEC_EXCLUDE_WOLFSON

#include "codec_private.h"
#include "codec_init_handler.h"
#include "codec_wm8731_init_handler.h"
#include "codec_wm8731.h"


/****************************************************************************
NAME	
	handleWm8731CodecInitReq

DESCRIPTION
	Handle internal codec init request, for the Wolfson codec.
*/
void handleWm8731CodecInitReq(WolfsonCodecTaskData *codec)
{
	wm8731Init(codec->init_params);
	sendInitCfmToApp(&codec->task, codec->clientTask, codec_success, codec_wm8731, 0xf, 0x4f);
}

#endif /* CODEC_EXCLUDE_WOLFSON */
