/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1
*/

/*!
@file    audioAdaptor_powermanager.c
@brief    Module responsible for managing the battery monitoring and battery charging functionaility.
*/


#include "audioAdaptor_battery.h"
#include "audioAdaptor_charger.h"
#include "audioAdaptor_powermanager.h"
#include "audioAdaptor_private.h"

#include <panic.h>
#include <stdlib.h>

#ifdef DEBUG_POWER
    #define PM_DEBUG(x) DEBUG(x)             
#else
    #define PM_DEBUG(x) 
#endif


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME 
    powerManagerConfig

DESCRIPTION
    Initialise the power handling.
    
*/
bool powerManagerConfig(const power_config_type* config)
{
    bool success = TRUE;
          
    the_app->power = (power_type *)malloc(sizeof(power_type));
      
    PanicNull(the_app->power);
    
    if (config)
    {
        /* Store power configuration */
        the_app->power->config = *config;
        
        /* Initialise the battery sub-system */        
        batteryInit();
    
        /* Initialise the battery charging sub-system */
        chargerInit();
    }
    else
    {
        success = FALSE;
    }

    return success;
}


/****************************************************************************
NAME 
    powerManagerChargerConnected

DESCRIPTION
    The charger has been connected.
    
*/
void powerManagerChargerConnected(void)
{
    chargerConnected();
}


/****************************************************************************
NAME 
    powerManagerChargerDisconnected

DESCRIPTION
    The charger has been disconnected.
    
*/
void powerManagerChargerDisconnected(void)
{
    chargerDisconnected();
}
