/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_leds.c
@brief    Module responsible for managing the LED outputs and the pios configured as led outputs.
*/

#ifdef ROM_LEDS

#include "headset_led_manager.h"
#include "headset_leds.h"
#include "headset_pio.h"
#include "headset_private.h"
#include "headset_statemanager.h"

#include <charger.h>
#include <panic.h>
#include <stddef.h>


#ifdef DEBUG_LEDS
#define LED_DEBUG(x) DEBUG(x)
#else
#define LED_DEBUG(x) 
#endif



#define LED_ON  TRUE    
#define LED_OFF FALSE

#define LEDS_STATE_START_DELAY_MS 300


/****************************************************************************
    LOCAL FUNCTION PROTOTYPES
*/

 /*internal message handler for the LED callback messages*/
static void LedsMessageHandler( Task task, MessageId id, Message message ) ;

 /*helper functions for the message handler*/
static uint16 LedsApplyFilterToTime     ( uint16 pTime )  ;
static LEDColour_t LedsGetPatternColour ( const LEDPattern_t * pPattern ) ;

 /*helper functions to change the state of LED pairs depending on the pattern being played*/
static void LedsTurnOffLEDPair ( LEDPattern_t * pPattern, bool pUseOveride  ) ;
static void LedsTurnOnLEDPair  ( LEDPattern_t * pPattern , LEDActivity_t * pLED );

    /*method to complete an event*/
static void LedsEventComplete ( LEDActivity_t * pPrimaryLed , LEDActivity_t * pSecondaryLed ) ;
    /*method to indicate that an event has been completed*/
static void LedsSendEventComplete ( headsetEvents_t pEvent , bool pPatternCompleted ) ;

    /*filter enable - check methods*/
static bool LedsIsFilterEnabled ( uint16 pFilter ) ;
static void LedsEnableFilter ( uint16 pFilter , bool pEnable) ;

static void LedsHandleOverideLED ( bool pOnOrOff ) ;

	/*Follower LED helper functions*/
static bool LedsCheckFiltersForLEDFollower( void ) ;

static uint16 LedsGetLedFollowerRepeatTimeLeft( LEDPattern_t* pPattern) ;

static uint16 LedsGetFollowerPin( void ) ;

static uint16 LedsGetLedFollowerStartDelay( void ) ;


/****************************************************************************
  FUNCTIONS
*/

/*****************************************************************************/
void LedsInit ( void ) 
{
	LedTaskData * pTask = &theHeadset.theLEDTask;
        /*Set the callback handler for the task*/
    pTask->task.handler = LedsMessageHandler ;
    
    pTask->gCurrentlyIndicatingEvent = FALSE ;
   
    pTask->gFollowing = FALSE ; 
}


