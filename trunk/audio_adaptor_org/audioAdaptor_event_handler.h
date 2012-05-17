/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handle application events.
*/

#ifndef AUDIOADAPTOR_EVENT_HANDLER_H
#define AUDIOADAPTOR_EVENT_HANDLER_H


void eventHandleCancelCommAction (void);

void eventHandleAppMessage (MessageId id, Message message);

void eventHandleInstanceMessage(devInstanceTaskData *inst, MessageId id, Message message);

void eventHandleSendKickCommActionMessage (mvdCommAction comm_action);


#endif /* AUDIOADAPTOR_EVENT_HANDLER_H */

