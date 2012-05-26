/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Stereo-Headset-SDK 2009.R2 */
#include <bdaddr.h>

bool BdaddrIsZero(const bdaddr *b)
{ return !b->nap && !b->uap && !b->lap; }