/*****************************************************************************/
void LedsIndicateEvent ( headsetEvents_t pEvent ) 
{ 
    uint16 lPrimaryLED = 0;
    uint16 lSecondaryLED = 0;
	
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
    
        /*get the event and start the display*/
    lPrimaryLED     = pLEDTask->gEventPatterns[pEvent]->LED_A;
    lSecondaryLED   = pLEDTask->gEventPatterns[pEvent]->LED_B;
    
    LED_DEBUG(("LM : Ev LED [%x]i[%x] ", pEvent , pEvent )) ;
    #ifdef DEBUG_LM
    	LMPrintPattern ( pLEDTask->gEventPatterns [pEvent] ) ;
    #endif    
            /*if the PIO we want to use is currently indicating an event then do interrupt the event*/
    MessageCancelAll ( &pLEDTask->task, lPrimaryLED ) ;
    MessageCancelAll ( &pLEDTask->task, lSecondaryLED ) ;
   
        /*cancel all led state indications*/
    LedsIndicateNoState() ; 
    
        /* - need to set the LEDS to a known state before starting the pattern*/
    LedsTurnOffLEDPair ( pLEDTask->gEventPatterns[pEvent] , TRUE) ;

            /*now set up and start the event indication*/
    LedsSetLedActivity ( &pLEDTask->gActiveLEDS[lPrimaryLED] , IT_EventIndication , pEvent  , pLEDTask->gEventPatterns[pEvent]->DimTime ) ;
        /*Set the Alternative LED up with the same info*/
    LedsSetLedActivity ( &pLEDTask->gActiveLEDS[lSecondaryLED] , IT_EventIndication , pEvent , pLEDTask->gEventPatterns[pEvent]->DimTime ) ;

        
        /*if we just want to set a pin as an output (numFlashes == 0) - then dont send a message*/
    LED_DEBUG(("LM : NF[%x]\n" , pLEDTask->gEventPatterns [ pEvent ]->NumFlashes )) ;
    if ( pLEDTask->gEventPatterns [ pEvent ]->NumFlashes == 0 )
    {
            /*set the pins on or off as required*/
        if ( pLEDTask->gEventPatterns [ pEvent ]->OnTime > 0 )
        {
            LED_DEBUG(("LM : PIO_ON\n")) ;
            LedsTurnOnLEDPair ( pLEDTask->gEventPatterns [ pEvent ] , &pLEDTask->gActiveLEDS[lPrimaryLED]  ) ;
        }
        else if ( pLEDTask->gEventPatterns [ pEvent ]->OffTime > 0 )
        {
            LED_DEBUG(("LM : PIO_OFF\n")) ;
            LedsTurnOffLEDPair ( pLEDTask->gEventPatterns [ pEvent ] ,TRUE) ;
          
              /*If we are turning a pin off the revert to state indication*/
            LedsEventComplete ( &pLEDTask->gActiveLEDS[lPrimaryLED] , &pLEDTask->gActiveLEDS[lSecondaryLED] ) ;
        }   
    }
    else
    {   /*start the pattern indication*/  /*All messages are handled via the primary LED*/
	    MessageSend (&pLEDTask->task , lPrimaryLED , 0 ) ;        
        pLEDTask->gCurrentlyIndicatingEvent = TRUE ;
    }
}

   
/*****************************************************************************/
void LedsIndicateState ( headsetHfpState pState , headsetA2dpState pA2dpState ) 
{ 
    uint16 led_state = stateManagerGetCombinedLEDState(pState, pA2dpState);
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
    LEDPattern_t * lPattern = pLEDTask->gStatePatterns[led_state] ;	

    #ifdef DEBUG_LM
    	LMPrintPattern( lPattern ) ;
    #endif
        
    
    LED_DEBUG(("LED: st[%x][%x]", pLEDTask->gActiveLEDS[lPattern->LED_A].Type , pLEDTask->gActiveLEDS[lPattern->LED_B].Type)) ;

        /*only indicate state if both leds we want to use are not currently indicating an event*/
    if (    ( pLEDTask->gActiveLEDS[lPattern->LED_A].Type   != IT_EventIndication  )
         && ( pLEDTask->gActiveLEDS[lPattern->LED_B].Type != IT_EventIndication  ) )
    {
        LED_DEBUG(("LED: IS : OK\n")) ;
            /*Find the LEDS that are set to indicate states and Cancel the messages,
              -do not want to indicate more than one state at a time*/
        LedsIndicateNoState() ;
        
           /*Set up the Active LED with the State we want to indicate*/
        LedsSetLedActivity ( &pLEDTask->gActiveLEDS[lPattern->LED_A] , IT_StateIndication , led_state , lPattern->DimTime ) ;
            /*Set the Alternative LED up with the same info*/
        LedsSetLedActivity ( &pLEDTask->gActiveLEDS[lPattern->LED_B] , IT_StateIndication , led_state ,lPattern->DimTime) ;

            /* - need to set the LEDS to a known state before starting the pattern*/
        LedsTurnOffLEDPair ( lPattern , TRUE ) ;

        LED_DEBUG(("LED: st2[%x][%x]", pLEDTask->gActiveLEDS[lPattern->LED_A].Type , pLEDTask->gActiveLEDS[lPattern->LED_B].Type)) ;


            /*Handle permanent output leds*/
        if ( lPattern->NumFlashes == 0 )
        {
            /*set the pins on or off as required*/
            if ( lPattern->OnTime > 0 )
            {
                LED_DEBUG(("LM :ST PIO_ON\n")) ;
                LedsTurnOnLEDPair ( lPattern , &pLEDTask->gActiveLEDS[lPattern->LED_A]  ) ;
            }
            else if ( lPattern->OffTime > 0 )
            {
                LED_DEBUG(("LM :ST PIO_OFF\n")) ;
                LedsTurnOffLEDPair ( lPattern ,TRUE) ;
            }   
        }
        else
            /*send the first message for this state LED indication*/ 
        MessageSendLater (&pLEDTask->task , lPattern->LED_A , 0 , LEDS_STATE_START_DELAY_MS ) ;
    }
}


