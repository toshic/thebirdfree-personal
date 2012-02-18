/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_wm8731_power_handler.h
    
DESCRIPTION
	
*/

#ifndef CODEC_WM8731_POWER_HANDLER_H_
#define CODEC_WM8731_POWER_HANDLER_H_


/****************************************************************************
NAME	
	handleWm8731CodecEnableReq

DESCRIPTION
	Handle internal codec enable request, for the Wolfson codec.
*/
void handleWm8731CodecEnableReq(WolfsonCodecTaskData *codec);


/****************************************************************************
NAME	
	handleWm8731CodecDisableReq

DESCRIPTION
	Handle internal codec disable request, for the Wolfson codec.
*/
void handleWm8731CodecDisableReq(WolfsonCodecTaskData *codec);


/****************************************************************************
NAME	
	handleWm8731CodecPowerDownReq

DESCRIPTION
	Handle internal codec power down request, for the Wolfson codec.
*/
void handleWm8731CodecPowerDownReq(WolfsonCodecTaskData *codec);


#endif /* CODEC_WM8731_POWER_HANDLER_H */
