/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    oppc_init.c
    
DESCRIPTION
	Initialisation for the Object Push Profile Client(oppc) library
*/

#include <message.h>
#include <print.h>

#include "oppc.h"
#include "oppc_private.h"

/****************************************************************************
NAME	
    oppcGetTheTask

DESCRIPTION
    This function returns the oppc library task so that the oppc library can use it.

RETURNS
    The oppc library task.
*/
Task oppcGetTheTask(void)
{
	oppcState *theOppc = PanicUnlessNew(oppcState);
	theOppc->task.handler = oppcHandler;
    return &(theOppc->task);
}