/*****************************************************************************/
void LedsIndicateNoState ( void )  
{
     /*Find the LEDS that are set to indicate states and Cancel the messages,
    -do not want to indicate more than one state at a time*/
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
    uint16 lLoop = 0;
    
    for ( lLoop = 0 ; lLoop < HEADSET_NUM_LEDS ; lLoop ++ )
    {
        if (pLEDTask->gActiveLEDS[lLoop].Type == IT_StateIndication)
        {
            MessageCancelAll ( &pLEDTask->task, lLoop ) ; 
            pLEDTask->gActiveLEDS[lLoop].Type =  IT_Undefined ;
            
            LED_DEBUG(("LED: CancelStateInd[%x]\n" , lLoop)) ;
            LedsTurnOffLEDPair ( pLEDTask->gStatePatterns[pLEDTask->gActiveLEDS[lLoop].Index] ,TRUE) ;
        }
    }
}


/*****************************************************************************/
bool LedsStateCanOverideDisable ( headsetHfpState pState , headsetA2dpState pA2dpState ) 
{
    uint16 led_state = stateManagerGetCombinedLEDState(pState, pA2dpState);
    LEDPattern_t * lPattern = theHeadset.theLEDTask.gStatePatterns[led_state] ;
    if (lPattern != NULL)
        return lPattern->OverideDisable;
    else
        return FALSE;
}


/*****************************************************************************/
bool LedsEventCanOverideDisable ( MessageId pEvent ) 
{
    LEDPattern_t * lPattern = theHeadset.theLEDTask.gEventPatterns[pEvent] ;
    if (lPattern != NULL)
    {
        return lPattern->OverideDisable;
    }
    else
        return FALSE;
}


/*****************************************************************************/
bool LedActiveFiltersCanOverideDisable( void )
{
    uint16 lFilterIndex ;
    
    for (lFilterIndex = 0 ; lFilterIndex< LM_NUM_FILTER_EVENTS ; lFilterIndex++ )
    {
        if ( LedsIsFilterEnabled(lFilterIndex) )
        {
            /* check if this filter overides LED disable flag */
            if ( theHeadset.theLEDTask.gEventFilters[lFilterIndex].OverideDisable)
                return TRUE;
        }
    }
    return FALSE;
}


/*****************************************************************************/       
void LedsCheckForFilter ( headsetEvents_t pEvent ) 
{
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
	
    uint16 lFilterIndex = 0 ;
    
    for (lFilterIndex = 0 ; lFilterIndex < pLEDTask->gLMNumFiltersUsed ; lFilterIndex ++ )
    { 
        if ( pLEDTask->gEventFilters[ lFilterIndex ].Event == pEvent )
        {
            if (pLEDTask->gEventFilters[ lFilterIndex ].IsFilterActive)
            {
                /* Check filter isn't already enabled */
                if (!LedsIsFilterEnabled(lFilterIndex))
                {
                    /* Enable filter */
                    LedsEnableFilter (lFilterIndex , TRUE) ;

                    /* If it is an overide fLED filter and the currently playing pattern is OFF then turn on the overide led immediately*/
                    if ( pLEDTask->gEventFilters [ lFilterIndex].OverideLEDActive )
                    {
                        uint16 lOverideLEDIndex = pLEDTask->gEventFilters[ lFilterIndex].OverideLED ;                    
                        /* this should only happen if the led in question is currently off*/
                        if ( pLEDTask->gActiveLEDS[lOverideLEDIndex].OnOrOff == LED_OFF)
                        {
                             PioSetLedPin ( pLEDTask , lOverideLEDIndex , LED_ON) ;
                        }
                    }
                }
            }
            else
            {
                 uint16 lFilterToCancel = pLEDTask->gEventFilters[ lFilterIndex ].FilterToCancel ;
                /*disable the according filter*/
                 if ( lFilterToCancel != 0 )
                 {
                     uint16 lFilterToCancelIndex = lFilterToCancel - 1 ;
                     uint16 lOverideLEDIndex = pLEDTask->gEventFilters[ lFilterToCancelIndex ].OverideLED ;
                    
                     LED_DEBUG(("LED: FilCancel[%d][%d] [%d]\n",lFilterIndex + 1 , lFilterToCancel , lFilterToCancelIndex )) ;
                     
                        /*lFilter To cancel = 1-n, LedsEbnable filter requires 0-n */
                     LedsEnableFilter (lFilterToCancelIndex , FALSE ) ;
                     
                     if ( pLEDTask->gActiveLEDS[lOverideLEDIndex].OnOrOff == LED_OFF)
                     {  
                         if ( pLEDTask->gEventFilters [ lFilterToCancelIndex].OverideLEDActive )
                         {
	          	             PioSetLedPin ( pLEDTask , pLEDTask->gEventFilters[lFilterToCancelIndex].OverideLED , LED_OFF) ;                
                         }    
                     }                           
                 }
                 else
                 {
                    LED_DEBUG(("LED: Fil !\n")) ;
                 }
            }
            LED_DEBUG(("LM : Filter Found[%d]A[%x] [%d]\n", lFilterIndex ,  pEvent , pLEDTask->gEventFilters[ lFilterIndex ].IsFilterActive )) ;
       }
    }
}


