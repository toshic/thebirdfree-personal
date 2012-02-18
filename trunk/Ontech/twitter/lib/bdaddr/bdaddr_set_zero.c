/* Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009 */
/* Part of Audio-Adaptor-SDK 2009.R1 */
#include <bdaddr.h>

void BdaddrSetZero(bdaddr *b)
{
    b->nap = 0;
    b->uap = 0;
    b->lap = 0;
}
