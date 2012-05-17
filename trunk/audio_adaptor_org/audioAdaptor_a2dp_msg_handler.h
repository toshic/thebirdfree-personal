/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles messages received from the A2DP library.
*/

/*!
@file    audioAdaptor_a2dp_msg_handler.h
@brief    HandleaudioAdaptor a2dp library messages.
*/
#ifndef AUDIOADAPTOR_A2DP_MSG_HANDLER_H
#define AUDIOADAPTOR_A2DP_MSG_HANDLER_H


#include <message.h>

#include "audioAdaptor_private.h"


/****************************************************************************
    FUNCTIONS
*/
void a2dpMsgChooseSbcAudioQuality(devInstanceTaskData *theInst);

void a2dpMsgHandleLibMessage(MessageId id, Message message);

void a2dpMsgHandleInstanceMessage(devInstanceTaskData *theInst, MessageId id, Message message);


#endif /* AUDIOADAPTOR_A2DP_MSG_HANDLER_H */
