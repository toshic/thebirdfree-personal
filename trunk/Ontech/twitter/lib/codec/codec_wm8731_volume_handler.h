/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_wm8731_volume_handler.h
    
DESCRIPTION
	
*/

#ifndef CODEC_WM8731_VOLUME_HANDLER_H_
#define CODEC_WM8731_VOLUME_HANDLER_H_


/****************************************************************************
NAME	
	handleWm8731InputGainReq

DESCRIPTION
	Handle internal input gain request, for the Wolfson codec.
	
	Volume range 11111 = 0x1F (+12dB) in 1.5dB steps down to 00000 (-34.5dB).
    Set mute = 1 to mute input.
*/
void handleWm8731InputGainReq(WolfsonCodecTaskData *codec, const CODEC_INTERNAL_INPUT_GAIN_REQ_T *req);


/****************************************************************************
NAME	
	Wm8731SetInputGain

DESCRIPTION
	Set the codec input gain immediately
*/
void Wm8731SetInputGain(WolfsonCodecTaskData *codec, uint16 volume, codec_channel chn);


/****************************************************************************
NAME	
	handleWm8731OutputGainReq

DESCRIPTION
	Handle internal output gain request, for the Wolfson codec.
	
	Volume range 1111111 = 0x7F (+6dB) in 1dB steps down to 0110000 = 0x30 (-73dB).
	Passed in value for volume will be 0..0x4F.
	Set mute = 1 to mute output.   
*/
void handleWm8731OutputGainReq(WolfsonCodecTaskData *codec, const CODEC_INTERNAL_OUTPUT_GAIN_REQ_T *req);


/****************************************************************************
NAME	
	Wm8731SetOutputGain

DESCRIPTION
	Set the codec output gain immediately
*/
void Wm8731SetOutputGain(WolfsonCodecTaskData *codec, uint16 volume, codec_channel chn);


#endif /* CODEC_WM8731_VOLUME_HANDLER_H */
