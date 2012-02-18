/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_csr_internal_init_handler.c
    
DESCRIPTION
	
*/

#include "codec.h"
#include "codec_private.h"
#include "codec_init_handler.h"
#include "codec_csr_internal_init_handler.h"


/****************************************************************************
NAME	
	handleCsrInternalCodecInitReq

DESCRIPTION
	Function to handle internal init request message, for the CSR internal
	codec.
*/
void handleCsrInternalCodecInitReq(CsrInternalCodecTaskData *codec)
{
	sendInitCfmToApp(&codec->task, codec->clientTask, codec_success, codec_csr_internal, CodecInputGainRange(), CodecOutputGainRange());
}
