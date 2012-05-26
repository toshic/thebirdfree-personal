/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_rom_led_manager.c
@brief    Module responsible for managing the ROM LEDs.
*/

#ifdef ROM_LEDS


#include "headset_configmanager.h"
#include "headset_debug.h"
#include "headset_statemanager.h"
#include "headset_led_manager.h"
#include "headset_leds.h"
#include "headset_pio.h"
#include "headset_private.h"

#include <stddef.h>
#include <panic.h>
#include <pio.h>


#ifdef DEBUG_LM
#define LM_DEBUG(x) DEBUG(x)
#else
#define LM_DEBUG(x) 
#endif


/****************************************************************************
    LOCAL FUNCTION PROTOTYPES
*/

 /*internal method to provide a pointer to one of the malloced patterns*/
static LEDPattern_t * LMGetPattern ( void )  ;
    /*method to release a pattern - actually clears data held in pattern so it can be used again*/
static void LMResetPattern ( LEDPattern_t * pPattern ) ;

static bool LMIsPatternEmpty (LEDPattern_t * pPattern ) ;

static LEDPattern_t *  LMAddPattern ( LEDPattern_t * pSourcePattern , LEDPattern_t * pDestPattern ) ;

 /*methods to allocate/ initialise the space for the patterns and mappings*/
static void LEDManagerInitStatePatterns   ( void ) ;
static void LEDManagerInitEventPatterns   ( void ) ;
static void LEDManagerInitActiveLEDS      ( void ) ;
static void LEDManagerCreateFilterPatterns( void ) ;


/****************************************************************************
  FUNCTIONS
*/

/*****************************************************************************/
void LEDManagerInit ( void ) 
{
    uint16 lIndex = 0 ;
	LedTaskData * ptheLEDTask = &theHeadset.theLEDTask;
  
    LM_DEBUG(("LM Init :\n")) ;
   
    for (lIndex = 0 ; lIndex < LM_MAX_NUM_PATTERNS ; lIndex ++ )
    {
            /*make sure the pattern is released and ready for use*/
        LMResetPattern ( &ptheLEDTask->gPatterns[lIndex] )  ;      
    }
    
    /* create the patterns we want to use*/
    LEDManagerInitStatePatterns() ;
    
    LEDManagerInitActiveLEDS() ;
    
    LEDManagerInitEventPatterns() ;
    
    ptheLEDTask->Queue.Event1 = 0 ;
    ptheLEDTask->Queue.Event2 = 0 ;
    ptheLEDTask->Queue.Event3 = 0 ;
    ptheLEDTask->Queue.Event4 = 0 ;
    
	/*the filter information*/
    LEDManagerCreateFilterPatterns() ;
    
    LedsInit() ;

}


/*****************************************************************************/
void LEDManagerAddLEDStatePattern ( headsetHfpState pState, headsetA2dpState pA2dpState , LEDPattern_t* pPattern )
{  
	LedTaskData * ptheLEDTask = &theHeadset.theLEDTask;
    uint16 led_state =  stateManagerGetCombinedLEDState(pState, pA2dpState);
	
    ptheLEDTask->gStatePatterns [ led_state ] = LMAddPattern ( pPattern ,  ptheLEDTask->gStatePatterns [ led_state ] )  ;
    LM_DEBUG(("LM: AddState[%x][%x][%x]\n" , pState , pA2dpState ,(int) ptheLEDTask->gStatePatterns [ led_state ] )) ;
}


/*****************************************************************************/
void LEDManagerAddLEDFilter ( LEDFilter_t* pLedFilter ) 
{
	LedTaskData * ptheLEDTask = &theHeadset.theLEDTask;
	
    if ( ptheLEDTask->gLMNumFiltersUsed < LM_NUM_FILTER_EVENTS )
    {
        /*then add the filter pattern*/
       ptheLEDTask->gEventFilters [ ptheLEDTask->gLMNumFiltersUsed ] = *pLedFilter ;
  
       LM_DEBUG(("LM :  AF[%x][%d][%d][%d][%d] [%d][%d] [%d][%d]\n", pLedFilter->Event ,pLedFilter->IsFilterActive , 
                                                   pLedFilter->Speed, pLedFilter->SpeedAction, pLedFilter->Colour ,
                                                   pLedFilter->OverideLEDActive , pLedFilter->OverideLED , 
                                                   pLedFilter->FollowerLEDActive , pLedFilter->FollowerLEDDelay
                                                   )) ;
     /*inc the filters*/
        ptheLEDTask->gLMNumFiltersUsed ++ ;
    }
}


