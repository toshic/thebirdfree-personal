/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	a2dp_sep_in_use.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/

#include "a2dp_private.h"

#include <stdlib.h>


/*****************************************************************************/
void A2dpSetSepInUse(device_sep_list *sep_list, uint8 seid, bool in_use)
{
	uint16 i;
	sep_type *pSeps;

#ifdef A2DP_DEBUG_LIB
	if (!sep_list)
		A2DP_DEBUG(("A2dpSetSepInUse NULL SEPs\n"));
#endif

	pSeps = sep_list->seps;

	for (i=0; i<sep_list->size_seps; i++)
	{
		if (pSeps->sep_config->seid == seid)
		{
			if (pSeps->in_use != in_use)
				pSeps->in_use = in_use;

			return;
		}

		pSeps++;
	}
}


