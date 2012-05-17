/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Battery reading functionality.
*/

/*!
@file    audioAdaptor_battery.c
@brief    This file contains the reading of the battery voltage and acting on the read voltage.

*/


/****************************************************************************
    Header files
*/

#include "audioAdaptor_battery.h"
#include "audioAdaptor_private.h"

#include <battery.h>
#include <stdio.h>


/****************************************************************************
  LOCAL FUNCTIONS
*/


/****************************************************************************
NAME    
    batteryNormal
    
DESCRIPTION
    Called when the battery voltage is detected to be in a Normal state.
    
*/
static void batteryNormal(Task pTask, uint16 pLevel)
{
    if (the_app->charger_state != disconnected) 
        MessageSend(&the_app->task, APP_OK_BATTERY, 0);

    /* reset any low battery warning that may be in place */
    the_app->low_battery_warning = FALSE;
}


/****************************************************************************
NAME    
    batteryLow
    
DESCRIPTION
    Called when the battery voltage is detected to be in a Low state.

*/
static void batteryLow(Task pTask)
{
   
    /* we only want low battery reminders if the headset is ON and not charging */
    if ( (the_app->charger_state == disconnected) && the_app->audioAdaptorPoweredOn )
    {
        /* for the first low battery warning kick off the low batt tone timer, it is
           then self sustaining until the low battery warning goes away */
        if (the_app->low_battery_warning == FALSE)
        {
            /* cancel any oustanding low battery messages to prevent multiple timers running should
               the battery voltage drop in and out of low battery condition */
            MessageCancelAll(&the_app->task, APP_CHECK_FOR_LOW_BATT);
            /* start the low battery event generation timer message,this will repeat
               the tone at the pre-configured time period */
            the_app->low_battery_warning = TRUE;
            /* start by sending the first low batt warning */        
            MessageSend(&the_app->task, APP_LOW_BATTERY, 0); 
            
            /* post a message to send the next low batt warning at the configured timer period */
            MessageSendLater(&the_app->task, APP_CHECK_FOR_LOW_BATT , 0 ,D_SEC(the_app->power->config.battery.low_batt_tone_period));
        }
    }
    /* if the charger is plugged in or in limbo state or warning gone away cancel the flag to 
       prevent low battery warnning tones from being played */
    else if ((the_app->charger_state != disconnected) || the_app->audioAdaptorPoweredOn )
    {
        /* reset low battery flag */        
        the_app->low_battery_warning = FALSE;
    }

}


/****************************************************************************
NAME    
    batteryShutdown
    
DESCRIPTION
    Called when the battery voltage is detected to be in a Shutdown state.
    
*/
static void batteryShutdown(Task pTask)
{
    /*we only want low battery reminders if the headset is ON and not charging*/
    if (the_app->audioAdaptorPoweredOn)
    {    
        the_app->PowerOffIsEnabled = TRUE;
        
        MessageSend(&the_app->task, APP_LOW_BATTERY,  0); 
        MessageSend(&the_app->task, APP_POWEROFF_REQ, 0);
        
    }    
}


