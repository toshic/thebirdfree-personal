/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2006-2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
	
*/

#ifndef AUDIOADAPTOR_LED_H
#define AUDIOADAPTOR_LED_H


#include "audioAdaptor_states.h"

#include <stdlib.h>

#define LED_A2DP	(1<<15)
#define LED_HSP		(1<<11)
#define LED_SPP		(1<<10)

typedef enum
{
	LedTypeHFP,
	LedTypeA2DP,
	LedTypeSCO,
	LedTypePBAP,
	LedTypeStreaming
}LedType_t;

void ledSetProfile(LedType_t type,bool connected);

void ledPlayPattern(mvdAppState state);


#endif

