/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Audio-Adaptor-SDK 2009.R1 */
#include "region.h"

#include <string.h>

bool RegionMatchesUUID128(const Region *r, const uint8 uuid[16])
{
    return RegionSize(r) == 16 && memcmp(uuid, r->begin, 16) == 0;
}

