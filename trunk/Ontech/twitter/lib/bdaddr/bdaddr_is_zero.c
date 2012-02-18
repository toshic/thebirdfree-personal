/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Audio-Adaptor-SDK 2009.R1 */
#include <bdaddr.h>

bool BdaddrIsZero(const bdaddr *b)
{ return !b->nap && !b->uap && !b->lap; }
