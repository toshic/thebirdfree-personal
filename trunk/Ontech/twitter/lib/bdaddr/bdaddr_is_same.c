/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Audio-Adaptor-SDK 2009.R1 */
#include <bdaddr.h>

bool BdaddrIsSame(const bdaddr *a, const bdaddr *b)
{ return a->nap == b->nap && a->uap == b->uap && a->lap == b->lap; }
