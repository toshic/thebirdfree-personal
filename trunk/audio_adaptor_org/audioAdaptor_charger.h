/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Charger handling functionality.
*/

/*!
@file    audioAdaptor_charger.h
@brief   Interface to the audioAdaptor charger controls. 
*/

#ifndef _AUDIOADAPTOR_CHARGER_H_
#define _AUDIOADAPTOR_CHARGER_H_


#include "audioAdaptor_powermanager.h"


void chargerHandler(void);

void chargerInit(void);

bool chargerIsConnected ( void );

void chargerConnected(void);

void chargerDisconnected(void);


#endif /* _AUDIOADAPTOR_CHARGER_H_ */
