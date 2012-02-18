/* Copyright (C) Cambridge Silicon Radio Limited 2005-2009 */
/* Part of BlueLab 4.1.2-Release 

FILE NAME
	spp_common.c        

DESCRIPTION
	This file contains common functions used throughout the Spp library.

*/

#include "spp.h"
#include "spp_common.h"
#include "spp_private.h"


/*****************************************************************************/
void sppSetState(SPP *spp, sppState state)
{
	spp->state = state;
}
