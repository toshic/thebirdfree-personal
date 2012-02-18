/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
	Handles messages received from the AVRCP library.
*/

#ifndef AUDIOADAPTOR_AVRCP_MSG_HANDLER_H
#define AUDIOADAPTOR_AVRCP_MSG_HANDLER_H


#include <message.h>


void avcrpMsgHandleLibMessage(MessageId id, Message message);

void avrcpMsgHandleInstanceMessage(devInstanceTaskData *theInst, MessageId id, Message message);


#endif /* AUDIOADAPTOR_AVRCP_MSG_HANDLER_H */
