/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_volume.h
@brief  Interface to volume controls.
*/

#ifndef HEADSET_VOLUME_H
#define HEADSET_VOLUME_H


#include "headset_private.h"


#define VOL_MAX_VOLUME_LEVEL (0x0f)


/****************************************************************************
NAME 
    VolumeInit

DESCRIPTION
    Initialises the volume.

*/
void VolumeInit ( int16 pConfigID );


/****************************************************************************
NAME 
    VolumeToggleMute

DESCRIPTION
    Toggles the mute state.

*/
void VolumeToggleMute ( void );


/****************************************************************************
NAME 
    VolumeMuteOn

DESCRIPTION
    Enables Mute.

*/
void VolumeMuteOn ( void ) ;


/****************************************************************************
NAME 
    VolumeMuteOff

DESCRIPTION
    Disables Mute.

*/
void VolumeMuteOff ( void ) ;


/****************************************************************************
NAME 
    VolumeUp

DESCRIPTION
    Increments volume.
    
*/
void VolumeUp ( void ) ;


/****************************************************************************
NAME 
    VolumeDown

DESCRIPTION
    Decrements volume.

*/
void VolumeDown ( void ) ;


/****************************************************************************
NAME 
    VolumeSendSettingsToAG

DESCRIPTION
    Send local speaker and mic volume levels to the connected AG .

*/
void VolumeSendSettingsToAG(bool send_speaker, bool send_mic);


/****************************************************************************
NAME 
    VolumeGetHeadsetVolume

DESCRIPTION
    Retrieve the current headset volume. 

RETURNS
	Returns FALSE if no volume was retrieved.

*/
bool VolumeGetHeadsetVolume(uint16 * actVol, bool * avAudio);


/****************************************************************************
NAME 
    VolumeSetHeadsetVolume

DESCRIPTION
    Sets the current headset volume. This volume can also be sent to the connected AG. 

RETURNS
	Returns FALSE if no volume was retrieved.

*/
void VolumeSetHeadsetVolume(uint16 actVol, bool avAudio, bool sendVolToAg);


#endif