/*****************************************************************************/
void LedsSetLedActivity ( LEDActivity_t * pLed , IndicationType_t pType , uint16 pIndex , uint16 pDimTime)
{
    pLed->NumFlashesComplete = 0 ;
    pLed->Type               = pType ;
    pLed->Index              = pIndex ;
    pLed->FilterIndex        = 0 ;
    pLed->OnOrOff            = FALSE ;
    pLed->NumRepeatsComplete = 0 ;
    
    pLed->DimState  = 0 ;  
    pLed->DimDir    = 0 ;  
    pLed->DimTime = pDimTime ;   
    LED_DEBUG(("LED[%d]\n" , pDimTime)) ; 
  
}


/*****************************************************************************/
void LedsResetAllLeds ( void ) 
{   
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
	
		/*Cancel all event indications*/ 
    uint16 lLoop = 0;
    
    for ( lLoop = 0 ; lLoop < HEADSET_NUM_LEDS ; lLoop ++ )
    {
        if (pLEDTask->gActiveLEDS[lLoop].Type == IT_EventIndication)
        {
            MessageCancelAll ( &pLEDTask->task, lLoop ) ; 
            pLEDTask->gActiveLEDS[lLoop].Type =  IT_Undefined ;
            
            LED_DEBUG(("LED: CancelEventInd[%x]\n" , lLoop)) ;
            LedsTurnOffLEDPair ( pLEDTask->gEventPatterns[pLEDTask->gActiveLEDS[lLoop].Index] ,TRUE) ;
        }
    }
    	/*cancel all state indications*/
    LedsIndicateNoState()  ;   
}


