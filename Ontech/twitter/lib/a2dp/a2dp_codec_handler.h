/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_sep_handler.h
    
DESCRIPTION
	
*/

#ifndef A2DP_CODEC_HANDLER_H_
#define A2DP_CODEC_HANDLER_H_

#include "a2dp_private.h"


/****************************************************************************
NAME	
	processCodecInfo

DESCRIPTION
	Process the codec capabilities selected.
	
*/
void processCodecInfo(A2DP *a2dp, bool accept, uint16 size_codec_service_caps, const uint8 *codec_service_caps);


/****************************************************************************
NAME	
	a2dpHandleSelectingCodecSettings

DESCRIPTION
	Select the correct capabilities depending on which codec is selected.
		
*/
bool a2dpHandleSelectingCodecSettings(A2DP *a2dp, uint16 size_service_caps, uint8 *service_caps);


/****************************************************************************
NAME	
	a2dpHandleConfigureCodecRes

DESCRIPTION
	Handle the A2DP_INTERNAL_CONFIGURE_CODEC_RSP message.
		
*/
void a2dpHandleConfigureCodecRes(A2DP *a2dp, const A2DP_INTERNAL_CONFIGURE_CODEC_RSP_T *res);


/****************************************************************************
NAME	
	a2dpSendCodecAudioParams

DESCRIPTION
	Choose configured codec parameterss and send them to the application.
		
*/
void a2dpSendCodecAudioParams(A2DP *a2dp);


#endif /* A2DP_CODEC_HANDLER_H_ */
