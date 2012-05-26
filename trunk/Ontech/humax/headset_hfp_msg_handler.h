/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_hfp_msg_handler.h
@brief    Handles hfp library messages.
*/
#ifndef HEADSET_HFP_MSG_HANDLER_H
#define HEADSET_HFP_MSG_HANDLER_H

#include <message.h>


/****************************************************************************
  FUNCTIONS
*/

/*************************************************************************
NAME    
    handleHFPMessage
    
DESCRIPTION
    Handles messages from the HFP library.
    
*/
void handleHFPMessage( Task task, MessageId id, Message message );
        
        
#endif
