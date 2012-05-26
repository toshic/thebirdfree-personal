/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_common_no_dsp_plugin.h

DESCRIPTION


NOTES

*/
#ifndef _CSR_COMMON_NO_DSP_PLUGIN_H_
#define _CSR_COMMON_NO_DSP_PLUGIN_H_

#include <message.h>

/*! \name audio plugin

    This is an audio plugin that can be used with the audio library.
*/

typedef struct
{
    TaskData    data;
    unsigned    no_dsp_plugin_variant:4 ;   /* Selects the no_dsp plugin variant */
    unsigned    sco_encoder:3 ;         /* Sets if its CVSD, Auri or SBC */
    unsigned    two_mic:1;              /* Set the bit if using 2mic plugin */
    unsigned    sco_config:3;           /* Value to send in Kalimba MESSAGE_SCO_CONFIG message */
    unsigned    reserved:5 ;            /* Set the reserved bits to zero */
}NoDspPluginTaskdata;

extern const NoDspPluginTaskdata csr_cvsd_no_dsp_plugin;

#ifdef INCLUDE_NO_DSP_AURISTREAM            

extern const NoDspPluginTaskdata csr_auristream_2bit_no_dsp_plugin;

extern const NoDspPluginTaskdata csr_auristream_4bit_no_dsp_plugin;

#endif /* INCLUDE_NO_DSP_AURISTREAM */

#endif

