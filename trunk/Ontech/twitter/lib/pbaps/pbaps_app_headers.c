/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    pbaps_app_headers.h
    
DESCRIPTION
	PhoneBook Access Profile Server Library - Handles the addition of App. Specific. Parameters.

*/

#include <vm.h>
#include <bdaddr.h>
#include <print.h>
#include <sink.h>

#include <pbap_common.h>
#include <goep.h>

#include "pbaps.h"
#include "pbaps_private.h"

#include "goep_apphdrs.h"


uint16 pbapsAddApplicationHeaders(pbapsState *state, Sink sink)
{
	uint16 lenUsed = 0;
	uint16 len;
	
    PRINT(("pbapsAddApplicationHeaders\n"));
	
	/* Application specific parameters.  Only add if different from the default value */
	if (state->pbook_size != pbap_param_pbook_size_def)
	{
		len = Goep_apphdr_AddUint16(sink, pbap_param_pbook_size, state->pbook_size);
		PBAPS_ASSERT((len != 0), ("PBAPS - Could not add Phonebook Size\n"));
		lenUsed+=len;
	}
	if (state->new_missed != pbap_param_missed_calls_def)
	{
		len = Goep_apphdr_AddUint8(sink, pbap_param_missed_calls, state->new_missed);
		PBAPS_ASSERT((len != 0), ("PBAPS - Could not add New Missed Calls\n"));
		lenUsed+=len;
	}
	
	return lenUsed;
}
