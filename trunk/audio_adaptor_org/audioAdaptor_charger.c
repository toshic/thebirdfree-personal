/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Charger handling functionality.
*/

/*!
@file    audioAdaptor_charger.c
@brief    This file contains the battery charging functionality for NiMH batteries.
*/


/****************************************************************************
    Header files
*/

#include "audioAdaptor_charger.h"
#include "audioAdaptor_led.h"
#include "audioAdaptor_private.h"

#include <charger.h>
#include <pio.h>
#include <vm.h>
#include <stdio.h>


/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************
NAME    
    charger_handler
    
DESCRIPTION
    Messages for the charger control task arrive here

*/
void chargerHandler(void)
{
#ifndef NO_CHARGER_TRAPS
    charger_status lcharger_status = NOT_CHARGING ;
    
    lcharger_status = ChargerStatus() ;   

    DEBUG_POWER(("CH: monitor status = %d\n",lcharger_status));

    switch (lcharger_status)
    {            
        case CHARGING :     
        case TRICKLE_CHARGE :
        case FAST_CHARGE :
        {
            MessageSend(&the_app->task, APP_FAST_CHARGE, 0);
            break ;
        }    
        case DISABLED_ERROR :     
        case STANDBY :    
        case NO_POWER :    
        case NOT_CHARGING :
        default:
        {
            MessageSend(&the_app->task, APP_TRICKLE_CHARGE, 0);
            break ;    
        }
    }
#endif                
    /* Monitoring period of 1 second while the charger is plugged in */                
    MessageSendLater(&the_app->task, APP_CHARGER_MONITOR, 0, D_SEC(1));                
}


/****************************************************************************
NAME    
    chargerInit
    
DESCRIPTION
    Initialise the charger code.

*/
void chargerInit(void)
{
    DEBUG_POWER(("CH: Init\n"));
    
    /* Assume charger disconnected at boot time */
    the_app->charger_state = disconnected;
}


/****************************************************************************
NAME
    chargerIsConnected
 
DESCRIPTION
    Returns if the charger is connected or not.

*/ 
bool chargerIsConnected ( void )
{
    bool connected = FALSE;
    
#ifndef NO_CHARGER_TRAPS    
    switch (ChargerStatus())
    {
        case TRICKLE_CHARGE:
        case FAST_CHARGE:
        case CHARGING:
        case STANDBY:
        case DISABLED_ERROR:
        {
            connected = TRUE;
            break;
        }    
        case NO_POWER:
        case NOT_CHARGING:
        {
            break;
        }    
        default:
        {
            break;
        }
    }
    DEBUG_POWER(("B : CHARGER CONN [%d]\n",connected));
#endif    
    
    return connected;
}


/****************************************************************************
NAME
    chargerConnected
 
DESCRIPTION
    Called when the charger is connected.

*/
void chargerConnected(void)
{
    DEBUG_POWER(("CH: Connected\n"));
    
    switch(the_app->charger_state)
    {
        case disconnected:
        {
            /* LiON Handled automatically */
                
            /* Fast charge state */
            the_app->charger_state = fast_charge;
                    
            /* Start monitoring onchip LiON battery charger status */
            MessageSend(&the_app->task, APP_CHARGER_MONITOR, 0);
            
            /* update LED pattern */
            ledPlayPattern(the_app->app_state);
        
            break;
        }    
        default:
        {
            break;
        }
    }    
}


/****************************************************************************
NAME
    chargerDisconnected
 
DESCRIPTION
    Called when the charger is disconnected.

*/
void chargerDisconnected(void)
{
    DEBUG_POWER(("CH: Disconnected\n"));
    
    switch(the_app->charger_state)
    {
        case trickle_charge:
        case fast_charge:
        case charge_error:
        {
            /* The charger has just been removed, move to disconnected state */
            the_app->charger_state = disconnected;

            /* Cancel any pending LiON battery charger status monitoring messages */
            MessageCancelAll(&the_app->task, APP_CHARGER_MONITOR);
            
            /* update LED pattern */
            ledPlayPattern(the_app->app_state);
            
            break;
        }    
        default:
        {
            break;
        }
    }
}

