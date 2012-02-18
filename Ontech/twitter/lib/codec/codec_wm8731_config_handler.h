/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_wm8731_config_handler.h
    
DESCRIPTION
*/

#ifndef CODEC_WM8731_CONFIG_HANDLER_H_
#define CODEC_WM8731_CONFIG_HANDLER_H_


/****************************************************************************
NAME	
	handleWm8731CodecConfigureReq

DESCRIPTION
	Handle internal codec configure request, for the Wolfson codec.
*/
void handleWm8731CodecConfigureReq(WolfsonCodecTaskData *codec, const CODEC_INTERNAL_CONFIG_REQ_T *req);
		

#endif /* CODEC_WM8731_CONFIG_HANDLER_H */

