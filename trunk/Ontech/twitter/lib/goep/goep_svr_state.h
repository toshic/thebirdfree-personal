/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2010
Part of Mono-Headset-SDK 2009.R2

FILE NAME
    goep_svr_state.h
    
DESCRIPTION
	Server driven state machine header for the Generic Object Exchange Profile (GOEP) library

	This is the base profile for FTP Server, FTP Client and the Object Push 
	Profile (OPP) Libraries.
*/


#ifndef GEOP_SVR_STATE_H_
#define GEOP_SVR_STATE_H_

/* Server Driven State Machine for the GOEP library */
bool handleServerCommand(goepState *sess, Source src);

#endif /* GEOP_SVR_STATE_H_ */
