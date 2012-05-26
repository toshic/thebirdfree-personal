/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_absolute_volume.h
    
DESCRIPTION
    
*/

#ifndef  AVRCP_ABSOLUTE_VOLUME_H_
#define  AVRCP_ABSOLUTE_VOLUME_H_

#include "avrcp_common.h"

/* Preprocessor definitions */
#define AVRCP_MAX_VOL_MASK      0x7F

/* Internal function declarations */
void avrcpSendAbsoluteVolumeCfm(AVRCP               *avrcp,
                                avrcp_status_code   status,
                                const uint8         *data);

void avrcpHandleInternalAbsoluteVolumeRsp(AVRCP                   *avrcp,
                    const AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES_T   *res);

void avrcpHandleInternalAbsoluteVolumeEvent(AVRCP                *avrcp,
                    const AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES_T  *res);

void avrcpHandleSetAbsoluteVolumeCommand(AVRCP *avrcp, uint8 volume);



#endif /*  AVRCP_ABSOLUTE_VOLUME_H_*/

