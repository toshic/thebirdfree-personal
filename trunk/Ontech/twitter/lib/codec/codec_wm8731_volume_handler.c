/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_wm8731_volume_handler.c
    
DESCRIPTION
	
*/

#include "codec.h"
#ifndef CODEC_EXCLUDE_WOLFSON

#include "codec_private.h"
#include "codec_wm8731.h"
#include "codec_wm8731_volume_handler.h"


/* Set Line Input gain for either left, right or both channels */
void Wm8731SetInputGain(WolfsonCodecTaskData *codec, uint16 volume, codec_channel chn)
{
	uint16 mute = 0;
	uint16 both_chns = 0;
	
	if (volume > 0x1F)
		volume = 0x1F;
	
	if (!volume)
		mute = 1;
	
	if (chn == right_ch)
		wm8731SetRightLineIn(codec->init_params, volume, mute, 0);
	else
	{
		if (chn == left_and_right_ch)
			both_chns = 1;
		wm8731SetLeftLineIn(codec->init_params, volume, mute, both_chns);
	}
}


/* Set Headphone Output gain for either left, right or both channels */
void Wm8731SetOutputGain(WolfsonCodecTaskData *codec, uint16 volume, codec_channel chn)
{
	uint16 both_chns = 0;
	
    uint16 tmp = (uint16)(79+(volume*3));
    if (tmp > 127)
        tmp = 127;
	
	if (chn == right_ch)
		wm8731SetRightHeadphoneOut(codec->init_params, tmp, 0, 0);
	else
	{
		if (chn == left_and_right_ch)
			both_chns = 1;
		wm8731SetLeftHeadphoneOut(codec->init_params, tmp, 0, both_chns);
	}
	wm8731SetActive(codec->init_params, 1);
}


/****************************************************************************
NAME	
	handleWm8731InputGainReq

DESCRIPTION
	Handle internal input gain request, for the Wolfson codec.
	
	Volume range 11111 = 0x1F (+12dB) in 1.5dB steps down to 00000 (-34.5dB).
    Set mute = 1 to mute input.
*/
void handleWm8731InputGainReq(WolfsonCodecTaskData *codec, const CODEC_INTERNAL_INPUT_GAIN_REQ_T *req)
{
	Wm8731SetInputGain(codec, req->volume, req->channel);
}


/****************************************************************************
NAME	
	handleWm8731OutputGainReq

DESCRIPTION
	Handle internal output gain request, for the Wolfson codec.
	
	Volume range 1111111 = 0x7F (+6dB) in 1dB steps down to 0110000 = 0x30 (-73dB).
	Passed in value for volume will be 0..0x4F.
	Set mute = 1 to mute output.   
*/
void handleWm8731OutputGainReq(WolfsonCodecTaskData *codec, const CODEC_INTERNAL_OUTPUT_GAIN_REQ_T *req)
{
	Wm8731SetOutputGain(codec, req->volume, req->channel);
}

#endif /* CODEC_EXCLUDE_WOLFSON */
