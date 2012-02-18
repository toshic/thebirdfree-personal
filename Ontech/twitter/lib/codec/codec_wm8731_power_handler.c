/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_wm8731_power_handler.c
    
DESCRIPTION
	
*/

#include "codec.h"
#ifndef CODEC_EXCLUDE_WOLFSON

#include "codec_private.h"
#include "codec_wm8731_power_handler.h"
#include "codec_wm8731.h"


/****************************************************************************
NAME	
	handleWm8731CodecEnableReq

DESCRIPTION
	Handle internal codec enable request, for the Wolfson codec.
*/
void handleWm8731CodecEnableReq(WolfsonCodecTaskData *codec)
{
	wm8731SetActive(codec->init_params, 1);
}


/****************************************************************************
NAME	
	handleWm8731CodecDisableReq

DESCRIPTION
	Handle internal codec disable request, for the Wolfson codec.
*/
void handleWm8731CodecDisableReq(WolfsonCodecTaskData *codec)
{
	wm8731SetActive(codec->init_params, 0);
}


/****************************************************************************
NAME	
	handleWm8731CodecPowerDownReq

DESCRIPTION
	Handle internal codec power down request, for the Wolfson codec.
*/
void handleWm8731CodecPowerDownReq(WolfsonCodecTaskData *codec)
{
	wm8731SetPowerDown(codec->init_params, 0, 0, 0, 0, 0, 1, 1, 1);
}

#endif /* CODEC_EXCLUDE_WOLFSON */