/****************************************************************************
NAME 
    LEDManagerMessageHandler

DESCRIPTION
    The main message handler for the LED task. Controls the PIO in question, then 
    queues a message back to itself for the next LED update.

*/
static void LedsMessageHandler( Task task, MessageId id, Message message )
{  
    LedTaskData * lLEDTask = (LedTaskData *) task ;
    bool lOldState = LED_OFF ;
    uint16 lTime   = 0 ;
    LEDColour_t lColour ;    
    
    LEDActivity_t * lLED   = &lLEDTask->gActiveLEDS[id] ;
    LEDPattern_t *  lPattern = NULL ;
    bool lPatternComplete = FALSE ;
    
    if (id < DIM_MSG_BASE )
    {
        
            /*which pattern are we currently indicating for this LED pair*/
        if ( lLED->Type == IT_StateIndication)
        {
           lPattern = lLEDTask->gStatePatterns[ lLED->Index] ;
        }
        else
        {      /*is an event indication*/
            lPattern = lLEDTask->gEventPatterns [ lLED->Index ] ;
        }
            /*get which of the LEDs we are interested in for the pattern we are dealing with*/
        lColour = LedsGetPatternColour ( lPattern ) ;
         
            /*get the state of the LED we are dealing with*/
        lOldState = lLEDTask->gActiveLEDS [ lPattern->LED_A ].OnOrOff ;
     
        LED_DEBUG(("LM : LED[%d] [%d] f[%d]of[%d]\n", id ,lOldState , lLED->NumFlashesComplete , lPattern->NumFlashes )) ;
        
             
            /*The actual LED handling*/
        if (lOldState == LED_OFF)
        {
            lTime = lPattern->OnTime ;
               /*Increment the number of flashes*/
            lLED->NumFlashesComplete++ ;
                  
            LED_DEBUG(("LED: Pair On\n")) ;
            LedsTurnOnLEDPair ( lPattern , lLED ) ;
            
        }
        else
        {    /*restart the pattern if we have palayed all of the required flashes*/
            if ( lLED->NumFlashesComplete >= lPattern->NumFlashes )
            {
                lTime = lPattern->RepeatTime ;
                lLED->NumFlashesComplete = 0 ;       
                    /*inc the Num times the pattern has been played*/
                lLED->NumRepeatsComplete ++ ;
                LED_DEBUG(("LED: Pat Rpt [%d][%d]\n",lLED->NumRepeatsComplete , lPattern->TimeOut)) ;
          
                /*if a single pattern has completed*/
                if ( lPattern->RepeatTime == 0 ) 
                {
                    LED_DEBUG(("LED: PC: Rpt\n")) ;
                    lPatternComplete = TRUE ;
                }
                   /*a pattern timeout has occured*/
                if ( ( lPattern->TimeOut !=0 )  && ( lLED->NumRepeatsComplete >= lPattern->TimeOut) )
                {
                    lPatternComplete = TRUE ;
                    LED_DEBUG(("LED: PC: Rpt b\n")) ;
                }              
                
                /*if we have reached the end of the pattern and are using a follower then revert to the orig pattern*/
                if (lLEDTask->gFollowing)
                {
                    lLEDTask->gFollowing = FALSE ;
                    lTime = LedsGetLedFollowerRepeatTimeLeft( lPattern ) ;    
                }
                else
                {
                    /*do we have a led follower filter and are we indicating a state, if so use these parameters*/
                    if (lLED->Type == IT_StateIndication)
                    {
                        if( LedsCheckFiltersForLEDFollower() )
                        {
                            lTime = LedsGetLedFollowerStartDelay() ;       
                            lLEDTask->gFollowing = TRUE ;
                        }
                    }    
                 }            
            } 
            else /*otherwise set up for the next flash*/
            {
                lTime = lPattern->OffTime ;
            } 
                /*turn off both LEDS*/
         
            LED_DEBUG(("LED: Pair OFF\n")) ;   
            
            if ( (lTime == 0 ) && ( lPatternComplete == FALSE ) )
            {
                    /*ie we are switching off for 0 time - do not use the overide led as this results in a tiny blip*/
                LedsTurnOffLEDPair ( lPattern , FALSE) ;
            }
            else
            {
                LedsTurnOffLEDPair ( lPattern , TRUE) ;
            }
        }
        
       
            /*handle the completion of the pattern or send the next update message*/
        if (lPatternComplete)
        {
            LED_DEBUG(("LM : P C [%x][%x]  [%x][%x]\n" , lLEDTask->gActiveLEDS[lPattern->LED_B].Index, lLED->Index , lLEDTask->gActiveLEDS[lPattern->LED_B].Type , lLED->Type    )) ;
            /*set the type of indication for both LEDs as undefined as we are now indicating nothing*/
            if ( lLEDTask->gActiveLEDS[id].Type == IT_EventIndication )
            {
                      /*signal the completion of an event*/
                LedsSendEventComplete ( lLED->Index, TRUE ) ;
                    /*now complete the event, and indicate a new state if required*/        
                LedsEventComplete ( lLED , &lLEDTask->gActiveLEDS[lPattern->LED_B] ) ;
            }  
            else if (lLEDTask->gActiveLEDS[id].Type == IT_StateIndication )
            {
                /*then we have completed a state indication and the led pattern is now off*/    
                /*Indicate that we are now with LEDS disabled*/
               lLEDTask->gLEDSStateTimeout = TRUE ;
            }
        }
        else
        {       /*apply the filter in there is one  and schedule the next message to handle for this led pair*/
            lTime = LedsApplyFilterToTime ( lTime ) ;
            MessageSendLater (&lLEDTask->task , id , 0 , lTime ) ;
        } 
        
    }
    else
    {
        /*DIMMING LED Update message */       
        PioSetDimState ( lLEDTask , (id - DIM_MSG_BASE) );
    }
}


