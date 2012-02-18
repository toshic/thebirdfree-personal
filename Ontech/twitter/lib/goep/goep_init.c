/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_init.c
    
DESCRIPTION
	Initialisation for the Generic Object Exchange Profile (GOEP) library

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/

#include <message.h>
#include <memory.h>

#include "goep.h"
#include "goep_private.h"
#include "goep_init.h"

/****************************************************************************
NAME	
    goepGetTheTask

DESCRIPTION
    This function returns the GOEP library task so that the GOEP library can use it.

RETURNS
    The GOEP library task.
*/
Task goepGetTheTask(void)
{
	goepState *theGoep = PanicUnlessNew(goepState);
	memset(theGoep, 0, sizeof(goepState));
	theGoep->task.handler = goepHandler;
    return &(theGoep->task);
}
