/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Maintains a device instance for each remote device the audio adaptor is communicating with.
*/

#ifndef AUDIOADAPTOR_DEV_INSTANCE_H
#define AUDIOADAPTOR_DEV_INSTANCE_H


devInstanceTaskData *devInstanceFindFromBddr(const bdaddr *bd_addr, bool create_instance);

devInstanceTaskData *devInstanceFindFromAG(AGHFP *aghfp);

devInstanceTaskData *devInstanceCreate(const bdaddr *bd_addr);

void devInstanceDestroy(devInstanceTaskData *theInst);

void devInstanceHandleMessage(Task task, MessageId id, Message message);

        
#endif /* AUDIOADAPTOR_DEV_INSTANCE_H */
