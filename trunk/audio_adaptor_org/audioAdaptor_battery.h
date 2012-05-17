/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Battery reading functionality.
*/

/*!
@file    audioAdaptor_battery.h
@brief  Interface to battery functionality.
*/

#ifndef _AUDIOADAPTOR_BATTERY_H_
#define _AUDIOADAPTOR_BATTERY_H_


#include "audioAdaptor_powermanager.h"
#include "audioAdaptor_private.h" 


void batteryInit(void);

void batteryCheckLowBatt(void);


#endif /* _AUDIOADAPTOR_BATTERY_H_ */
