/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_get_sep.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/

#include "a2dp_get_sep.h"
#include "a2dp_private.h"

#include <stdlib.h>


/*****************************************************************************/
sep_type *getSepInstanceBySeid(A2DP *a2dp, uint16 seid)
{
	uint16 i;
	sep_type *pSeps = a2dp->sep.sep_list->seps;

	for (i=0; i<a2dp->sep.sep_list->size_seps; i++)
	{
		if (pSeps->sep_config->seid == seid)
			return pSeps;

		pSeps++;
	}

	return NULL;
}

