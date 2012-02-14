/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1
*/

/*!
@file    audioAdaptor_powermanager.h
@brief    Interface to module responsible for managing the battery monitoring and battery charging functionaility.
*/
#ifndef AUDIOADAPTOR_POWER_MANAGER_H
#define AUDIOADAPTOR_POWER_MANAGER_H


#include "audioAdaptor_private.h"


bool powerManagerConfig(const power_config_type* config);

void powerManagerChargerConnected(void);

void powerManagerChargerDisconnected(void);


#endif /* AUDIOADAPTOR_POWER_MANAGER_H */
