/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_buttonmanager.h
@brief   Interface to the headset button manager. 
*/
#ifndef HEADSET_BUTTON_MANAGER_H
#define HEADSET_BUTTON_MANAGER_H

#include "headset_events.h"
#include "headset_private.h"


typedef struct
{
 	unsigned    event:8;
 	unsigned 	type:8;
    uint16  	pio_mask_16_to_31;
 	uint16  	pio_mask_0_to_15;
	unsigned  	hfp_state_mask:12;
	unsigned  	a2dp_state_mask:4;
}event_config_type;

typedef enum ButtonsTimeTag
{
    B_INVALID ,
    B_SHORT ,
    B_LONG  ,
    B_VERY_LONG , 
    B_DOUBLE ,
    B_REPEAT , 
    B_LOW_TO_HIGH ,
    B_HIGH_TO_LOW , 
    B_SHORT_SINGLE,
    B_LONG_RELEASE,
    B_VERY_LONG_RELEASE ,
    B_VERY_VERY_LONG ,
    B_VERY_VERY_LONG_RELEASE
}ButtonsTime_t ;

    /*usd byt the button manager*/
typedef struct ButtonEventsTag
{
    uint32        ButtonMask ;
    unsigned int HfpStateMask:12 ;  
    unsigned int A2dpStateMask:4 ;
    unsigned int Duration:4;
    unsigned int Event:12 ;
}ButtonEvents_t ;

#define BM_NUM_BUTTON_MATCH_PATTERNS 2

#define BM_NUM_BUTTONS_PER_MATCH_PATTERN 6 

#define BM_EVENTS_PER_BLOCK (25)

typedef struct ButtonPatternTag
{
    headsetEvents_t EventToSend ;
    uint16          NumberOfMatches ;
    uint32          ButtonToMatch[BM_NUM_BUTTONS_PER_MATCH_PATTERN] ;   
}ButtonMatchPattern_t ;


/* Definition of the button configuration */
typedef struct
{
    uint16 double_press_time;
    uint16 long_press_time;
    uint16 very_long_press_time; 
	uint16 repeat_time;
	uint16 very_very_long_press_time ;
  
	unsigned debounce_number:8 ;
  	unsigned debounce_period_ms:8;
	
	uint32 key_test_pios;

}button_config_type;

	/*the buttons structure - part of the main app task*/
typedef struct
{
    TaskData    task;
    
    uint32      gBOldState  ;       /*the last state we received*/    
    uint32      gBDoubleState  ;    
    uint32 		gButtonLevelMask ;  /*mask used when detecting the PIOs changed*/
    uint32 		gBOldEdgeState ;
    
    button_config_type button_config ; /*the button durations etc*/
    
	unsigned    gBDoubleTap:1  ;
	unsigned 	gBTime:8 ; /**ButtonsTime_t   */
	unsigned    unused:7;
   
    ButtonEvents_t * gButtonEvents [2] ;/*pointer to the array of button event maps*/
             
    ButtonMatchPattern_t gButtonPatterns [BM_NUM_BUTTON_MATCH_PATTERNS]; /*the button match patterns*/
    
    uint16      gButtonMatchProgress[BM_NUM_BUTTON_MATCH_PATTERNS] ;  /*the progress achieved*/
    
    uint32      gPerformEdgeCheck;      /* bit mask of pio's that are configured for edge detect */
    uint32      gPerformLevelCheck;     /* bit mask of pio's that are configured for level detect */
    uint32      gOldPioState;           /* store of previous pio state for edge/level checking */

} ButtonsTaskData;


/* Button pattern pio mask structure */
typedef struct
{
	uint16 pio_mask_16_to_31;
	uint16 pio_mask_0_to_15;
}button_pattern_type ;


/****************************************************************************
NAME 
 buttonManagerInit

DESCRIPTION
 Initialises the button event 

RETURNS
 void
    
*/
void buttonManagerInit ( void ) ;

/****************************************************************************
NAME 
 buttonManagerAddMapping

DESCRIPTION
 Maps a button event to a system event
        
    pButtonMask - 
    mask of the buttons that when pressed will generate an event
    e.g.  0x0001 = button PIO 0
    
          0x0003 = combination of PIO 0  and PIO 1
    pSystemEvent
        The Event to be signalled as define in headset_events.h
    pStateMask
        the states as defined in headset_states that the event will be generated in
    pDuration
        the Duration of the button press as defined in headset_buttons.h
        B_SHORT , B_LONG , B_VLONG, B_DOUBLE
         
*/
void buttonManagerAddMapping ( event_config_type *event_config, 
							   uint8 index ) ;

/****************************************************************************
DESCRIPTION
 Adds a button pattern to match against
          
RETURNS
 void
*/    
bool buttonManagerAddPatternMapping ( uint16 pSystemEvent , button_pattern_type * pButtonsToMatch ) ;

/****************************************************************************
NAME 
 ButtonManagerConfigDurations
    
DESCRIPTION
 Wrapper method for the button Duration Setup
    
RETURNS

    void
*/   
void buttonManagerConfigDurations ( button_config_type pButtons ) ; 

/****************************************************************************
NAME 
 BMButtonDetected

DESCRIPTION
 function call for when a button has been detected 
          
RETURNS
 void
*/    
void BMButtonDetected ( uint32 pButtonMask  , ButtonsTime_t pTime ) ;


/****************************************************************************
DESCRIPTION
 	perform an initial read of pios after configuration has been read as it is possible
    that pio states may have changed whilst the config was being read and now needs
    checking to see if any relevant events need to be generated 
*/

void BMCheckButtonsAfterReadingConfig( void );


#endif