/*****************************************************************************/
void LEDManagerAddLEDEventPattern ( headsetEvents_t pEvent , LEDPattern_t* pPattern )
{
	LedTaskData * ptheLEDTask = &theHeadset.theLEDTask;
	
    ptheLEDTask->gEventPatterns [ pEvent ] = LMAddPattern ( pPattern , ptheLEDTask->gEventPatterns [ pEvent ]  ) ;   
    
    LM_DEBUG(("LM: AddEvent[%x] [%x]\n" , pEvent ,(int)ptheLEDTask->gEventPatterns [ pEvent ])) ;    

}


/*****************************************************************************/
void LEDManagerIndicateEvent ( MessageId pEvent ) 
{
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
	
    /*only indicate if LEDs are enabled*/
    if ((pLEDTask->gLEDSEnabled ) ||
        LedsEventCanOverideDisable( pEvent ) ||
        LedActiveFiltersCanOverideDisable())
    {
    
        LM_DEBUG(("LM : IE[%x]\n",pEvent )) ;
            /*if there is an event configured*/
        if ( pLEDTask->gEventPatterns [pEvent] != NULL )
        {
                /*only update if wer are not currently indicating an event*/
            if ( ! pLEDTask->gCurrentlyIndicatingEvent )
            {
                LedsIndicateEvent ( pEvent ) ;  
            }    
            else
            {                          
                if (theHeadset.features.QueueLEDEvents )
                {
             
                        /*try and add it to the queue*/
                    LM_DEBUG(("LM: Queue LED Event [%x]\n" , pEvent )) ;
                
                    if ( pLEDTask->Queue.Event1 == 0)
                    {
                        pLEDTask->Queue.Event1 = ( pEvent ) ;
                    }
                    else if ( pLEDTask->Queue.Event2 == 0)
                    {
                        pLEDTask->Queue.Event2 = ( pEvent ) ;
                    }
                    else if ( pLEDTask->Queue.Event3 == 0)
                    {
                        pLEDTask->Queue.Event3 = ( pEvent ) ;
                    }
                    else if ( pLEDTask->Queue.Event4 == 0)
                    {
                        pLEDTask->Queue.Event4 = ( pEvent ) ;
                    }    
                    else
                    {
                        LM_DEBUG(("LM: Err Queue Full!!\n")) ;
                    }
                }    
            }
        }
        else
        {
            LM_DEBUG(("LM: NoEvPatCfg\n")) ;
        }  
        }
    else
    {
        LM_DEBUG(("LM : No IE[%x] disabled\n",pEvent )) ;
    }
    
    /*indicate a filter if there is one present*/
    LedsCheckForFilter ( pEvent ) ;
}


/*****************************************************************************/
void LEDManagerIndicateState ( headsetHfpState pState , headsetA2dpState pA2dpState )  
{   
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
    uint16 led_state = stateManagerGetCombinedLEDState(pState, pA2dpState);
    
    /* only indicate if LEDs are enabled*/
    if ((pLEDTask->gLEDSEnabled ) ||
        LedsStateCanOverideDisable( pState , pA2dpState ) ||
        LedActiveFiltersCanOverideDisable())
    {
            /*if there is no pattern associated with the sate then do nothing*/
        if ( pLEDTask->gStatePatterns[ led_state ] == NULL )
        {
            LM_DEBUG(("LM: NoStCfg[%x][%x]\n",pState, pA2dpState)) ;
            LedsIndicateNoState() ;
        }
        else
        {
            LM_DEBUG(("LM : IS[%x][%x]\n", pState, pA2dpState)) ;
            LedsIndicateState ( pState, pA2dpState );
        } 
    }
    else
    {
        LM_DEBUG(("LM : DIS NoStCfg[%x]\n", pState)) ;
        LedsIndicateNoState();
    }
}


/*****************************************************************************/
void LedManagerDisableLEDS ( void )
{
    LM_DEBUG(("LM Disable LEDS\n")) ;

    /*turn off all current LED Indications if not overidden by state or filter */
    if (!LedsStateCanOverideDisable(stateManagerGetHfpState(), stateManagerGetA2dpState()) && !LedActiveFiltersCanOverideDisable())
    {
        LedsIndicateNoState() ;
    }    
    
    theHeadset.theLEDTask.gLEDSEnabled = FALSE ;
}


/*****************************************************************************/
void LedManagerEnableLEDS ( void )
{
    LM_DEBUG(("LM Enable LEDS\n")) ;
    
    theHeadset.theLEDTask.gLEDSEnabled = TRUE ;
         
    LEDManagerIndicateState ( stateManagerGetHfpState () , stateManagerGetA2dpState () ) ;    
}