/****************************************************************************
NAME 
    LMTurnOnLEDPair

DESCRIPTION
    Fn to turn on the LED associated with the pattern / LEDs depending upon the 
    colour.
    
*/
static void LedsTurnOnLEDPair ( LEDPattern_t * pPattern , LEDActivity_t * pLED )
{
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
    LEDColour_t lColour = LedsGetPatternColour ( pPattern ) ; 
    
    if (pLEDTask->gFollowing )
    {	 /*turn of the pair of leds (and dont use an overide LED */
        LedsTurnOffLEDPair ( pPattern , FALSE) ;
        PioSetLedPin ( pLEDTask , LedsGetFollowerPin(), LED_ON );
    }
    else
    {/*we are not following*/
        switch (lColour )
        {
        case LED_COL_LED_A:
    
            LED_DEBUG(("LED: A ON[%x][%x]\n", pPattern->LED_A , pPattern->LED_B)) ;            
            if (pPattern->LED_A != pPattern->LED_B)
            {
                PioSetLedPin ( pLEDTask , pPattern->LED_B , LED_OFF )  ;
            }
            PioSetLedPin ( pLEDTask,  pPattern->LED_A , LED_ON )  ;
        
        break;
        case LED_COL_LED_B:
    
            LED_DEBUG(("LED: B ON[%x][%x]\n", pPattern->LED_A , pPattern->LED_B)) ;
            if (pPattern->LED_A != pPattern->LED_B)
            {
                PioSetLedPin ( pLEDTask , pPattern->LED_A , LED_OFF )  ;
            }
            PioSetLedPin ( pLEDTask , pPattern->LED_B , LED_ON )  ;
            
        break;
        case LED_COL_LED_ALT:
            if (pLED->NumFlashesComplete % 2 )
            {
        
                LED_DEBUG(("LED: A ALT[%x][%x]\n", pPattern->LED_A , pPattern->LED_B)) ;
                PioSetLedPin ( pLEDTask , pPattern->LED_A , LED_OFF )  ;
            	PioSetLedPin ( pLEDTask , pPattern->LED_B , LED_ON )  ;
            }
            else
            {
        
                LED_DEBUG(("LED: B ALT[%x][%x]\n", pPattern->LED_A , pPattern->LED_B)) ;
                PioSetLedPin ( pLEDTask , pPattern->LED_B , LED_OFF )  ;
                PioSetLedPin ( pLEDTask , pPattern->LED_A , LED_ON )  ;                
            }
        break;
        case LED_COL_LED_BOTH:
    
            LED_DEBUG(("LED: AB Both[%x][%x]\n", pPattern->LED_A , pPattern->LED_B)) ;
            PioSetLedPin ( pLEDTask , pPattern->LED_A , LED_ON )  ;
            PioSetLedPin ( pLEDTask , pPattern->LED_B , LED_ON )  ;
        break;
        default:
            LED_DEBUG(("LM : ?Col\n")) ;
        break;
        }
    }
    
        /*handle an overide LED if there is one will also dealit is different to one of the pattern LEDS)*/
    LedsHandleOverideLED ( LED_OFF ) ;
    
    pLED->OnOrOff = TRUE ;
        
}


/****************************************************************************
NAME 
    LMTurnOffLEDPair

DESCRIPTION
    Fn to turn OFF the LEDs associated with the pattern.
    
*/
static void LedsTurnOffLEDPair ( LEDPattern_t * pPattern  , bool pUseOveride ) 
{
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
        /*turn off both LEDS*/
    PioSetLedPin ( pLEDTask , pPattern->LED_A , LED_OFF )  ;
    PioSetLedPin ( pLEDTask , pPattern->LED_B , LED_OFF )  ;
        /*handle an overide LED if we want to use one*/
    if ( pUseOveride )
    {
        LedsHandleOverideLED ( LED_ON ) ;
    }
    pLEDTask->gActiveLEDS [ pPattern->LED_A ].OnOrOff  = FALSE ;        
}


/****************************************************************************
NAME 
    LedsHandleOverideLED

DESCRIPTION
    Enables / diables any overide LEDS if there are some.

*/
static void LedsHandleOverideLED ( bool pOnOrOff ) 
{   
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
    uint16 lFilterIndex = 0 ;
    
    for (lFilterIndex = 0 ; lFilterIndex< LM_NUM_FILTER_EVENTS ; lFilterIndex++ )
    {
        if ( LedsIsFilterEnabled(lFilterIndex) )
        {
             if ( pLEDTask->gEventFilters[lFilterIndex].OverideLEDActive )
             {
                if ( pLEDTask->gEventFilters[lFilterIndex].OverideLED )
                {
                    /*Overide the Off LED with the Overide LED*/
                    LED_DEBUG(("LM: LEDOveride [%d] [%d]\n" , pLEDTask->gEventFilters[lFilterIndex].OverideLED , pOnOrOff)) ;    
                    PioSetLedPin ( pLEDTask , pLEDTask->gEventFilters[lFilterIndex].OverideLED , pOnOrOff) ;                                         
                }
            }    
        }
    }  
}


/****************************************************************************
NAME 
    LMGetPatternColour

DESCRIPTION
    Fn to determine the LEDColour_t of the LED pair we are currently playing
    takes into account whether or not a filter is currently active.
    
RETURNS
    LEDColour_t
*/
static LEDColour_t LedsGetPatternColour ( const  LEDPattern_t * pPattern )
{
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
    uint16 lFilterIndex = 0 ;
        /*sort out the colour of the LED we are interested in*/
    LEDColour_t lColour = pPattern->Colour ;
   
    for (lFilterIndex = 0 ; lFilterIndex< LM_NUM_FILTER_EVENTS ; lFilterIndex++ )
    {
        if ( LedsIsFilterEnabled(lFilterIndex) )
        {
            if ( pLEDTask->gEventFilters[lFilterIndex].Colour != LED_COL_EITHER )
            {
                    /*Overide the Off LED with the Overide LED*/
                lColour = pLEDTask->gEventFilters[lFilterIndex].Colour;   
            } 
        }
    }
    return lColour ;
}


