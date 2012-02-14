/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1
*/

/*!
@file       audioAdaptor_states.h
@brief      The state machines used in the application.
*/


#ifndef AUDIOADAPTOR_STATES_H
#define AUDIOADAPTOR_STATES_H


typedef enum
{
    AppStateUninitialised, /* Dongle is not initialised for operation */
    AppStateInitialising,  /* Dongle is currently initialising */
    AppStateIdle,          /* Dongle is idle, with or without connections */
    AppStateInquiring,     /* Dongle is looking for devices */
    AppStateSearching,     /* Dongle is performing an SDP search on a specific device */
    AppStateConnecting,    /* Dongle is connecting to a specific device */
    AppStateConnected,
    AppStateStreaming,     /* Dongle is streaming audio via a2dp */
    AppStateInCall,        /* Dongle is setting-up or managing an active call via hsp/hfp */
    AppStateEnteringDfu,   /* Dongle is waiting to enter DFU mode */
    
    AppStateLowBattery,    /* Only change the LED flash pattern */
    AppStatePoweredOff,    /* Only change the LED flash pattern */
    MaxAppStates
} mvdAppState;


typedef enum
{
    AghfpStateUninitialised,
    AghfpStateDisconnected,
    AghfpStatePaging,
    AghfpStatePaged,
    AghfpStateConnected,
    AghfpStateDisconnecting,
    AghfpStateAudioOpening,
    AghfpStateAudioStreaming,
    AghfpStateAudioClosing,
    AghfpStateCallSetup,
    AghfpStateCallActive,
    AghfpStateCallShutdown
} mvdAghfpState;


typedef enum
{
    AvrcpStateUninitialised,
    AvrcpStateDisconnected,
    AvrcpStatePaging,
    AvrcpStatePaged,
    AvrcpStateConnected,
    AvrcpStateDisconnecting
} mvdAvrcpState;


typedef enum
{
    A2dpStateUninitialised,
    A2dpStateDisconnected,
    A2dpStatePaged,
    A2dpStateConnected,
    A2dpStateOpening,
    A2dpStateOpen,
    A2dpStateDisconnecting,
    A2dpStateStarting,
    A2dpStateStreaming,
    A2dpStateSuspending,
    A2dpStateClosing
} mvdA2dpState;


#endif /* AUDIOADAPTOR_STATES_H */