/*****************************************************************************/
void LedManagerToggleLEDS ( void ) 
{
    if ( theHeadset.theLEDTask.gLEDSEnabled )
    {
        LedManagerDisableLEDS() ;        
    }
    else
    {
        LedManagerEnableLEDS() ;
    }
}


/*****************************************************************************/
void LedManagerResetLEDIndications ( void )
{    
    LedsResetAllLeds() ;
    
    theHeadset.theLEDTask.gCurrentlyIndicatingEvent = FALSE ;
    
    LEDManagerIndicateState ( stateManagerGetHfpState() , stateManagerGetA2dpState () ) ;
}


/*****************************************************************************/
void LEDManagerResetStateIndNumRepeatsComplete  ( void ) 
{
    /*get state*/
    uint16 led_state = stateManagerGetCombinedLEDState(stateManagerGetHfpState(), stateManagerGetA2dpState());
    /*get pattern*/ 
    LEDPattern_t * lPattern = theHeadset.theLEDTask.gStatePatterns[led_state] ;

    if (lPattern)
    {
        LEDActivity_t * lLED   = &theHeadset.theLEDTask.gActiveLEDS[lPattern->LED_A] ;
        if (lLED)
        {
            /*reset num repeats complete to 0*/
            lLED->NumRepeatsComplete = 0 ;
        }    
    }
}


/*****************************************************************************/
#ifdef DEBUG_LM
void LMPrintPattern ( LEDPattern_t * pLED ) 
{
    const char * const lColStrings [ 5 ] =   {"LED_E ","LED_A","LED_B","ALT","Both"} ;

    LM_DEBUG(("[%d][%d] [%d][%d][%d] ", pLED->LED_A , pLED->LED_B, pLED->OnTime, pLED->OffTime, pLED->RepeatTime)) ;  
    LM_DEBUG(("[%d] [%d] [%s]\n",       pLED->NumFlashes, pLED->TimeOut, lColStrings[pLED->Colour])) ;    
    LM_DEBUG(("[%d]\n",       pLED->OverideDisable)) ;    
}
#endif


/****************************************************************************
NAME 
    LEDManagerInitActiveLEDS

DESCRIPTION
    Creates the active LED space for the number of leds the system supports.

*/
static void LEDManagerInitActiveLEDS ( void ) 
{
    uint16 lIndex = 0; 
 
    for ( lIndex= 0 ; lIndex < HEADSET_NUM_LEDS ; lIndex ++ )
    {
        LedsSetLedActivity ( &theHeadset.theLEDTask.gActiveLEDS [ lIndex ] , IT_Undefined , 0 , 0 ) ;    
    }
}


/****************************************************************************
NAME 
    LEDManagerInitStatePatterns

DESCRIPTION
    Creates the state patterns space for the system states.
    
*/
static void LEDManagerInitStatePatterns ( void ) 
{
    uint16 lIndex = 0; 
 
    for ( lIndex= 0 ; lIndex < LED_TOTAL_STATES; lIndex ++ )
    {
        theHeadset.theLEDTask.gStatePatterns [ lIndex ] = NULL ;     
    }
}


/****************************************************************************
NAME 
    LEDManagerInitEventPatterns

DESCRIPTION
    Inits the Event pattern pointers.

*/
static void LEDManagerInitEventPatterns ( void )
{
    uint16 lIndex = 0; 
 
    for ( lIndex= 0 ; lIndex < EVENTS_MAX_EVENTS ; lIndex ++ )
    {
       theHeadset.theLEDTask.gEventPatterns [ lIndex ] = NULL ;     
    } 
}


/****************************************************************************
NAME 
    LEDManagerCreateFilterPatterns

DESCRIPTION
    Creates the Filter patterns space.
    
*/
static void LEDManagerCreateFilterPatterns ( void )
{
	LedTaskData * ptheLEDTask = &theHeadset.theLEDTask;
    uint16 lIndex = 0 ;
    /*create the space for the filter patterns*/
    
    for (lIndex = 0 ; lIndex < LM_NUM_FILTER_EVENTS ; lIndex++ )
    {
        ptheLEDTask->gEventFilters [ lIndex ].Event             = 0 ;
        ptheLEDTask->gEventFilters [ lIndex ].IsFilterActive    = FALSE ;      
        ptheLEDTask->gEventFilters [ lIndex ].Speed             = 0 ;
        ptheLEDTask->gEventFilters [ lIndex ].SpeedAction       = SPEED_MULTIPLY ;
        ptheLEDTask->gEventFilters [ lIndex ].Colour            = LED_COL_EITHER ;  
        ptheLEDTask->gEventFilters [ lIndex ].OverideLED        = 0 ;  
        ptheLEDTask->gEventFilters [ lIndex ].FilterToCancel    = 0 ;
        ptheLEDTask->gEventFilters [ lIndex ].OverideLEDActive  = FALSE ;
        ptheLEDTask->gEventFilters [ lIndex ].FollowerLEDActive = FALSE ;
        ptheLEDTask->gEventFilters [ lIndex ].FollowerLEDDelay  = 0 ;
        ptheLEDTask->gEventFilters [ lIndex ].OverideDisable    = FALSE ;
    }
    
    ptheLEDTask->gLMNumFiltersUsed = 0 ;

    ptheLEDTask->gTheActiveFilters = 0x0000 ;
}


