/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_config_handler.h
    
DESCRIPTION
*/

#ifndef CODEC_CONFIG_HANDLER_H_
#define CODEC_CONFIG_HANDLER_H_


/****************************************************************************
NAME	
	sendCodecConfigureCfm

DESCRIPTION
	Send a CODEC_CONFIG_CFM message back to the client application with
	a status code.
*/
void sendCodecConfigureCfm(Task clientTask, codec_status_code status);
		

#endif /* CODEC_CONFIG_HANDLER_H */
