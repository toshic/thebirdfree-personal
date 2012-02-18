/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2006-2007
Part of Audio-Adaptor-SDK Q3-2007.Release

DESCRIPTION
    
*/
#include "audioAdaptor_private.h"
#include "audioAdaptor_led.h"
#include "audioAdaptor_states.h"
#include "leds.h"

#include <pio.h>

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

void ledSetProfile(LedType_t type,bool connected)
{
	switch(type)
	{
		case LedTypeA2DP:
			PioSetDir(LED_A2DP,LED_A2DP);
			if(connected)
				PioSet(LED_A2DP,0);
			else
				PioSet(LED_A2DP,LED_A2DP);
			break;
		case LedTypeHFP:
			PioSetDir(LED_HSP,LED_HSP);
			if(connected)
				PioSet(LED_HSP,0);
			else
				PioSet(LED_HSP,LED_HSP);
			break;
		case LedTypePBAP:
		case LedTypeSCO:
			PioSetDir(LED_SPP,LED_SPP);
			if(connected)
				PioSet(LED_SPP,0);
			else
				PioSet(LED_SPP,LED_SPP);
			break;
		default:
			break;
	}
}

void ledPlayPattern(mvdAppState state)
{
    if (state == AppStatePoweredOff)
    {
        ledsPlay(ledStates[ state ].normal);
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
            ledsPlay(ledStates[ the_app->app_state ].normal);
        }
    }
}
