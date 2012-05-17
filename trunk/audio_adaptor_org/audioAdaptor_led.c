/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2006-2007
Part of Audio-Adaptor-SDK Q3-2007.Release

DESCRIPTION
    
*/

#include "audioAdaptor_charger.h"
#include "audioAdaptor_led.h"
#include "audioAdaptor_states.h"
#include "leds.h"


/*! 
    @brief a led state pattern type
*/
typedef struct
{
	unsigned normal:8;  
	unsigned low_battery:8 ;
	unsigned charging:8 ;
	unsigned reserved:8 ;
} ledStatePattern_t ;

static const ledStatePattern_t ledStates [ MaxAppStates ] = 
{
/*AppStateUninitialised*/ {LEDS_OFF,               LEDS_OFF,	           RED_ON,                     0 } ,
/*AppStateInitialising*/  {LEDS_OFF,               LEDS_OFF,               RED_ON,                     0 } ,
/*AppStateIdle*/          {BLUE_ONE_SEC_ON_RPT,    RED_ONE_SEC_ON_RPT,     RED_ON_BLUE_ONE_SEC_ON_RPT, 0 } ,
/*AppStateInquiring*/     {RED_BLUE_ALT_RPT_FAST,  RED_TWO_FLASHES_RPT,    RED_BLUE_BOTH_RPT_THREE,    0 } ,
/*AppStateSearching*/     {RED_BLUE_ALT_RPT_FAST,  RED_TWO_FLASHES_RPT,    RED_BLUE_BOTH_RPT_THREE,    0 } ,
/*AppStateConnecting*/    {RED_BLUE_ALT_RPT,       RED_REP_BLUE_ALT_RPT,   RED_BLUE_BOTH_RPT,          0 } ,
/*AppStateStreaming*/     {BLUE_SHORT_ON_RPT,      RED_SHORT_ON_RPT,       RED_ON_BLUE_SHORT_RPT,      0 } ,
/*AppStateInCall*/        {RED_BLUE_BOTH_RPT,      RED_SHORT_ON_RPT,       RED_BLUE_BOTH_RPT_FAST,     0 } ,
/*AppStateEnteringDfu*/   {RED_BLUE_BOTH_RPT_FAST, RED_BLUE_BOTH_RPT_FAST, RED_BLUE_BOTH_RPT_FAST,     0 } ,
/*AppStateLowBattery*/    {LEDS_OFF,               LEDS_OFF,               RED_ON,                     0 } ,
/*AppStatePoweredOff*/    {LEDS_OFF,               LEDS_OFF,               RED_ON,                     0 } 
} ;


void ledPlayPattern(mvdAppState state)
{
    if (state == AppStatePoweredOff)
    {
        if (the_app->a2dp_source == SourceAnalog && chargerIsConnected())
        {
            ledsPlay(ledStates[ state ].charging);
        }
        else 
        {
            ledsPlay(ledStates[ state ].normal);
        }
    }
    else if (state == AppStateLowBattery)
    {
        if (the_app->audioAdaptorPoweredOn)
        {
            ledsPlay(ledStates[ the_app->app_state ].low_battery);
        }
    }
    else
    {
        if (the_app->audioAdaptorPoweredOn)
        {
            if (the_app->a2dp_source == SourceAnalog && chargerIsConnected())
            {
                ledsPlay(ledStates[ the_app->app_state ].charging);
            }
            else 
            {
                ledsPlay(ledStates[ the_app->app_state ].normal);
            }
        }
    }
}
