/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_close.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "a2dp.h"
#include "a2dp_private.h"

#include <stdlib.h>


/*****************************************************************************/
void A2dpClose(A2DP *a2dp)
{

#ifdef A2DP_DEBUG_LIB
	if (!a2dp)
		A2DP_DEBUG(("A2dpClose NULL instance\n"));
#endif

	MessageSend(&a2dp->task, A2DP_INTERNAL_CLOSE_REQ, 0);
}

