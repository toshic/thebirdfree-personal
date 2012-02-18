/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Audio-Adaptor-SDK 2009.R1 */
#include "service.h"

bool ServiceFindAttribute(Region *r, uint16 id, ServiceDataType *type, Region *out)
{
    uint16 found;
    while(ServiceNextAttribute(r, &found, type, out))
	if(found == id)
	    return 1;
    return 0;
}

