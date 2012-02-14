/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1
*/

/*!
@file    audioAdaptor_statemanager.h
@brief    main audio adaptor state information
*/
#ifndef _AUDIOADAPTOR_STATE_MANAGER_H
#define _AUDIOADAPTOR_STATE_MANAGER_H


#include "audioAdaptor_private.h"
#include "audioAdaptor_states.h"


void setA2dpState(devInstanceTaskData *theInst, mvdA2dpState state);

void setAvrcpState(devInstanceTaskData *theInst, mvdAvrcpState state);

void setAghfpState(devInstanceTaskData *theInst, mvdAghfpState state);

void setAppState(mvdAppState state);

mvdAghfpState getAghfpState(devInstanceTaskData *theInst);

mvdA2dpState getA2dpState(devInstanceTaskData *theInst);

mvdAvrcpState getAvrcpState(devInstanceTaskData *theInst);


#endif /* _AUDIOADAPTOR_STATE_MANAGER_H */

