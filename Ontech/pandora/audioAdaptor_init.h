/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Initialisation code.
*/

#ifndef AUDIOADAPTOR_INIT_H
#define AUDIOADAPTOR_INIT_H


#include "audioAdaptor_private.h"
#include "csr_sbc_encoder_plugin.h"
#ifdef INCLUDE_MP3_ENCODER_PLUGIN
#include "csr_mp3_encoder_plugin.h"
#endif
#include "csr_faststream_source_plugin.h"
#include "csr_cvsd_usb_no_dsp_plugin.h"


Task initA2dpPlugin(uint8 seid);

Task initScoPlugin(void);

uint16 initSeidConnectPriority(uint8 *seid_list);

void initUserFeatures(void);

void initCodec(void);

void initAvrcp(void);

void initA2dp(void);

void initApp(void);

void initProfile(void);

void initProfileCfm(mvdProfiles profile, bool success);
        

#endif /* AUDIOADAPTOR_INIT_H */
