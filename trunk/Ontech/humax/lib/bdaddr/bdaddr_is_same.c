/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Stereo-Headset-SDK 2009.R2 */
#include <bdaddr.h>

bool BdaddrIsSame(const bdaddr *a, const bdaddr *b)
{ return a->nap == b->nap && a->uap == b->uap && a->lap == b->lap; }
