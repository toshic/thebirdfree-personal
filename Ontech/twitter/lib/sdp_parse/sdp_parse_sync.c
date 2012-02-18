/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    sdp_parse_arbitrary.c
    
DESCRIPTION
	Contains functions for accessing SYNC specific fields in a service record
*/

#include "sdp_parse.h"
#include <service.h>
#include <region.h>

/************************************ Private *****************************/

/* Find SYNC Repository */

static bool findSyncStore(const uint8 size_service_record, const uint8* service_record, Region* value)
{
	ServiceDataType type;
    Region record;
    record.begin = service_record;
    record.end   = service_record + size_service_record;
	
	if (ServiceFindAttribute(&record, saSyncStores, &type, value))
		if(type == sdtUnsignedInteger)
		{
			/* Found the Attribute Field */
			return TRUE;
		}
	/* Failed */
	return FALSE;
}

/************************************ Public ******************************/

/* Access SYNC Repository */

bool SdpParseGetSyncStore(const uint8 size_service_record, const uint8* service_record, uint8* store)
{
	Region value;
	if(findSyncStore(size_service_record, service_record, &value))
	{
		*store = RegionReadUnsigned(&value);
		/* Accessed Successfully */
		return TRUE;
	}
	/* Failed */
	return FALSE;
}

/* Insert SYNC Repository */

bool SdpParseInsertSyncStore(const uint8 size_service_record, const uint8* service_record, uint8 store)
{
	Region value;
	if(findSyncStore(size_service_record, service_record, &value))
	{
		RegionWriteUnsigned(&value, store);
		/* Inserted Successfully */
		return TRUE;
	}
	/* Failed */
	return FALSE;
}
