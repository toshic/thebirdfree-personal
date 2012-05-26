/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Stereo-Headset-SDK 2009.R2 */
#include "region.h"

void RegionWriteUnsigned(const Region *r, uint32 value)
{
    uint8 *p = (uint8 *) (r->end);
    while(p != r->begin)
    {
	*--p = (uint8) (value & 0xFF);
	value >>= 8;
    }
}

