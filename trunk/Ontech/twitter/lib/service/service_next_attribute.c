/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Audio-Adaptor-SDK 2009.R1 */
#include "service_private.h"

bool ServiceNextAttribute(Region *r, uint16 *id, ServiceDataType *type, Region *value)
{ 
    Region id_region;
    if(ServiceGetValue(r, type, &id_region) && *type == sdtUnsignedInteger && RegionSize(&id_region) == 2)
    {
	*id = serviceUnpack16(id_region.begin);
	return ServiceGetValue(r, type, value);
    }
    return 0;
}
