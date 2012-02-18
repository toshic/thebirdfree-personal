/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_csr_internal_volume_handler.c
    
DESCRIPTION
	
*/

#include "codec.h"
#include "codec_private.h"
#include "codec_csr_internal_volume_handler.h"


/****************************************************************************
NAME	
	handleCsrInternalInputGainReq

DESCRIPTION
	Function to handle internal input gain request message, for the CSR internal
	codec.
*/
void handleCsrInternalInputGainReq(const CODEC_INTERNAL_INPUT_GAIN_REQ_T *req)
{
    if (req->channel != right_ch)
    	CodecSetInputGainA(req->volume);
    if (req->channel != left_ch)
    	CodecSetInputGainB(req->volume);
}


/****************************************************************************
NAME	
	handleCsrInternalOutputGainReq

DESCRIPTION
	Function to handle internal output gain request message, for the CSR internal
	codec.
*/
void handleCsrInternalOutputGainReq(const CODEC_INTERNAL_OUTPUT_GAIN_REQ_T *req)
{
    if (req->channel != right_ch)
    	CodecSetOutputGainA(req->volume);
    if (req->channel != left_ch)
    	CodecSetOutputGainB(req->volume);	
}


