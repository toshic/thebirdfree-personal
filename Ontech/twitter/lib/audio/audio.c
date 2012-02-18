/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	audio.c        
	
DESCRIPTION
     The main audio manager file
     
*/ 
#include "audio.h"

#include <codec.h>
#include <pcm.h>
#include <stdlib.h>
#include <panic.h>
#include <stream.h>
#include <csr_tone_plugin.h> /*allows tones to be played while no other plugin is in use*/
#include <print.h>

typedef struct audio_Tag
{
    Task 	 plugin ;    
	
	/*parameters needed for the reconnection following a tts command*/
	/*
		Currently Stereo Headset does not require TTS.
		This code uses a lot of globals, so remove using #if 0.
		When TTS is required, this will need to be re-included somehow.
	*/
#if 0
    Sink         audio_sink ;
    AUDIO_SINK_T sink_type ;
    Task         codec_task ;
    uint16       volume ;
    uint32       rate ; 
    bool         stereo; 
    AUDIO_MODE_T mode ;
    const void * params ; 
#endif
}AUDIO_t ;

    /*the global audio library data structure*/
static AUDIO_t AUDIO = {0} ;

Task AUDIO_BUSY = NULL ;

/****************************************************************************
NAME	
	AudioSyncConnect

DESCRIPTION
	This function connects cvc to the stream subsystem

RETURNS
	void
*/
bool AudioConnect (  Task audio_plugin,  Sink audio_sink , AUDIO_SINK_T sink_type , Task codec_task , uint16 volume , uint32 rate ,  bool stereo , AUDIO_MODE_T mode , const void * params ) 
{   
	/*send a message to the audio plugin*/
	MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_CONNECT_MSG ) ;
	
	message->audio_sink = audio_sink ;
	message->sink_type  = sink_type ; 
	message->codec_task = codec_task ;
	message->volume     = volume ;
	message->rate		= rate;
	message->mode 		= mode ;
	message->stereo     = stereo ; 
	message->params     = params ;
	
    MessageSendConditionally ( audio_plugin, AUDIO_PLUGIN_CONNECT_MSG , message , (const uint16 *)&AUDIO_BUSY ) ;
	
    	/*store local audio state*/
    PRINT(("AUD: SyncConnect[%x]\n", (int)audio_sink)) ;
 	
    AUDIO.plugin = audio_plugin ;
    
	/*
		Currently Stereo Headset does not require TTS.
		This code uses a lot of globals, so remove using #if 0.
		When TTS is required, this will need to be re-included somehow.
	*/
#if 0
	AUDIO.audio_sink = audio_sink ;
	AUDIO.sink_type  = sink_type ; 
	AUDIO.codec_task = codec_task ;
	AUDIO.volume     = volume ;
	AUDIO.rate 		 = rate; 
	AUDIO.mode 	 	 = mode ;
	AUDIO.stereo     = stereo ; 
	AUDIO.params     = params ;
#endif
	
	return TRUE ;
}

/****************************************************************************
NAME	
	AudioDisconnect

DESCRIPTION
	Disconnect any currently connected audio
    
RETURNS
	void
*/
void AudioDisconnect( void )
{
    PRINT(("AUD: Disconnect\n")) ;
	
    if ( AUDIO.plugin )
    {
    	MessageSendConditionally ( AUDIO.plugin, AUDIO_PLUGIN_DISCONNECT_MSG , 0 , (const uint16 *)&AUDIO_BUSY ) ;
    }
    
    AUDIO.plugin = NULL ;
}

/****************************************************************************
NAME	
	AudioSetVolume

DESCRIPTION
	update the volume of any currently conencted audio

RETURNS
	void
*/
void AudioSetVolume( uint16 volume , Task codec_task )
{
	if ( AUDIO.plugin )   
    {
	    MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_SET_VOLUME_MSG ) ; 
	
		message->codec_task = codec_task ;
		message->volume     = volume ;
		
	    MessageSendConditionally ( AUDIO.plugin, AUDIO_PLUGIN_SET_VOLUME_MSG, message , (const uint16 *)&AUDIO_BUSY ) ;
    }
}

/****************************************************************************
NAME	
	AudioSetMode

DESCRIPTION
	Set the audio conenction mode

RETURNS
	void
*/
bool AudioSetMode ( AUDIO_MODE_T mode , const void * params )
{
    bool lResult = FALSE ;
    
    if ( AUDIO.plugin )
    {
	    MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_SET_MODE_MSG ) ; 
	
		message->mode = mode ;
		message->params = params ;
		
	    MessageSendConditionally ( AUDIO.plugin, AUDIO_PLUGIN_SET_MODE_MSG, message , (const uint16 *)&AUDIO_BUSY ) ;
	    
	    lResult = TRUE ;
    }
    
    return lResult ;   
}