/****************************************************************************
NAME 
    LMApplyFilterToTime

DESCRIPTION
    Fn to change the callback time if a filter has been applied - if no filter is applied
    just returns the original time.
    
RETURNS
    uint16 the callback time
*/
static uint16 LedsApplyFilterToTime ( uint16 pTime ) 
{
    uint16 lFilterIndex = 0 ;
    uint16 lTime = pTime ; 
	
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
    
    for (lFilterIndex = 0 ; lFilterIndex< LM_NUM_FILTER_EVENTS ; lFilterIndex++ )
    {
        if ( LedsIsFilterEnabled(lFilterIndex) )
        {
            if ( pLEDTask->gEventFilters[lFilterIndex].Speed )
            {
                if (pLEDTask->gEventFilters[lFilterIndex].SpeedAction == SPEED_MULTIPLY)
                {
                    LED_DEBUG(("LED: FIL_MULT[%d]\n" , pLEDTask->gEventFilters[lFilterIndex].Speed )) ;
                    lTime *= pLEDTask->gEventFilters[lFilterIndex].Speed ;
                }
                else /*we want to divide*/
                {
                    if (lTime)
                    {
                       LED_DEBUG(("LED: FIL_DIV[%d]\n" , pLEDTask->gEventFilters[lFilterIndex].Speed )) ;
                      lTime /= pLEDTask->gEventFilters[lFilterIndex].Speed ;
                    }
                }
            }
        }
    }

    return lTime ;
}


/****************************************************************************
NAME 
    LEDManagerSendEventComplete

DESCRIPTION
    Sends a message to the main task thread to say that an event indication has been completed.
    
*/
void LedsSendEventComplete ( headsetEvents_t pEvent , bool pPatternCompleted )
{
    LMEndMessage_t * lEventMessage = PanicUnlessNew ( LMEndMessage_t ) ;
    
        /*check that the event is in the expected range*/
    if ( (pEvent > EVENTS_EVENT_BASE) && (pEvent < EVENTS_LAST_EVENT ) )
    {   
            /*need to add the message containing the EventType here*/
        lEventMessage->Event = pEvent  ;
        lEventMessage->PatternCompleted =  pPatternCompleted ;
                        
        LED_DEBUG(("LM : lEvCmp[%x] [%x]\n",lEventMessage->Event , lEventMessage->PatternCompleted )) ;
            
        MessageSend ( &theHeadset.task , EventLEDEventComplete , lEventMessage ) ;
    }
}


/****************************************************************************
NAME 
    LedsEventComplete

DESCRIPTION
    Signal that a given event indicatio has completed.

*/
static void LedsEventComplete ( LEDActivity_t * pPrimaryLed , LEDActivity_t * pSecondaryLed ) 
{       
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
	
    pPrimaryLed->Type = IT_Undefined ;
    
    pSecondaryLed->Type = IT_Undefined ;
    
    pLEDTask->gCurrentlyIndicatingEvent = FALSE ;

    /* restart state indication */
    LEDManagerIndicateState ( stateManagerGetHfpState () , stateManagerGetA2dpState () ) ; 
    
}     


/****************************************************************************
NAME 
    LedsEnableFilter

DESCRIPTION
    Enable / disable a given filter ID

*/
static void LedsEnableFilter ( uint16 pFilter , bool pEnable)
{
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
    uint16 lOldMask = pLEDTask->gTheActiveFilters ;
    
    if (pEnable)
    {
        /*to set*/
        pLEDTask->gTheActiveFilters |= (  0x1 << pFilter ) ;
        LED_DEBUG(("LED: EnF [%x] [%x] [%x]\n", lOldMask , pLEDTask->gTheActiveFilters , pFilter))    ;
    }
    else
    {
        /*to unset*/
        pLEDTask->gTheActiveFilters &= (0xFFFF - (  0x1 << pFilter ) ) ;
        LED_DEBUG(("LED: DisF [%x] [%x] [%x]\n", lOldMask , pLEDTask->gTheActiveFilters , pFilter))    ;
    }
    
    /* Check if we should indicate state */
    if ((pLEDTask->gEventFilters[pFilter].OverideDisable) && (lOldMask != pLEDTask->gTheActiveFilters))
        LEDManagerIndicateState ( stateManagerGetHfpState () , stateManagerGetA2dpState () ) ;                          
}

