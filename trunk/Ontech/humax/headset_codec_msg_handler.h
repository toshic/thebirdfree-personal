/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_codec_msg_handler.h
@brief    Handles codec library messages.
*/
#ifndef HEADSET_CODEC_MSG_HANDLER_H
#define HEADSET_CODEC_MSG_HANDLER_H

#include <message.h>


/****************************************************************************
  FUNCTIONS
*/

/*************************************************************************
NAME    
    handleCodecMessage
    
DESCRIPTION
    Handles messages from the codec library.
    
*/
void handleCodecMessage( Task task, MessageId id, Message message );
        
        
#endif
