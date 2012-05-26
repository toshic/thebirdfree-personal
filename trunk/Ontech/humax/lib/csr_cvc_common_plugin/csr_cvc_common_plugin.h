/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_cvc_common_plugin.h

DESCRIPTION
    
    
NOTES
   
*/
#ifndef _CSR_CVC_COMMON_PLUGIN_H_
#define _CSR_CVC_COMMON_PLUGIN_H_

#include <message.h> 

/*! \name CSR_CVC_COMMON plugin

	This is an cVc plugin that can be used with the cVc DSP library.
*/

typedef struct
{
	TaskData	data;
	unsigned	cvc_plugin_variant:4 ;	/* Selects the CVC plugin variant */
    unsigned	sco_encoder:3 ;			/* Sets if its CVSd, Auri or SBC */
    unsigned	two_mic:1;				/* Set the bit if using 2mic plugin */
    unsigned	sco_config:3;			/* Value to send in Kalimba MESSAGE_SCO_CONFIG message */
    unsigned	reserved:5 ;			/* Set the reserved bits to zero */
}CvcPluginTaskdata;

extern const CvcPluginTaskdata csr_cvsd_cvc_1mic_handsfree_plugin ;

extern const CvcPluginTaskdata csr_cvsd_cvc_1mic_headset_plugin ;

extern const CvcPluginTaskdata csr_cvsd_cvc_2mic_headset_plugin ;

extern const CvcPluginTaskdata csr_auristream_2bit_cvc_1mic_handsfree_plugin ;

extern const CvcPluginTaskdata csr_auristream_2bit_cvc_1mic_headset_plugin ;

extern const CvcPluginTaskdata csr_auristream_2bit_cvc_2mic_headset_plugin ;

extern const CvcPluginTaskdata csr_auristream_4bit_cvc_1mic_handsfree_plugin ;

extern const CvcPluginTaskdata csr_auristream_4bit_cvc_1mic_headset_plugin ;

extern const CvcPluginTaskdata csr_auristream_4bit_cvc_2mic_headset_plugin ;

extern const CvcPluginTaskdata csr_sbc_cvc_1mic_headset_plugin ;

extern const CvcPluginTaskdata csr_cvsd_cvc_1mic_lite_headset_plugin ;

extern const CvcPluginTaskdata csr_cvsd_purespeech_1mic_headset_plugin ;

extern const CvcPluginTaskdata csr_sbc_cvc_1mic_handsfree_plugin ;

#define AUDIO_MODE_CVC_MSG_BASE     (0x1000)

/* Additional modes not explicitly defined by the audio manager */
enum 
{
   AUDIO_MODE_CVC_BASE = AUDIO_MODE_CVC_MSG_BASE,
   AUDIO_MODE_SPEECH_REC
};

#endif

