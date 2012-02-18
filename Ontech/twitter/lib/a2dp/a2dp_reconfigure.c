/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_reconfigure.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_caps_parse.h"
#include "a2dp_codec_handler.h"
#include "a2dp_private.h"
#include "a2dp_reconfigure_handler.h"
#include "a2dp_send_packet_handler.h"
#include "a2dp_signalling_handler.h"

#include <stdlib.h>
#include <string.h>


/*****************************************************************************/
void A2dpReconfigure(A2DP *a2dp, uint16 size_sep_caps , uint8 *sep_caps)
{
	/* Send an internal message to initiate the connection. */
	MAKE_A2DP_MESSAGE(A2DP_INTERNAL_RECONFIGURE_REQ);

#ifdef A2DP_DEBUG_LIB
	if (!a2dp)
		A2DP_DEBUG(("A2dpReconfigure NULL instance\n"));
#endif

	message->size_sep_caps = size_sep_caps;
	message->sep_caps = sep_caps;
	MessageSend(&a2dp->task, A2DP_INTERNAL_RECONFIGURE_REQ, message);    
}