/****************************************************************************
NAME 
    LMGetPattern

DESCRIPTION
    Method to get a pointer to one of the pre allocated patterns - if there are no patterns left,
    returns NULL.

RETURNS
    LedPattern_t *
    
*/
static LEDPattern_t * LMGetPattern ( void ) 
{
    LEDPattern_t * lPattern = NULL ;
	LedTaskData * ptheLEDTask = &theHeadset.theLEDTask;
    
    uint16 lIndex = 0 ;
    
        /*iterate through the patterns looking for one that is unused*/
    for (lIndex = 0 ; lIndex < LM_MAX_NUM_PATTERNS ; lIndex ++ )
    {
        if ( LMIsPatternEmpty ( &ptheLEDTask->gPatterns [ lIndex ] ) )
        {
            /*then this pattern is not used and we can use it*/
            lPattern = &ptheLEDTask->gPatterns [ lIndex ] ;
            LM_DEBUG(("LM : PatFound[%d]\n", lIndex  )) ;
                /*if we have found a free pattern then there is no need to continue looking*/
            return lPattern ;
            
        }        
    }
        /*if we could not find a pattern*/
    if (lPattern == NULL)
    {
        LM_DEBUG(("LM : Pat !\n")) ;
    }
    
    return ( lPattern ) ;
}


/****************************************************************************
NAME 
    LMIsPatternUsed

DESCRIPTION
    Helper method to determine if a pattern has been used or not - checks
    whether the pattern contains valid data.

RETURNS
    bool IsUsed
    
*/
static bool LMIsPatternEmpty (LEDPattern_t * pPattern )
{
    bool lIsUsed = FALSE ;
    
     if ( pPattern->OnTime == 0 )
     {
        if ( pPattern->OffTime == 0 )
        {
            lIsUsed = TRUE ;
        }
     }
    return lIsUsed ; 
}
 

/****************************************************************************
NAME 
    LMReleasePattern

DESCRIPTION
    Method to set apattern to a known state so that it can be used by GetPattern.

*/
static void LMResetPattern ( LEDPattern_t * pPattern )
{
    pPattern->LED_A      = 0 ;
    pPattern->LED_B      = 0 ;
    pPattern->OnTime     = 0 ;
    pPattern->OffTime    = 0 ;
    pPattern->RepeatTime = 0 ;
    pPattern->NumFlashes = 0 ;
    pPattern->DimTime    = 0;
    pPattern->TimeOut    = 0 ;
    pPattern->Colour     = LED_COL_LED_A ;  
    pPattern->OverideDisable = FALSE;
}


/****************************************************************************
NAME 
    LMAddPattern

DESCRIPTION
    Adds an LED Mapping (Event / State). 

RETURNS
    The new destination ptr if there was one allocated.
    
*/
static LEDPattern_t * LMAddPattern ( LEDPattern_t * pSourcePattern , LEDPattern_t * pDestPattern ) 
{
        /*if the pattern we have been passed is empty then we want to make sure there is no pattern present*/
    if ( LMIsPatternEmpty ( pSourcePattern )  )
    {
            /*if a pattern is already defined for this Location*/
        if ( pDestPattern ) 
        {
                /*then delete the pattern*/
            LMResetPattern (  pDestPattern ) ;            
            pDestPattern = NULL ;
        }
        /*otherwise there was no pattern defined anyway*/
    }
    else
    {
           /*if a pattern is not already defined for this state*/
        if ( ! pDestPattern ) 
        {
                /*get a pattern pointer from our block*/  
            pDestPattern = LMGetPattern() ;
        }
        
        /*if we have a pattern (wither from before or just given)*/
        if (pDestPattern)
        {
               /*populate the pattern*/ 	
			*pDestPattern = *pSourcePattern;
        
           #ifdef DEBUG_LM
           		LMPrintPattern ( pDestPattern ) ;
           #endif		
        }
        else
        {
            LM_DEBUG(("LM: NoPat!\n")) ;
        }
    }
        /*pass the new pointer back to the caller as we may have modified it*/
    return pDestPattern ;
}

#else

static const int temp ;

#endif /* ROM_LEDS */
