/****************************************************************************
NAME 
    LedsIsFilterEnabled

DESCRIPTION
    Determine if a filter is enabled.
    
RETURNS
 	bool - enabled or not
*/
static bool LedsIsFilterEnabled ( uint16 pFilter )
{
    bool lResult = FALSE ;
    
    if ( theHeadset.theLEDTask.gTheActiveFilters & (0x1 << pFilter ) )
    {
        lResult = TRUE ;
    }
    
    return lResult ;
}





/****************************************************************************
NAME 
	LedsCheckFiltersForLEDFollower
DESCRIPTION
    Determine if a follower is currently active.
RETURNS
 	bool - active or not
*/
static bool LedsCheckFiltersForLEDFollower( void )
{
    uint16 lResult = FALSE ;    
    uint16 lFilterIndex = 0 ;
    
    for (lFilterIndex = 0 ; lFilterIndex< LM_NUM_FILTER_EVENTS ; lFilterIndex++ )
    {
        if ( LedsIsFilterEnabled(lFilterIndex) )
        {
                /*if this filter defines a lefd follower*/
            if ( theHeadset.theLEDTask.gEventFilters[lFilterIndex].FollowerLEDActive)
            {
                lResult = TRUE ;
            }    
        }
    }
    return lResult ;
}


/****************************************************************************
NAME 
	LedsGetLedFollowerRepeatTimeLeft
DESCRIPTION
    Calculate the new repeat time based upon the follower led delay and the normal repeat time.
RETURNS
 	uint16 - updated repeat time to use
*/
static uint16 LedsGetLedFollowerRepeatTimeLeft( LEDPattern_t * pPattern) 
{
    uint16 lTime = pPattern->RepeatTime ;
    uint16 lPatternTime = ( ( pPattern->OnTime  *  pPattern->NumFlashes) + 
                            ( pPattern->OffTime * (pPattern->NumFlashes - 1 ) )   +
                            ( LedsGetLedFollowerStartDelay() ) ) ;
                            
    if(lPatternTime < pPattern->RepeatTime )
    {
        lTime = pPattern->RepeatTime - lPatternTime ;
        LED_DEBUG(("LED: FOllower Rpt [%d] = [%d] - [%d]\n " , lTime , pPattern->RepeatTime , lPatternTime)) ;
    }
    
    return lTime ;        
}


/****************************************************************************
NAME 
	LedsGetLedFollowerStartDelay
DESCRIPTION
    Get the delay associated with a follower led pin.
RETURNS
 	uint16 - delay to use for the follower
*/             
static uint16 LedsGetLedFollowerStartDelay( void )
{
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
    uint16 lDelay = 0 ;
    uint16 lFilterIndex =0 ;    
    
    for (lFilterIndex = 0 ; lFilterIndex< LM_NUM_FILTER_EVENTS ; lFilterIndex++ )
    {
        if ( LedsIsFilterEnabled(lFilterIndex) )
        {
                /*if this filter defines a lefd follower*/
            if ( pLEDTask->gEventFilters[lFilterIndex].FollowerLEDActive)
            {		 /*the led to use to follow with*/
                LED_DEBUG(("LM: LEDFollower Led[%d] Delay[%d]\n" , pLEDTask->gEventFilters[lFilterIndex].OverideLED ,
                                                                   pLEDTask->gEventFilters[lFilterIndex].FollowerLEDDelay)) ;    
                lDelay = pLEDTask->gEventFilters[lFilterIndex].FollowerLEDDelay * 50 ;
            }    
        }
    }

    return lDelay ;
}


/****************************************************************************
NAME 
	LedsGetFollowerPin
DESCRIPTION
    Determine which follower pin is set - 0 = none set.
RETURNS
 	uint16 - which pin to use as a follower
*/
static uint16 LedsGetFollowerPin( void )
{
	LedTaskData * pLEDTask = &theHeadset.theLEDTask;
    uint16 lLED = 0 ;
    uint16 lFilterIndex =0 ;    
    
    for (lFilterIndex = 0 ; lFilterIndex< LM_NUM_FILTER_EVENTS ; lFilterIndex++ )
    {
        if ( LedsIsFilterEnabled(lFilterIndex) )
        {
                /*if this filter defines a lefd follower*/
            if ( pLEDTask->gEventFilters[lFilterIndex].FollowerLEDActive)
            {
                    /*the led to use to follow with*/
                 lLED = pLEDTask->gEventFilters[lFilterIndex].OverideLED ;                
            }    
        }
    }       

    return lLED ;
}


#else
	static const int dummy;
#endif /* ROM_LEDS */
