/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_cvsd_usb_no_dsp_plugin.h

DESCRIPTION
    
    
NOTES
   
*/

/*!
@file   csr_cvsd_usb_no_dsp_plugin.h
@brief  Header file for the csr plugin (usb input) which connects a synchronous audio connection to
        a codec task without the need of a DSP
        
	This can connect the synchronous audio connection using the internal codecs on 
    either DAC channel A only or DAC channel A & B
    
    
    AudioplayTone() is supported in this plugin. In the case of playing a tone, the audio
    connection to the local speaker is disconencted whilst the tone connection is made.
    The Synchronous audio connection will be re-connected on successful playback of the tone
    
    Tone playback opccurs in the order that tone play requests are made.
    
    AudioStopTone() is supported in this plugin - i.e. tones can be stopped prematurely 
    (e.g ring tones)
    The Synchronous audio connection will be re-connected on successful termination of the tone
     

*/
#ifndef _CSR_CVSD_USB_NO_DSP_PLUGIN_H_
#define _CSR_CVSD_USB_NO_DSP_PLUGIN_H_

#include <message.h>

/*! \name audio plugin

	This is an audio plugin that can be used with the audio library
	The plugin provides sco handling with no dsp routing
*/
extern const TaskData csr_cvsd_usb_no_dsp_plugin ;

#endif

