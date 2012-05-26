/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_tones.c
@brief    Module responsible for tone generation and playback.
*/

#include "headset_amp.h"
#include "headset_debug.h"
#include "headset_tones.h"

#include <audio.h>
#include <stream.h>
#include <panic.h>


#ifdef DEBUG_TONES
    #define TONE_DEBUG(x) DEBUG(x)
#else
    #define TONE_DEBUG(x) 
#endif


#define TONE_TYPE_RING (0xFF)


/****************************************************************/
/*
    SIMPLE TONES
 */
/****************************************************************/

/* power tone: */
static const audio_note tone_1[] =
{
    AUDIO_TEMPO(120), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, HEMIDEMISEMIQUAVER),        
    AUDIO_NOTE(G7,   CROTCHET), 
    
    AUDIO_END
};

/* pairing tone: */
static const audio_note tone_2[] =
{
    AUDIO_TEMPO(2400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(G5 , SEMIBREVE),
    AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(REST, QUAVER),
    AUDIO_NOTE(G5 , SEMIBREVE),
    
    AUDIO_END
};

/* mute off: */
static const audio_note tone_3[] =
{
    AUDIO_TEMPO(2400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(G6 , SEMIBREVE),
    AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(C7 , SEMIBREVE),
    
    AUDIO_END
};

/* mute on: */
static const audio_note tone_4[] =
{
    AUDIO_TEMPO(2400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(G6 , SEMIBREVE),
    AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(G5 , SEMIBREVE),
    
    AUDIO_END
};

/* battery low: */
static const audio_note tone_5[] =
{
    AUDIO_TEMPO(120), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, HEMIDEMISEMIQUAVER),
    AUDIO_NOTE(G6 , CROTCHET),
    AUDIO_NOTE(REST, HEMIDEMISEMIQUAVER),
    AUDIO_NOTE(G6 , CROTCHET),
    AUDIO_NOTE(REST, HEMIDEMISEMIQUAVER),
    AUDIO_NOTE(G6 , CROTCHET),
    
    AUDIO_END
};

/* vol limit: */
static const audio_note tone_6[] =
{
    AUDIO_TEMPO(600), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    AUDIO_NOTE(REST, SEMIQUAVER),
    AUDIO_TEMPO(200), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    AUDIO_NOTE(G7 , CROTCHET),
    
    AUDIO_END
};

/* connection: Mute 25ms, 1600Hz 100ms */
static const audio_note tone_7[] =
{
    AUDIO_TEMPO(2400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(GS7 , SEMIBREVE),
    
    AUDIO_END
};

/* error tone: Mute 25ms, 400Hz 500ms */
static const audio_note tone_8[] =
{
    AUDIO_TEMPO(120), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, HEMIDEMISEMIQUAVER),
    AUDIO_NOTE(G5 , CROTCHET),
    
    AUDIO_END
};

/* short confirmation: Mute 25ms, 800Hz 100ms */
static const audio_note tone_9[] =
{
    AUDIO_TEMPO(2400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(G6 , SEMIBREVE),
    
    AUDIO_END
};

/* long confirmation: Mute 25ms, 800Hz 800ms */
static const audio_note tone_a[] =
{
    AUDIO_TEMPO(150), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, HEMIDEMISEMIQUAVER),
    AUDIO_NOTE(G6 , MINIM),
    
    AUDIO_END
};

/* mute reminder: */
static const audio_note tone_b[] =
{
    AUDIO_TEMPO(600), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    AUDIO_NOTE(REST, SEMIQUAVER),
    AUDIO_TEMPO(120), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    AUDIO_NOTE(G5, CROTCHET),
    AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(G5, CROTCHET),
    AUDIO_END
};

/* ringtone 1 */
static const audio_note ring_twilight[] =
{
    AUDIO_TEMPO(180), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(E7, QUAVER),
    AUDIO_NOTE(F7, QUAVER),
    AUDIO_NOTE(E7, QUAVER),
    AUDIO_NOTE(C7, QUAVER),
    AUDIO_NOTE(E7, QUAVER),
    AUDIO_NOTE(F7, QUAVER),
    AUDIO_NOTE(E7, QUAVER),
    AUDIO_NOTE(C7, QUAVER),

    AUDIO_END
};

/* ringtone 2 */
static const audio_note ring_greensleeves[] =
{
    AUDIO_TEMPO(400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
              
    AUDIO_NOTE(F6,CROTCHET),                                  
    AUDIO_NOTE(AF6,MINIM),                                            
    AUDIO_NOTE(BF6,CROTCHET),                         
    AUDIO_NOTE(C7,CROTCHET),                          
    AUDIO_NOTE_TIE(C7,QUAVER),                                            
    AUDIO_NOTE(DF7,QUAVER),                           
    AUDIO_NOTE(C7,CROTCHET),                                          
    AUDIO_NOTE(BF6,MINIM),                            
    AUDIO_NOTE(G6,CROTCHET),          
    AUDIO_NOTE(EF6,CROTCHET), 
    AUDIO_NOTE_TIE(EF6,QUAVER),
       
    AUDIO_END
};

/* ringtone 3 */
static const audio_note ring_major_scale[] =
{
    AUDIO_TEMPO(300), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),

    AUDIO_NOTE(E6,QUAVER),                                    
    AUDIO_NOTE(FS6,QUAVER),                                           
    AUDIO_NOTE(GS6,QUAVER),                           
    AUDIO_NOTE(A6,QUAVER),                            
    AUDIO_NOTE(B6,QUAVER),                                            
    AUDIO_NOTE(CS7,QUAVER),                           
    AUDIO_NOTE(DS7,QUAVER),                                           
    AUDIO_NOTE(E7,QUAVER),    

    AUDIO_END
};

/* disconnect tone: Mute 25ms, 600Hz 300ms */	
static const audio_note tone_f[] =
{
    AUDIO_TEMPO(400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
    AUDIO_NOTE(D6  , MINIM),
    
    AUDIO_END
};

/* power on: Mute 25ms, 400Hz 100ms, Mute 25ms, 600Hz 100ms, Mute 25ms, 800Hz 100ms, Mute 25ms, 1000Hz 300ms */
static const audio_note tone_10[] =
{
    AUDIO_TEMPO(400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
    AUDIO_NOTE(G5  , CROTCHET_TRIPLET),
	AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
	AUDIO_NOTE(D6  , CROTCHET_TRIPLET),
	AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
	AUDIO_NOTE(G6  , CROTCHET_TRIPLET),
	AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
	AUDIO_NOTE(B6  , MINIM),
    
    AUDIO_END
};

/* power off: Mute 25ms, 1000Hz 100ms, Mute 25ms, 800Hz 100ms, Mute 25ms, 600Hz 100ms, Mute 25ms, 400Hz 300ms */ 		
static const audio_note tone_11[] =
{
    AUDIO_TEMPO(400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
    AUDIO_NOTE(B6  , CROTCHET_TRIPLET),
	AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
	AUDIO_NOTE(G6  , CROTCHET_TRIPLET),
	AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
	AUDIO_NOTE(D6  , CROTCHET_TRIPLET),
	AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
	AUDIO_NOTE(G5  , MINIM),
    
    AUDIO_END
};

/* vol min: Mute 25ms, 400Hz 300ms */
static const audio_note tone_12[] =
{
    AUDIO_TEMPO(200), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(G5,   CROTCHET), 
    
    AUDIO_END
};

/* vol max: Mute 25ms, 1600Hz 300ms */
static const audio_note tone_13[] =
{
    AUDIO_TEMPO(200), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(GS7, CROTCHET), 
        
    AUDIO_END
};

/* battery low: Mute 25ms, 800Hz 300ms, Mute 25ms, 600Hz 300ms, Mute 25ms, 400Hz 300ms */	
static const audio_note tone_14[] =
{
    AUDIO_TEMPO(400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
    AUDIO_NOTE(G6  , MINIM),
	AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
	AUDIO_NOTE(D6  , MINIM),
	AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
	AUDIO_NOTE(G5  , MINIM),
	
    
    AUDIO_END
};

/* battery empty: Mute 25ms, 600Hz 300ms, Mute 25ms, 400Hz 500ms */
static const audio_note tone_15[] =
{
    AUDIO_TEMPO(200), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST  , DEMISEMIQUAVER_TRIPLET),
    AUDIO_NOTE(D6  , CROTCHET),
	AUDIO_NOTE(REST  , DEMISEMIQUAVER_TRIPLET),
	AUDIO_NOTE(G5  , MINIM_TRIPLET),
	AUDIO_NOTE_TIE(G5  , QUAVER_TRIPLET),
    
    AUDIO_END
};

/* pairing reset: Mute 25ms, 400Hz 500ms, Mute 25ms, 400Hz 500ms */
static const audio_note tone_16[] =
{
    AUDIO_TEMPO(480), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
    AUDIO_NOTE(G5  , SEMIBREVE),
	AUDIO_NOTE(REST  , QUAVER_TRIPLET),
	AUDIO_NOTE(G5  , SEMIBREVE),
    
    AUDIO_END
};

/* mute on: Mute 25ms, 600Hz 100ms, Mute 25ms, 400Hz 100ms */	
static const audio_note tone_17[] =
{
    AUDIO_TEMPO(2400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST  , CROTCHET),
    AUDIO_NOTE(D6  , SEMIBREVE),
	AUDIO_NOTE(REST  , CROTCHET),
	AUDIO_NOTE(G5  , SEMIBREVE),
    
    AUDIO_END
};

/* mute off : Mute 25ms, 400Hz 100ms, Mute 25ms, 600Hz 100ms */
static const audio_note tone_18[] =
{
    AUDIO_TEMPO(2400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST  , CROTCHET),
    AUDIO_NOTE(G5  , SEMIBREVE),
	AUDIO_NOTE(REST  , CROTCHET),
	AUDIO_NOTE(D6  , SEMIBREVE),
    
    AUDIO_END
};

/* double press: Mute 25ms, 800Hz 300ms, Mute 100ms, 800Hz 300ms */	
static const audio_note tone_19[] =
{
    AUDIO_TEMPO(800), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(G6 , SEMIBREVE),  
    AUDIO_NOTE(REST, MINIM_TRIPLET),
    AUDIO_NOTE(G6 , SEMIBREVE),
    
    AUDIO_END
};

static const audio_note CustomRingTone[] =
{
    /* A sample custom ring tone */
    AUDIO_TEMPO(280), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    AUDIO_NOTE(E6,CROTCHET),                                  
    AUDIO_NOTE(G6,CROTCHET),                                          
    AUDIO_NOTE(AS6,CROTCHET),                         
    AUDIO_NOTE(DF7,CROTCHET),                         
    AUDIO_END
};

/* 2 tone falling scale: 300ms A6 E6 */
static const audio_note tone_1b[] =
{
    AUDIO_TEMPO(150), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(A6, QUAVER),
    AUDIO_NOTE(REST,DEMISEMIQUAVER),
    AUDIO_NOTE(E6, QUAVER),
    
    AUDIO_END
};

/* 4 low to high 38ms tones */
static const audio_note tone_1c[] =
{
    AUDIO_TEMPO(1600), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(G5 , CROTCHET),
    AUDIO_NOTE(D6 , CROTCHET),
    AUDIO_NOTE(G6 , CROTCHET),
    AUDIO_NOTE(B5 , CROTCHET),

    AUDIO_END
};

/* 5 high beeps: 50ms C6 REST C6 REST C6 REST C6 REST C6 REST*/
static const audio_note tone_1d[] =
{
    AUDIO_TEMPO(1200), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(C6  , CROTCHET),
    AUDIO_NOTE(REST  , CROTCHET),

    AUDIO_NOTE(C6  , CROTCHET),
    AUDIO_NOTE(REST  , CROTCHET),

    AUDIO_NOTE(C6  , CROTCHET),
    AUDIO_NOTE(REST  , CROTCHET),

    AUDIO_NOTE(C6  , CROTCHET),
    AUDIO_NOTE(REST  , CROTCHET),

    AUDIO_NOTE(C6  , CROTCHET),
    AUDIO_NOTE(REST  , CROTCHET),
    
    
    AUDIO_END
};

/* short double low: 100ms G5 G5 */
static const audio_note tone_1e[] =
{
    AUDIO_TEMPO(800), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, QUAVER_TRIPLET),
    AUDIO_NOTE(G5 , MINIM_TRIPLET),  
    AUDIO_NOTE(REST, QUAVER),
    AUDIO_NOTE(G5 , MINIM_TRIPLET),
    
    AUDIO_END
};

/* short high: 100ms G6 */
static const audio_note tone_1f[] =
{
    AUDIO_TEMPO(2400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(G6 , SEMIBREVE),
    
    AUDIO_END
};

/* short double high: 100ms G6 G6 */
static const audio_note tone_20[] =
{
    AUDIO_TEMPO(800), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST, QUAVER_TRIPLET),
    AUDIO_NOTE(G6 , MINIM_TRIPLET),  
    AUDIO_NOTE(REST, QUAVER),
    AUDIO_NOTE(G6 , MINIM_TRIPLET),
    
    AUDIO_END
};

/* 4 high to low 38ms tones */
static const audio_note tone_21[] =
{
    AUDIO_TEMPO(1600), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(B5 , CROTCHET),
    AUDIO_NOTE(G6 , CROTCHET),
    AUDIO_NOTE(D6 , CROTCHET),
    AUDIO_NOTE(G5 , CROTCHET),

    AUDIO_END
};

/* short 3: 100ms G6 */
static const audio_note tone_22[] =
{
    AUDIO_TEMPO(2400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(G6, SEMIBREVE), 
    
    AUDIO_END
};

/* short low to high: 100ms DS7 G7 */
static const audio_note tone_23[] =
{
    AUDIO_TEMPO(600), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(DS7  , CROTCHET),
    AUDIO_NOTE(REST , QUAVER),
    AUDIO_NOTE(G7   , CROTCHET),
    
    AUDIO_END
};

/* 5 rapid high tones: 94 ms B6 B6 B6 B6 B6 */
static const audio_note tone_24[] =
{
    AUDIO_TEMPO(640), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(B6 , CROTCHET),
    AUDIO_NOTE(B6 , CROTCHET),
    AUDIO_NOTE(B6 , CROTCHET),
    AUDIO_NOTE(B6 , CROTCHET),
    AUDIO_NOTE(B6 , CROTCHET),
    
    AUDIO_END
};

/* short: 100ms G5 */
static const audio_note tone_25[] =
{
    AUDIO_TEMPO(2400), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
	AUDIO_NOTE(REST, CROTCHET),
	AUDIO_NOTE(REST, CROTCHET),
	AUDIO_NOTE(REST, CROTCHET),
    AUDIO_NOTE(G5, SEMIBREVE), 
	AUDIO_NOTE(REST, CROTCHET),
	AUDIO_NOTE(REST, CROTCHET),
    
    AUDIO_END
};

/* long low: Mute 25ms, 400Hz 500ms */
static const audio_note tone_26[] =
{
    AUDIO_TEMPO(480), AUDIO_VOLUME(64), AUDIO_TIMBRE(sine),
    
    AUDIO_NOTE(REST  , SEMIQUAVER_TRIPLET),
    AUDIO_NOTE(G5  , SEMIBREVE),
    
    AUDIO_END
};



/***************************************************************************/
/*
    The Tone Array
*/
/*************************************************************************/
#define NUM_FIXED_TONES (38)


/* This must make use of all of the defined tones - requires the extra space first */
static const audio_note * const gFixedTones [ NUM_FIXED_TONES ] = 
{
/*1*/    tone_1,
/*2*/    tone_2,
/*3*/    tone_3,
/*4*/    tone_4,
/*5*/    tone_5,
/*6*/    tone_6,
/*7*/    tone_7, 
/*8*/    tone_8,
/*9*/    tone_9,
/*a*/    tone_a,		
/*b*/	 tone_b,
/*c*/    ring_twilight,
/*d*/    ring_greensleeves,
/*e*/    ring_major_scale,
/*f*/	 tone_f,
/*10*/	 tone_10,
/*11*/	 tone_11,
/*12*/	 tone_12,
/*13*/	 tone_13,
/*14*/	 tone_14,
/*15*/	 tone_15,
/*16*/	 tone_16,
/*17*/	 tone_17,
/*18*/	 tone_18,
/*19*/	 tone_19,
/*1a*/	 CustomRingTone,
/*1b*/	 tone_1b,		 
/*1c*/	 tone_1c,		 		 
/*1d*/	 tone_1d,		 		 
/*1e*/	 tone_1e,
/*1f*/	 tone_1f,
/*20*/	 tone_20,
/*21*/	 tone_21,
/*22*/	 tone_22,		 
/*23*/	 tone_23,
/*24*/	 tone_24,
/*25*/	 tone_25,		 
/*26*/	 tone_26				 
};    

/****************************************************************************
  FUNCTIONS
*/

/****************************************************************************
NAME    
    IsToneDefined
    
DESCRIPTION
  	Helper fn to determine if a tone has been defined or not.
    
RETURNS
    bool 
*/
static bool IsToneDefined ( HeadsetTone_t pTone )
{
    bool lResult = TRUE ;
    
    if ( pTone == TONE_NOT_DEFINED )
    {
        lResult = FALSE ;
    }
    
    if (  ! gFixedTones [ (pTone - 1) ] )
    {	    /*the tone is also not defined if no entry exists for it*/
        lResult = FALSE ; 
    }
    
    return lResult ;
}


/*****************************************************************************/
void TonesInit ( void ) 
{
    uint16 lEvent = 0 ;
    
    for ( lEvent =  0 ; lEvent  < EVENTS_MAX_EVENTS ; lEvent ++ )
    {
        theHeadset.gEventTones[ lEvent ] = TONE_NOT_DEFINED ;
    }
}


/*****************************************************************************/
void TonesConfigureEvent ( headsetEvents_t pEvent , HeadsetTone_t pTone ) 
{
    if ( IsToneDefined ( pTone ) ) 
    {
        if (pEvent == TONE_TYPE_RING )
        {   
            TONE_DEBUG(("TONE: ConfRingTone [%x]\n" , pTone)) ;
            theHeadset.RingTone = pTone ;
        }
        else
        {
                /* gEventTones is an array of indexes to the tones*/
            TONE_DEBUG(("Add Tone[%x][%x]\n" , pEvent , pTone ));
   
            theHeadset.gEventTones [ pEvent ] = pTone  ;
    
            TONE_DEBUG(("TONE; Add Ev[%x][%x]Tone[%x][%x]\n", pEvent , pEvent , pTone 
                                 , (int) theHeadset.gEventTones [ pEvent] )) ;
        }
    }    
}


/*****************************************************************************/
void TonesPlayEvent ( headsetEvents_t pEvent )
{    
    /*if the tone is present*/
    if (theHeadset.gEventTones [ pEvent ] != TONE_NOT_DEFINED )
    {
        TONE_DEBUG(("TONE: EvPl[%x][%x][%x]\n", pEvent, pEvent , (int) theHeadset.gEventTones [ pEvent]  )) ;
		
		if(pEvent == EventMuteReminder)			
		{
			/* check whether to play mute reminder tone at default volume level, never queue mute reminders to 
			   protect against the case that the tone is longer than the mute reminder timer */
	     	TonesPlayTone (theHeadset.gEventTones [ pEvent] ,FALSE, (theHeadset.features.MuteToneFixedVolume)) ;			
		}
		else
		{        
	    	TonesPlayTone (theHeadset.gEventTones [ pEvent] ,theHeadset.features.QueueEventTones, FALSE ) ;
		}
    }
    else
    {
        TONE_DEBUG(("TONE: NoPl[%x]\n", pEvent)) ;
    }        
} 


/*****************************************************************************/
void TonesPlayTone ( HeadsetTone_t pTone , bool pCanQueue , bool PlayToneAtDefaultLevel )
{
    if ( IsToneDefined(pTone) )			
    {   
        uint16 lToneVolume;
		uint16 ampOffDelay = theHeadset.ampOffDelay;
		uint16 newDelay = ampOffDelay;
		
		/* Turn the audio amp on */
		AmpOn();
		
		TONE_DEBUG(("TONE: PlayTone [%x]\n" , pTone)) ;
		
		/* The audio off delay must be greater than all tone lengths, so if it's
		   set very low then increase it slightly while a tone is played as 
		   otherwise the amp could be turned off before a tone has complete.
		*/
		if (theHeadset.dsp_process == dsp_process_none)
		{
			/* Only switch amp off if SCO and A2DP not active */
			if (theHeadset.ampAutoOff && (ampOffDelay < 3))
			{
				newDelay += 3; 
				AmpSetOffDelay(newDelay);
				AmpOffLater();
				AmpSetOffDelay(ampOffDelay);
			}
			else
			{
				AmpOffLater();
			}
		}
        
        if(PlayToneAtDefaultLevel || theHeadset.features.PlayHfpTonesAtFixedVolume)
        {
            lToneVolume = theHeadset.config->gVolLevels.toneVol;            
        }
        else
        {
			lToneVolume = theHeadset.config->gVolLevels.volumes[theHeadset.gHfpVolumeLevel].hfpGain;
        }
        
		AudioPlayTone ( gFixedTones [ pTone  - 1 ] , pCanQueue ,theHeadset.theCodecTask, lToneVolume , theHeadset.features.mono ? FALSE : TRUE ) ;
    }    
}


/*****************************************************************************/
void ToneTerminate ( void )
{
    AudioStopTone() ;
}  

