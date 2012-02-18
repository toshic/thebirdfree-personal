/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    codec_csr_internal_config_handler.h
    
DESCRIPTION
*/

#ifndef CODEC_CSR_INTERNAL_CONFIG_HANDLER_H_
#define CODEC_CSR_INTERNAL_CONFIG_HANDLER_H_


/****************************************************************************
NAME	
	handleCsrInternalCodecConfigureReq

DESCRIPTION
	Function to handle internal configure request message, for the CSR internal
	codec.
*/
void handleCsrInternalCodecConfigureReq(CsrInternalCodecTaskData *codec, const CODEC_INTERNAL_CONFIG_REQ_T *req);
	

#endif /* CODEC_CSR_INTERNAL_CONFIG_HANDLER_H */

