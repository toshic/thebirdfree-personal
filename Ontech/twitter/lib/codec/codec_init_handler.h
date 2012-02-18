/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_init_handler.h
    
DESCRIPTION
*/

#ifndef CODEC_INIT_HANDLER_H_
#define CODEC_INIT_HANDLER_H_


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
					  uint16 outputGainRange);
	

#endif /* CODEC_INIT_HANDLER_H */
