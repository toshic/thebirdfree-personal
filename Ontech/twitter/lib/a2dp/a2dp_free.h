/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    a2dp_free.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_FREE_H_
#define A2DP_FREE_H_

#include "a2dp.h"


/****************************************************************************
NAME	
	a2dpFreeSeidListMemory

DESCRIPTION
	Free SEID list memory.
*/
void a2dpFreeSeidListMemory(A2DP *a2dp);


/****************************************************************************
NAME	
	a2dpFreeConfiguredCapsMemory

DESCRIPTION
	Free configured caps memory.
*/
void a2dpFreeConfiguredCapsMemory(A2DP *a2dp);


#endif /* A2DP_FREE_H_ */
