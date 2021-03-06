/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_init.h
    
DESCRIPTION
	Initialisation for the Generic Object Exchange Profile (GOEP) library

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/


#ifndef GEOP_INIT_H_
#define GEOP_INIT_H_

/****************************************************************************
NAME	
    goepGetTheTask

DESCRIPTION
    This function returns the GOEP library task so that the GOEP library can use it.

RETURNS
    The GOEP library task.
*/
Task goepGetTheTask(void);

#endif /* GEOP_INIT_H_ */
