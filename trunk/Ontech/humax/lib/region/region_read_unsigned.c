/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Stereo-Headset-SDK 2009.R2 */
#include "region.h"

uint32 RegionReadUnsigned(const Region *r)
{
    uint32 v = 0;
    const uint8 *p;
    for(p = r->begin; p != r->end; ++p)
	v = (v<<8) | *p;
    return v;
}
