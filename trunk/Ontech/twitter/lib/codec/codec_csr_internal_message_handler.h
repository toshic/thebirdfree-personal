/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_csr_internal_message_handler.h
    
DESCRIPTION

*/

#ifndef CODEC_CSR_INTERNAL_MESSAGE_HANDLER_H_
#define CODEC_CSR_INTERNAL_MESSAGE_HANDLER_H_


/****************************************************************************
NAME	
	csrInternalMessageHandler

DESCRIPTION
	All messages for the CSR internal codec are handled by this function

RETURNS
	void
*/
void csrInternalMessageHandler(Task task, MessageId id, Message message);


#endif /* CODEC_CSR_INTERNAL_MESSAGE_HANDLER_H */
