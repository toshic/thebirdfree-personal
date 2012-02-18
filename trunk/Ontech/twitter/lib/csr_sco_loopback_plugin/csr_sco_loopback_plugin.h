/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_sco_loopback_plugin.h

DESCRIPTION
    
    
NOTES
   
*/
#ifndef _CSR_SCO_LOOPBACK_PLUGIN_H_
#define _CSR_SCO_LOOPBACK_PLUGIN_H_

#include <message.h> 

#include <message.h>

/*! \name audio plugin

	This is an audio plugin that can be used with the audio library.
*/

typedef struct
{
	TaskData	data;
}ExamplePluginTaskdata;

extern const ExamplePluginTaskdata sco_loopback_plugin ;

#endif

