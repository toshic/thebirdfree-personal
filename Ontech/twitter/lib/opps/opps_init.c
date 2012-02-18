/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    opps_init.c
    
DESCRIPTION
	Initialisation for the Object Push Profile Server(opps) library
*/

#include <message.h>
#include <print.h>

#include "opps.h"
#include "opps_private.h"

/****************************************************************************
NAME	
    oppsGetTheTask

DESCRIPTION
    This function returns the opps library task so that the opps library can use it.

RETURNS
    The opps library task.
*/
Task oppsGetTheTask(void)
{
	oppsState *theOpps = PanicUnlessNew(oppsState);
	theOpps->task.handler = oppsHandler;
    return &(theOpps->task);
}
