/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Audio-Adaptor-SDK 2009.R1 */
#include "region.h"

uint32 RegionReadUnsigned(const Region *r)
{
    uint32 v = 0;
    const uint8 *p;
    for(p = r->begin; p != r->end; ++p)
	v = (v<<8) | *p;
    return v;
}
