/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    spp_common.h
    
DESCRIPTION
	Header file for the common spp functions.
*/

#ifndef SPP_COMMON_H_
#define SPP_COMMON_H_

#include "spp_private.h"


/****************************************************************************
NAME	
	sppSetState

DESCRIPTION
	Update the local spp state.
*/
void sppSetState(SPP *spp, sppState state);


#endif /* SPP_COMMON_H_ */
