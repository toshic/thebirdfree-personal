/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    General profile connection functionality.
*/

#ifndef AUDIOADAPTOR_PROFILE_SLC_H
#define AUDIOADAPTOR_PROFILE_SLC_H


void profileSlcConnectReq (void);

void profileSlcConnectComplete (devInstanceTaskData *inst, bool status);

void profileSlcStartConnectionProcess (void);

void profileSlcCancelConnectionAttempt(devInstanceTaskData *inst);

void profileSlcConnectCfm (devInstanceTaskData *inst, mvdProfiles profile, bool remote_connecting);

void profileSlcAcceptConnectInd (devInstanceTaskData *inst);

void profileSlcDisconnectInd (devInstanceTaskData *inst, mvdProfiles profile);

bool profileSlcAreAnyProfilesConnecting(void);

bool profileSlcAreAllProfilesDisconnected(devInstanceTaskData *inst);

bool profileSlcAreAllDevicesDisconnected(void);

bool profileSlcAreAnyRemoteDevicesConnecting(void);

bool profileSlcAreAllDevicesNotConnected(void);

bool profileSlcCheckPoweredOff(devInstanceTaskData *inst);


#endif /* AUDIOADAPTOR_PROFILE_SLC_H */
