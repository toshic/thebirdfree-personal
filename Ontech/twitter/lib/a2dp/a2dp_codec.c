/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_codec.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_private.h"

#include <string.h>


/*****************************************************************************/
void A2dpConfigureCodecResponse(A2DP *a2dp, bool accept, uint16 size_codec_service_caps, uint8 *codec_service_caps)
{
	MAKE_A2DP_MESSAGE_WITH_LEN(A2DP_INTERNAL_CONFIGURE_CODEC_RSP, size_codec_service_caps);       

#ifdef A2DP_DEBUG_LIB
	if (!a2dp)
		A2DP_DEBUG(("A2dpConfigureCodecResponse NULL instance\n"));
#endif

	message->accept = accept;
	message->size_codec_service_caps = size_codec_service_caps;
	memmove(message->codec_service_caps, codec_service_caps, size_codec_service_caps); 

	MessageSend(&a2dp->task, A2DP_INTERNAL_CONFIGURE_CODEC_RSP, message);
}

