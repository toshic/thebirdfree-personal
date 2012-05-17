/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Support code for reading and writing the Persistent Store, and buffering of device data.
*/


#ifndef AUDIOADAPTOR_CONFIGURE_H
#define AUDIOADAPTOR_CONFIGURE_H


#include "audioAdaptor_private.h"

#include <aghfp.h>
#include <bdaddr.h>

#define MIN_DEVICE_ORDERING 1

/* Persistent store key allocation */
#define PSKEY_BASE  (0)

enum
{
    PSKEY_LOCAL_PROFILES        = PSKEY_BASE,
    PSKEY_PIN_CODE_LIST         = 1,
    PSKEY_UNUSED                = 2,
    PSKEY_HID_CONFIG            = 3,
    PSKEY_HID_MAPPING           = 4,
    PSKEY_REMOTE_DEVICE_LIST    = 7,
    PSKEY_HID_SEQUENCE          = 8,
    PSKEY_STREAMING_TIMEOUT     = 9,
    PSKEY_CODEC_ENABLED         = 11,
    PSKEY_SOURCE_TYPE           = 12,
    PSKEY_BATTERY_CONFIG        = 13,
    PSKEY_MP3_CODEC_CONFIGURATION          = 14,
    PSKEY_SBC_CODEC_CONFIGURATION          = 15,
    PSKEY_FASTSTREAM_CODEC_CONFIGURATION   = 16
};


uint16 configureGetAudioStreamingTimeout (void);

void configureGetConfigCache (void);

bool configureHidConfig (void);

mvdHidCommand configureGetHidCommand (mvdAppEvent event, uint16 offset);

bool configureGetHidSequenceKeycode(uint16 idx, uint16 *keycode);

bool configureGetSupportedProfiles (void);

bool configureGetSupportedSourceType (void);

bool configureGetPowerManagerConfig(void);

bool configureResetPairedDeviceListBuffer(void);

char *configureGetPinCode(devInstanceTaskData *inst);

devInstanceTaskData *configureGetMostRecentDevice (void);

bool configureGetBdaddrMostRecentDevice (bdaddr *bd_addr);

bool configureGetBdaddrNextRecentDevice (bdaddr *bd_addr);

bool configureGetBdaddrNextDisconnectedDevice(bdaddr *bd_addr);

bool configureGetDeviceInfo (const bdaddr *bd_addr, uint16 *pin_idx, mvdProfiles *remote_profiles, bool *pin_authorised);

void configureClearRecentDeviceList (void);

bool configureStoreCurrentPairedDevice (devInstanceTaskData *inst);

bool configureIsPinRequested (devInstanceTaskData *inst);

bool configureSetPinAuthorised (bdaddr *bd_addr, bool authorised);

bool configureFindMostRecentDevice (uint16 *device_idx);

bool configureIsPinListExhausted(devInstanceTaskData *inst);


#endif /* AUDIOADAPTOR_CONFIGURE_H */


