/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
*/

/*!
@file    headset_init.h
@brief   Interface to the headset initialisation functions. 
*/

#ifndef _HEADSET_INIT_H_
#define _HEADSET_INIT_H_


#include "headset_private.h"


/****************************************************************************
  FUNCTIONS
*/

/*************************************************************************
NAME    
    InitHeadsetData
    
DESCRIPTION
    This function initialises the data contained in the hsTaskData structure.    
*/
void InitHeadsetData ( void );


/*************************************************************************
NAME    
    InitUserFeatures
    
DESCRIPTION
    This function initialises all of the user features - this will result in a
    power on message if a user event is configured correctly and the headset will 
    complete the power on.
*/
void InitUserFeatures (void);


/*************************************************************************
NAME    
    InitCodec
    
DESCRIPTION
    This function initialises the codec library.
*/
void InitCodec(void);


/*************************************************************************
NAME    
    InitConnection
    
DESCRIPTION
    This function initialises the connection library.
*/
void InitConnection(void);


/*************************************************************************
NAME    
    InitHfp
    
DESCRIPTION
    This function initialises the HFP library.
*/
void InitHfp(void);


/*************************************************************************
NAME    
    InitA2dp
    
DESCRIPTION
    This function initialises the A2DP library.
*/
void InitA2dp(void);


/*************************************************************************
NAME    
    InitAvrcp
    
DESCRIPTION
    This function initialises the AVRCP library.
*/
void InitAvrcp(void);


/*************************************************************************
NAME    
    InitSeidConnectPriority
    
DESCRIPTION
    Retrieves a list of the preferred Stream End Points to connect with.
*/
uint16 InitSeidConnectPriority(uint8 seid, uint8 *seid_list);


/*************************************************************************
NAME    
    InitA2dpPlugin
    
DESCRIPTION
    Retrieves the audio plugin for the requested SEP.
*/
Task InitA2dpPlugin(uint8 seid);


/*************************************************************************
NAME    
    InitConfigurationMemory
    
DESCRIPTION
    Allocates the memory required for the configuration.
*/
void InitConfigurationMemory(void);


#endif /* _HEADSET_INIT_H_ */
