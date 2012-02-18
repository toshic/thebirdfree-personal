/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_init_handler.c
    
DESCRIPTION
	
*/

#include "codec.h"
#include "codec_private.h"
#include "codec_init_handler.h"


/****************************************************************************
NAME	
	sendInitCfmToApp

DESCRIPTION
	Send an initialisation confirmation message back to the client application.
*/
void sendInitCfmToApp(Task codecTask, 
					  Task clientTask, 
					  codec_status_code status, 
					  codec_type type_of_codec, 
					  uint16 inputGainRange, 
					  uint16 outputGainRange)
{
	MAKE_CODEC_MESSAGE(CODEC_INIT_CFM);
	message->status = status;
	message->codecTask = codecTask;
	message->type_of_codec = type_of_codec;
	message->inputGainRange = inputGainRange;
	message->outputGainRange = outputGainRange;
	MessageSend(clientTask, CODEC_INIT_CFM, message);
}
