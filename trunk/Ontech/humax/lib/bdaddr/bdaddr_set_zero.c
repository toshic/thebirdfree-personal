/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Stereo-Headset-SDK 2009.R2 */
#include <bdaddr.h>

void BdaddrSetZero(bdaddr *b)
{
    b->nap = 0;
    b->uap = 0;
    b->lap = 0;
}
