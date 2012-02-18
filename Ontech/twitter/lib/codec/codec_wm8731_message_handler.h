/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_wm8731_message_handler.h
    
DESCRIPTION

*/

#ifndef CODEC_WM8731_MESSAGE_HANDLER_H_
#define CODEC_WM8731_MESSAGE_HANDLER_H_


/****************************************************************************
NAME	
	wolfsonMessageHandler

DESCRIPTION
	All messages for the Wolfson codec are handled by this function

RETURNS
	void
*/
void wolfsonMessageHandler(Task task, MessageId id, Message message);


#endif/* CODEC_WM8731_MESSAGE_HANDLER_H */
