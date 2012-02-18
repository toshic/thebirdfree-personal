/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_init.c
    
DESCRIPTION
	
*/

#include "codec.h"
#include "codec_private.h"
#include "codec_wm8731_message_handler.h"
#include "codec_csr_internal_message_handler.h"

#include <stdlib.h>

#ifdef CODEC_EXCLUDE_WOLFSON
#include <panic.h>
#endif /* CODEC_EXCLUDE_WOLFSON */

/****************************************************************************
NAME	
	CodecInitWolfson

DESCRIPTION
   Initialise the Wolfson Codec. 
   
   CODEC_INIT_CFM message will be received by the application.	
*/
void CodecInitWolfson(Task appTask, wolfson_init_params *init)
{
#ifdef CODEC_EXCLUDE_WOLFSON
	/****************************************************************************
		Library variant chosen (codec_nowolfson) does not contain support for
		the Wolfson wm8731 codec.  Use the library variant 'codec' if Wolfson
		support is required
	****************************************************************************/
	Panic();
#else
	WolfsonCodecTaskData *codec = (WolfsonCodecTaskData *)PanicUnlessMalloc(sizeof(WolfsonCodecTaskData));	
    codec->init_params = (wolfson_init_params *)PanicUnlessMalloc(sizeof(wolfson_init_params));	
	codec->task.handler = wolfsonMessageHandler;
	codec->clientTask = appTask;
    if (init)
    {
        codec->init_params->csb = init->csb;
        codec->init_params->sdin = init->sdin;
        codec->init_params->sclk = init->sclk;
    }
    else
    {
        /* 
           CSB - PIO9
           SDIN - PIO10
           SCLK - PIO11
        */
        codec->init_params->csb = 0x0200;
        codec->init_params->sdin = 0x0400;
        codec->init_params->sclk = 0x0800;
    }
	MessageSend(&codec->task, CODEC_INTERNAL_INIT_REQ, 0);
#endif /* CODEC_EXCLUDE_WOLFSON */
}


/****************************************************************************
NAME	
	CodecInitCsrInternal

DESCRIPTION
   Initialise the CSR internal Codec. 
   
   CODEC_INIT_CFM message will be received by the application.	
*/
void CodecInitCsrInternal(Task appTask)
{
	CsrInternalCodecTaskData *codec = (CsrInternalCodecTaskData *)PanicUnlessMalloc(sizeof(WolfsonCodecTaskData));	
	codec->task.handler = csrInternalMessageHandler;
	codec->clientTask = appTask;
	MessageSend(&codec->task, CODEC_INTERNAL_INIT_REQ, 0);
}


