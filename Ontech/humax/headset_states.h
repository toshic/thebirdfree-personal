/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_states.h
@brief    The Headset States.
*/

#ifndef _HEADSET_STATES_H
#define _HEADSET_STATES_H


/* HFP states */
typedef enum
{                                   /* stored as 12 bits in system events config 'event_config_type' */
    headsetPoweringOn,              /* 001 */
    headsetConnDiscoverable,        /* 002 */
    headsetHfpConnectable,          /* 004 */
    headsetHfpConnected,            /* 008 */
    headsetOutgoingCallEstablish,   /* 010 */
    headsetIncomingCallEstablish,   /* 020 */
    headsetActiveCall,              /* 040 */
    headsetTestMode,                /* 080 */
    headsetTWCWaiting,              /* 100 */
    headsetTWCOnHold,               /* 200 */
    headsetTWCMulticall             /* 400 */
} headsetHfpState;

#define HEADSET_NUM_HFP_STATES (headsetTWCMulticall + 1) 


/* A2DP states */
typedef enum
{
    headsetA2dpConnectable,
    headsetA2dpConnected,
    headsetA2dpStreaming,
    headsetA2dpPaused
} headsetA2dpState;

#define HEADSET_NUM_A2DP_STATES (headsetA2dpPaused + 1) 

/* AVRCP states */
typedef enum
{
    avrcpInitialising,
    avrcpReady,
    avrcpConnecting,
    avrcpConnected
} headsetAvrcpState;


#endif

