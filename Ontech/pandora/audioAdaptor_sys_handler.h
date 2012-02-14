/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles system messages received from Bluestack.
*/

#ifndef AUDIOADAPTOR_SYS_HANDLER_H
#define AUDIOADAPTOR_SYS_HANDLER_H

#include <connection.h>
#include <message.h>
#include <stdio.h>


void sysHandleSystemMessage(MessageId id, Message message);

bool sysHandleButtonsMessage(MessageId id, Message message);


#endif
