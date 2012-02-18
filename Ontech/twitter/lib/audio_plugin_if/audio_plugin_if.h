/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    audio_plugin_if.h
    
DESCRIPTION
	
*/

/*!
@file   audio_plugin_if.h
@brief  Header file for the audio plugin interface.

    The parameters / enums here define the message interface used for the 
    audio plugin.
    i.e This defines the interface between the audio library and the underlying plugin 
    
    The contents of these are similar to the parameters passed into the 
    audio library itself
    
    These messages are generated by the audio library and received in the message handler
    associate with the constant task of an audio plugin.
    
    The company_name_action_plugin.c / h files contain the meessage handler which 
    receives these messages.    
    
    The plugin itself is responsible for acting upon these messages.
     
*/


#ifndef _AUDIO_PLUGIN_INTERFACE_H_
#define _AUDIO_PLUGIN_INTERFACE_H_

/*the Mode*/
typedef enum AudioModeTag
{
    AUDIO_MODE_MUTE_MIC     ,
    AUDIO_MODE_MUTE_SPEAKER ,
    AUDIO_MODE_MUTE_BOTH    ,    
    AUDIO_MODE_CONNECTED      
}AUDIO_MODE_T ;


/*the audio sink type*/
typedef enum AudioSinkTag
{
    AUDIO_SINK_INVALID  ,
    AUDIO_SINK_SCO      ,
    AUDIO_SINK_ESCO     ,
    AUDIO_SINK_AV
} AUDIO_SINK_T ;

/*the TTS language*/
typedef enum AudioTTSLanguageTag
{
    AUDIO_TTS_LANGUAGE_BRITISH_ENGLISH,
    AUDIO_TTS_LANGUAGE_AMERICAN_ENGLISH,
    AUDIO_TTS_LANGUAGE_FRENCH,
    AUDIO_TTS_LANGUAGE_ITALIAN,
    AUDIO_TTS_LANGUAGE_GERMAN,
    AUDIO_TTS_LANGUAGE_SPANISH_EUROPEAN,
    AUDIO_TTS_LANGUAGE_SPANISH_MEXICAN,
    AUDIO_TTS_LANGUAGE_PORTUGUESE_BRAZILIAN,
    AUDIO_TTS_LANGUAGE_CHINESE_MANDARIN,
    AUDIO_TTS_LANGUAGE_KOREAN,
    AUDIO_TTS_LANGUAGE_RUSSIAN,
    AUDIO_TTS_LANGUAGE_ARABIC,
    AUDIO_TTS_LANGUAGE_FINNISH,
	AUDIO_TTS_LANGUAGE_JAPANESE
}AUDIO_TTS_LANGUAGE_T ;

/* Macros for creating messages */
#include <panic.h>
#define MAKE_AUDIO_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_AUDIO_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);

/*!
	@brief Define the types for the upstream messages sent from the Hfp profile
	library to the application.
*/
#define AUDIO_MESSAGE_BASE	0x5d00

/*! \name Audio Plugin Interface Messages

	These messages are sent to the audio plugin modules.
	
	An Audio plugin must implement all of the messages below
	
*/
typedef enum audio_plugin_interface_message_type_tag 
{
	AUDIO_PLUGIN_CONNECT_MSG    = AUDIO_MESSAGE_BASE,
	AUDIO_PLUGIN_DISCONNECT_MSG ,
	AUDIO_PLUGIN_SET_MODE_MSG   ,
	AUDIO_PLUGIN_SET_VOLUME_MSG , 
	AUDIO_PLUGIN_PLAY_TONE_MSG  ,
	AUDIO_PLUGIN_STOP_TONE_MSG  , 
	AUDIO_PLUGIN_PLAY_TTS_MSG   ,
	AUDIO_PLUGIN_STOP_TTS_MSG  , 
	
	AUDIO_MESSAGE_TOP	
	
} audio_plugin_interface_message_type_t ; 

/*!
	@brief This message is generated by the audio manager and is issued
	to an audio plugin module. 
	The plugin module should connect the Synchronous connection to its pre
	defined outputs.
*/
typedef struct 
{
		/*! The audio sink to connect*/
	Sink            audio_sink ;
		/*! The type of the audio sink to connect*/
	AUDIO_SINK_T sink_type ;
	    /*! The codec task to use to connect the audio*/
    Task            codec_task ;
 		/*! The volume at which to set the audio */
    uint16          volume ;
    	/*! The rate of the audio stream */
    uint32			rate ;
		/*! whether or not to route mono / stereo audio*/
    bool 			stereo ; 
		/*! The audio mode of connection required*/
    AUDIO_MODE_T	mode ;    
        /*!plugin specific parameters*/
    const void *    params ;       
}AUDIO_PLUGIN_CONNECT_MSG_T ;


/*!
	@brief This message is generated by the audio manager and is issued
	to an audio plugin module. 
	The plugin module should connect the Synchronous connection to its pre
	defined outputs.
*/
typedef struct 
{
 		/*! The volume at which to set the audio */
    uint16          volume ;
 		/*! The codec task to use to connect the audio*/
    Task            codec_task ;
}AUDIO_PLUGIN_SET_VOLUME_MSG_T ;


/*!
	@brief This message is generated by the audio manager and is issued
	to an audio plugin module. 
*/
typedef struct 
{
 		/*! The audio connection mode */
    uint16          mode ;
        /*! plugin specific parameters*/
    const void *    params ;
}AUDIO_PLUGIN_SET_MODE_MSG_T ;


/*!
	@brief This message is generated by the audio manager and is issued
	to an audio plugin module. 
*/
typedef struct 
{
 		/*! the tone to be played*/
    audio_note * tone;
    	/*! Whether or not to queue the requested tone*/
    bool 	    can_queue ; 
 		/*! The codec task to use to connect the audio*/
    Task            codec_task ;
    	/*! The volume at which to play the tone 0 - current volume*/
    uint16 		tone_volume ;
		/*! whether or not to route mono / stereo audio*/
    bool 		stereo ; 
    
}AUDIO_PLUGIN_PLAY_TONE_MSG_T ;

/*!
	@brief This message is generated by the audio manager and is issued
	to an audio plugin module. 
*/
typedef struct 
{
    	/*! the id of the TTS to be played*/
    uint16 id ;
    	/*! pointer to the payload for this particular id*/
    uint8 * data ;
    	/*! the number of bytes in the payload */
    uint16 size_data ;
    	/*! the language to use*/
    AUDIO_TTS_LANGUAGE_T language ;
    	/*! Whether or not to queue the requested tts phrase*/
    bool 	    can_queue ; 
 		/*! The codec task to use to connect the audio*/
    Task            codec_task ;
    	/*! The volume at which to play the tts phrase 0 - current volume*/
    uint16 		tts_volume ;
		/*! whether or not to route mono / stereo audio*/
    bool 		stereo ; 
    
}AUDIO_PLUGIN_PLAY_TTS_MSG_T ;


/*!
	@brief This global flag acts as a semaphore that indicates if the audio plugin is busy
    All messages will be queued conditionally on the value of AUDIO_BUSY ; 
    
    This is the main way that the audio library schedules audio conenctions / tone requests etc.
    
    If an audio plugin wishes to prevent tone or disconnection requests being received for
    short periods of time, then this cna be achieved by setting the AUDIO_BUSY flag = TRUE
    
    Note. Setting this to TRUE for long periods of time will result in a build up of audio messages
    which may result in a shortage of memory. As such this is only recommended for short periods 
    of time - see csr_cvc_headset_plugin for an example of use. 
    
*/
extern Task AUDIO_BUSY ;

#endif


