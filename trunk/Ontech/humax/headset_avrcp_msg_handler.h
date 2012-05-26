/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_avrcp_msg_handler.h
@brief    Interface to AVRCP library message handler.
*/
#ifndef HEADSET_AVRCP_MSG_HANDLER_H
#define HEADSET_AVRCP_MSG_HANDLER_H

#include <message.h>
#include <avrcp.h>

#include "headset_private.h"


/****************************************************************************
  FUNCTIONS
*/

/*************************************************************************
NAME    
    handleAVRCPMessage
    
DESCRIPTION
    Handles messages from the AVRCP library.
    
*/
void handleAVRCPMessage( Task task, MessageId id, Message message );
        

#endif
