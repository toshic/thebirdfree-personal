/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2006-2010
Part of Mono-Headset-SDK 2009.R2

DESCRIPTION
	Interface definition for handling PBAP library messages and functionality

	
FILE
	handle_pbap.h
*/


#ifndef HANDLE_PBAP_H
#define HANDLE_PBAP_H

#include <message.h>

/* Default maximum PBAP Packet Size */
#define PBAP_DEF_PACKET 255


/*
  PBAP Server Message Handler
*/
void handlePbapMessages(MessageId pId, Message pMessage);


/*
  Initialise the PBAP System
*/
void initPbap(void);



#endif /* HANDLE_PBAP_H */