/****************************************************************************
NAME	
	AudioPlayTone

DESCRIPTION

    queues the tone if can_queue is TRUE and there is already a tone playing

RETURNS 
    
*/
void AudioPlayTone ( const audio_note * tone , bool can_queue , Task codec_task, uint16 tone_volume , bool stereo ) 
{   	
	if (AUDIO_BUSY == NULL || can_queue) 
	{
		MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_PLAY_TONE_MSG ) ; 
	
    	message->tone        = (audio_note *) (tone) ;
    	message->can_queue   = can_queue ;
    	message->codec_task  = codec_task ;
    	message->tone_volume = tone_volume  ;
    	message->stereo      = stereo ;
	
		if ( AUDIO.plugin )
	    {
		    MessageSendConditionally ( AUDIO.plugin, AUDIO_PLUGIN_PLAY_TONE_MSG, message , (const uint16 *)&AUDIO_BUSY ) ;
	    }
    	else
	    {    
			MessageSendConditionally( (TaskData*)&csr_tone_plugin, AUDIO_PLUGIN_PLAY_TONE_MSG, message , (const uint16 *)&AUDIO_BUSY ) ;
	    }
	}
	else
	{
		PRINT(("AUDIO: discard tone \n"));
	} 
}

/****************************************************************************
NAME	
	AudioStopTone

DESCRIPTION
	Stop a tone from playing

RETURNS
	
*/
void AudioStopTone ( void ) 
{
	if(AUDIO_BUSY)
	{	
		MessageSend ( (TaskData*)AUDIO_BUSY , AUDIO_PLUGIN_STOP_TONE_MSG, 0 ) ;
		
		MessageCancelAll((TaskData*)&(csr_tone_plugin),AUDIO_PLUGIN_PLAY_TONE_MSG);
		MessageCancelAll((TaskData*) AUDIO_BUSY ,AUDIO_PLUGIN_PLAY_TONE_MSG);
	}
}


/****************************************************************************
NAME	
	AudioPlayTTS

DESCRIPTION

    plays / queues the TTS phrase if already TTS'ing or busy. 

RETURNS
    
*/
void AudioPlayTTS ( Task plugin , uint16 id , uint8 * data , uint16 size_data , AUDIO_TTS_LANGUAGE_T language , bool can_queue , Task codec_task, uint16 tts_volume , bool stereo )
{   
	MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_PLAY_TTS_MSG ) ; 
	
	message->id          = id ;
	message->data        = data ;
	message->size_data   = size_data ;
	message->language    = language ;
	message->can_queue   = can_queue ;
	message->codec_task  = codec_task ;
	message->tts_volume  = tts_volume  ;
	message->stereo      = stereo ;
	
	if (AUDIO.plugin) 
	{
        /*if we have a plugin connected, then perform the disconnect*/
        MessageSendConditionally ( AUDIO.plugin, AUDIO_PLUGIN_DISCONNECT_MSG , 0 , (const uint16 *)&AUDIO_BUSY ) ;
        MessageSendConditionally ( plugin , AUDIO_PLUGIN_PLAY_TTS_MSG , message , (const uint16 *)&AUDIO_BUSY ) ;           
    
        {
            MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_CONNECT_MSG ) ;
        	
	/*
		Currently Stereo Headset does not require TTS.
		This code uses a lot of globals, so remove using #if 0.
		When TTS is required, this will need to be re-included somehow.
	*/
#if 0
        	message->audio_sink = AUDIO.audio_sink ;
        	message->sink_type  = AUDIO.sink_type ; 
        	message->codec_task = AUDIO.codec_task ;
        	message->volume     = AUDIO.volume ;
        	message->rate		= AUDIO.rate;
        	message->mode 		= AUDIO.mode ;
        	message->stereo     = AUDIO.stereo ; 
        	message->params     = AUDIO.params ;
#endif
        	
            MessageSendConditionally ( AUDIO.plugin, AUDIO_PLUGIN_CONNECT_MSG , message , (const uint16 *)&AUDIO_BUSY ) ;
        }    
        
    }
    else
    {
        MessageSendConditionally ( plugin , AUDIO_PLUGIN_PLAY_TTS_MSG ,message , (const uint16 *)&AUDIO_BUSY ) ;        
    }	
}

/****************************************************************************
NAME	
	AudioStopTTS

DESCRIPTION
	Stop TTS from playing

RETURNS
*/
void AudioStopTTS ( TaskData * plugin )
{
	if ( AUDIO_BUSY && (AUDIO_BUSY == plugin) )  /* only send message if plugin is currently tts'ing */
	{
	    MessageSend ((TaskData*)plugin, AUDIO_PLUGIN_STOP_TTS_MSG, 0 ) ;
    }
}



