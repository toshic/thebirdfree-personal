/* Copyright (C) Cambridge Silicon Radio Limited 2005-2009 */
/* Part of Stereo-Headset-SDK 2009.R2 */
#include "service.h"

bool ServiceFindAttribute(Region *r, uint16 id, ServiceDataType *type, Region *out)
{
    uint16 found;
    while(ServiceNextAttribute(r, &found, type, out))
	if(found == id)
	    return 1;
    return 0;
}

