/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Deals with connecting and disconnecting AGHFP connections, and provides helper functions
    for determining active connections.
*/

#ifndef AUDIOADAPTOR_AGHFP_SLC_H
#define AUDIOADAPTOR_AGHFP_SLC_H


#include "audioAdaptor_private.h"


void aghfpSlcInitHs (void);

void aghfpSlcInitHf (void);

bool aghfpSlcConnect (devInstanceTaskData *inst);

void aghfpSlcDisconnectAll (void);

void aghfpSlcAudioOpen (devInstanceTaskData *inst);

void aghfpSlcAudioClose (void);

devInstanceTaskData *aghfpSlcGetConnectedHF(void);


#endif /* AUDIOADAPTOR_AGHFP_SLC_H */


