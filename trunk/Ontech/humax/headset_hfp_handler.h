/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2
*/

/*!
@file    headset_hfp_handler.h
@brief    Functions which handle the HFP library messages.
*/
#ifndef HEADSET_HFP_HANDLER_H
#define HEADSET_HFP_HANDLER_H


#include "headset_private.h"


/****************************************************************************
  FUNCTIONS
*/

/*************************************************************************
NAME    
    hfpHandlerInitCfm
    
DESCRIPTION
    Handles the HFP_INIT_CFM message from the HFP library.
    
*/
void hfpHandlerInitCfm( const HFP_INIT_CFM_T *cfm );
        

/*************************************************************************
NAME    
    hfpHandlerConnectInd
    
DESCRIPTION
    Handles the HFP_SLC_CONNECT_IND message from the HFP library.
    
*/
void hfpHandlerConnectInd( const HFP_SLC_CONNECT_IND_T *cfm );


/*************************************************************************
NAME    
    hfpHandlerConnectCfm
    
DESCRIPTION
    Handles the HFP_SLC_CONNECT_CFM message from the HFP library.
    
*/
void hfpHandlerConnectCfm( const HFP_SLC_CONNECT_CFM_T *cfm );
 

/*************************************************************************
NAME    
    hfpHandlerDisconnectInd
    
DESCRIPTION
    Handles the HFP_SLC_DISCONNECT_IND message from the HFP library.
    
*/
void hfpHandlerDisconnectInd(const HFP_SLC_DISCONNECT_IND_T *ind);


/*************************************************************************
NAME    
    hfpHandlerInbandRingInd
    
DESCRIPTION
    Handles the HFP_IN_BAND_RING_IND message from the HFP library.
    
*/
void hfpHandlerInbandRingInd( const HFP_IN_BAND_RING_IND_T * ind );


/*************************************************************************
NAME    
    hfpHandlerCallInd
    
DESCRIPTION
    Handles the HFP_CALL_IND message from the HFP library.
    
*/
void hfpHandlerCallInd ( const HFP_CALL_IND_T * pInd );


/*************************************************************************
NAME    
    hfpHandlerCallSetupInd
    
DESCRIPTION
    Handles the HFP_CALL_SETUP_IND message from the HFP library.
    
*/
void hfpHandlerCallSetupInd ( const HFP_CALL_SETUP_IND_T * pInd );


/*************************************************************************
NAME    
    hfpHandlerRingInd
    
DESCRIPTION
    Handles the HFP_RING_IND message from the HFP library.
    
*/
void hfpHandlerRingInd ( void );
        

/*************************************************************************
NAME    
    hfpHandlerVoiceRecognitionInd
    
DESCRIPTION
    Handles the HFP_VOICE_RECOGNITION_IND message from the HFP library.
    
*/
void hfpHandlerVoiceRecognitionInd( const HFP_VOICE_RECOGNITION_IND_T *ind );


/*************************************************************************
NAME    
    hfpHandlerVoiceRecognitionCfm
    
DESCRIPTION
    Handles the HFP_VOICE_RECOGNITION_ENABLE_CFM message from the HFP library.
    
*/
void hfpHandlerVoiceRecognitionCfm( const HFP_VOICE_RECOGNITION_ENABLE_CFM_T *cfm );


/*************************************************************************
NAME    
    hfpHandlerLastNoRedialCfm
    
DESCRIPTION
    Handles the HFP_LAST_NUMBER_REDIAL_CFM message from the HFP library.
    
*/
void hfpHandlerLastNoRedialCfm( const HFP_LAST_NUMBER_REDIAL_CFM_T *cfm );


/*************************************************************************
NAME    
    hfpHandlerEncryptionChangeInd
    
DESCRIPTION
    Handles the HFP_ENCRYPTION_CHANGE_IND message from the HFP library.
    
*/
void hfpHandlerEncryptionChangeInd( const HFP_ENCRYPTION_CHANGE_IND_T *ind );


