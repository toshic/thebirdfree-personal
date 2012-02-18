/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_free.c        

DESCRIPTION

NOTES

*/



/****************************************************************************
	Header files
*/

#include "a2dp_free.h"
#include "a2dp_private.h"

#include <stdlib.h>


/*****************************************************************************/
void a2dpFreeSeidListMemory(A2DP *a2dp)
{
	if (a2dp->sep.list_preferred_local_seids)
	{
		free(a2dp->sep.list_preferred_local_seids);
		a2dp->sep.list_preferred_local_seids = NULL;
	}

	if (a2dp->sep.list_discovered_remote_seids)
	{
		free(a2dp->sep.list_discovered_remote_seids);
		a2dp->sep.list_discovered_remote_seids = NULL;
	}   
}


/*****************************************************************************/
void a2dpFreeConfiguredCapsMemory(A2DP *a2dp)
{
	if (a2dp->sep.configured_service_caps != NULL)
	{
		free(a2dp->sep.configured_service_caps);
		a2dp->sep.configured_service_caps = NULL;
        a2dp->sep.configured_service_caps_size = 0;
	}
}
