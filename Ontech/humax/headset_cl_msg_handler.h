/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_cl_msg_handler.h
@brief    Handles connection library messages.
*/
#ifndef HEADSET_CL_MSG_HANDLER_H
#define HEADSET_CL_MSG_HANDLER_H

#include <message.h>


/****************************************************************************
  FUNCTIONS
*/

/*************************************************************************
NAME    
    handleCLMessage
    
DESCRIPTION
    Handles messages from the Connection library (CL).
    
*/
void handleCLMessage( Task task, MessageId id, Message message );
        
        
#endif