/*************************************************************************
NAME    
    hfpHandlerSpeakerVolumeInd
    
DESCRIPTION
    Handles the HFP_SPEAKER_VOLUME_IND message from the HFP library.
    
*/
void hfpHandlerSpeakerVolumeInd( const HFP_SPEAKER_VOLUME_IND_T *ind );


/*************************************************************************
NAME    
    hfpHandlerAudioConnectInd
    
DESCRIPTION
    Handles the HFP_AUDIO_CONNECT_IND message from the HFP library.
    
*/
void hfpHandlerAudioConnectInd( const HFP_AUDIO_CONNECT_IND_T *ind );


/*************************************************************************
NAME    
    hfpHandlerAudioConnectCfm
    
DESCRIPTION
    Handles the HFP_AUDIO_CONNECT_CFM message from the HFP library.
    
*/
void hfpHandlerAudioConnectCfm( const HFP_AUDIO_CONNECT_CFM_T *cfm );


/*************************************************************************
NAME    
    hfpHandlerAudioDisconnectInd
    
DESCRIPTION
    Handles the HFP_AUDIO_DISCONNECT_IND message from the HFP library.
    
*/
void hfpHandlerAudioDisconnectInd( const HFP_AUDIO_DISCONNECT_IND_T *ind );

/****************************************************************************
NAME    
    hfpHandlerCallWaitingInd
    
DESCRIPTION
    Handle indication that we have a call waiting.
*/
void hfpHandlerCallWaitingInd( const HFP_CALL_WAITING_IND_T* ind );

/****************************************************************************
NAME    
    hfpHandlerCallWaitingEnableCfm
    
DESCRIPTION
    Handles HFP_CALL_WAITING_ENABLE_CFM_T message from HFP library
*/
void hfpHandlerCallWaitingEnableCfm( const HFP_CALL_WAITING_ENABLE_CFM_T* cfm );

/****************************************************************************
NAME    
    hfpHandlerReleaseHeldRejectWaitingCallCfm
DESCRIPTION
    Handles HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM_T message from HFP library
*/
void hfpHandlerReleaseHeldRejectWaitingCallCfm ( const HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM_T *cfm );

/****************************************************************************
NAME    
    hfpHandlerReleaseActiveAcceptOtherCallCfm
DESCRIPTION
    Handles HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM_T message from HFP library

*/
void hfpHandlerReleaseActiveAcceptOtherCallCfm ( const HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM_T *cfm );

/****************************************************************************
NAME    
    hfpHandlerHoldActiveAcceptOtherCallCfm
DESCRIPTION
    Handles HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM_T message from HFP library

*/
void hfpHandlerHoldActiveAcceptOtherCallCfm ( const HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM_T *cfm );

/****************************************************************************
NAME    
    hfpHandlerAddHeldCallCfm
DESCRIPTION
    Handles HFP_ADD_HELD_CALL_CFM_T message from HFP library

*/
void hfpHandlerAddHeldCallCfm ( const HFP_ADD_HELD_CALL_CFM_T *cfm );

/****************************************************************************
NAME    
    hfpHandlerExplicitCallTransferCfm
DESCRIPTION
    Handles HFP_EXPLICIT_CALL_TRANSFER_CFM_T message from HFP library

*/
void hfpHandlerExplicitCallTransferCfm ( const HFP_EXPLICIT_CALL_TRANSFER_CFM_T *cfm );

/****************************************************************************
NAME    
    hfpHandlerThreeWayCallInd

DESCRIPTION
    Handles HFP_CALL_IND_T message from HFP library during threeway calling
    scenario

*/
void hfpHandlerThreeWayCallInd ( const HFP_CALL_IND_T *ind );

/****************************************************************************
NAME    
    hfpHandlerThreeWayCallSetupInd

DESCRIPTION
    Handles HFP_CALL_SETUP_IND_T message from HFP library during threeway calling
    scenario

*/
void hfpHandlerThreeWayCallSetupInd ( const HFP_CALL_SETUP_IND_T *ind );

#endif