/****************************************************************************
NAME    
    handleBatteryVoltageReading
    
DESCRIPTION
    Calculate current battery voltage and check to determine if the level
    has fallen below either the low or critical thresholds.  If the voltage
    has fallen below the low threshold generate a low battery system event.
    If the level has fallen below the critical threshold level then initiate
    a power down sequence.
    
*/
static void handleBatteryVoltageReading(uint32 reading , Task pTask)
{    
    power_type* power = the_app->power;
    
    /* Calculate the current battery voltage in mV */
    uint32 vb = reading;         
    
    DEBUG_POWER(("VBAT: %lumV\n", vb));
    
    /* Store current battery reading */
    power->vbat_task.current_reading = (int16)vb;
    
    DEBUG_POWER(("BAT [%d][%d][%d][%d]\n",                    
        (uint16)(power->config.battery.level_2_threshold  * 20) ,
        (uint16)(power->config.battery.level_1_threshold  * 20) ,
        (uint16)(power->config.battery.low_threshold      * 20) ,
        (uint16)(power->config.battery.shutdown_threshold * 20) 
    )) ;
    
    
    /* Check current voltage level against configured thresholds */
    if (vb > (uint16)(power->config.battery.level_2_threshold * 20) )
        batteryNormal( pTask, 2 );    
    else if (vb > (uint16)(power->config.battery.level_1_threshold * 20) )
        batteryNormal( pTask, 1 );    
    else if (vb > (uint16)(power->config.battery.low_threshold * 20) )
        batteryNormal( pTask, 0 );    
    else if (vb < (uint16)(power->config.battery.shutdown_threshold * 20) )
        batteryShutdown( pTask );
    
    if (vb < (uint16)(power->config.battery.low_threshold * 20) )
        batteryLow(pTask);
}


/****************************************************************************
NAME    
    aio_handler
    
DESCRIPTION
    AIO readings arrive here for processing.
    
*/
static void aio_handler(Task task, MessageId id, Message message)
{
    uint32    reading;
    
    /* Get Power configuration data */
    power_type* power = the_app->power;
    
    /* This function receives messages from the battery library */
    battery_reading_source source = BATTERY_INTERNAL;
    
    switch(id)
    {
        case BATTERY_READING_MESSAGE :        
        {
            /* New reading, extract reading in mV and handle accordingly */
            reading = (*((uint32*)message));
            
            /* Readings can either be AIO0 (Battery Voltage) or AIO1 (Battery Temperature) */
            switch(source)
            {
                case BATTERY_INTERNAL:
                {
                    /* Battery Voltage */
                    handleBatteryVoltageReading(reading , task);
                    break;
                }
                case AIO0:
                case AIO1:            
                case VDD:
                case AIO2:
                case AIO3:
                default:
                {
                    break;
                }
            }    
            /* If initial reading, revert back to default battery reading period */
            if (the_app->initial_battery_reading)
            {
                DEBUG_POWER(("BATT : Initial reading\n")) ;
                the_app->initial_battery_reading = FALSE;
                BatteryInit(&power->vbat_task.state, &power->vbat_task.task, BATTERY_INTERNAL, D_SEC(power->config.battery.monitoring_period));               
            }
            break;
        }        
        default:
        {
            break;
        }
    }
}



/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    batteryInit

DESCRIPTION
    Initialises the battery reading.
    
*/
void batteryInit(void)
{
    power_type* power = the_app->power;
    
    /* Initialise the default battery readings */
    the_app->initial_battery_reading = TRUE;
       
    /* --- Battery Voltage --- */
    /* The battery voltage is monitored at all times.  Initialise the battery
       library to read the battery voltage via BATTERY_INTERNAL */
    power->vbat_task.task.handler = aio_handler;
        
    /* Read battery now */
    the_app->initial_battery_reading = TRUE;
    BatteryInit(&power->vbat_task.state, &power->vbat_task.task, BATTERY_INTERNAL, 0);    
}


/****************************************************************************
NAME    
    batteryCheckLowBatt
    
DESCRIPTION
    Called when the battery voltage is detected to be in a Low state.

*/
void batteryCheckLowBatt(void)
{
    /* if the low battery warning is active send a low battery event and schedule the next check at
       the configured reminder interval period */
    if (the_app->low_battery_warning)
    {
        /* start by sending the first low batt warning only when charger not connected */        
        if (the_app->charger_state == disconnected)
        {
           MessageSend(&the_app->task, APP_LOW_BATTERY, 0); 
        }
        /* post a message to send the next low batt warning at the configured timer period */
        MessageSendLater(&the_app->task, APP_CHECK_FOR_LOW_BATT, 0, D_SEC(the_app->power->config.battery.low_batt_tone_period));                
    }    
}


