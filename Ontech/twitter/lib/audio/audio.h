/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
    audio.h
    
DESCRIPTION
    header file for the audio library	
*/

/*!
@file   audio.h
@brief  Header file for the audio library.
    This defines the Application Programming interface to the audio library.
    
    i.e. the interface between the VM application and the audio library.
        
    see the audio_plugin_if documentation for details of the API between the 
    audio library and an underlying audio plugin.
*/

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <message.h>
#include <stream.h> /*for the audio_note*/
#include "audio_plugin_if.h"

/*!
    @brief Connects an audio stream to the underlying audio plugin 
    
    @param audio_plugin The audio plugin to use for the audio connection
    @param audio_sink The Sink to connect (may be synchronous AV or other) 
    @param sink_type The type of audio conenction required - see AUDIO_SINK_T
    @param codec_task The codec task to connect the audio stream to
    @param volume The volume at which to connect the audio plugin. 
                  The plugin vendor is responsible for the use of this value    
    @param stereo Whether or not a stereo connection is to be used (channel A or channel A & B)
    @param mode The mode to set the audio connection to
                This behaviour is plugin dependent
    @param params Used to specify plugin specific parameters - See plugin vendor for details 
    
    A call to AudioConnect must be followed by a call to AudioDisconnect
    i.e. only a single audio plugin can be supported at any one time
    
    The underlying audio plugin is responsible for conencting up the audio sink
    as requested
    
    Calls to AudioConnect will be queued if the AUDIO_BUSY flag is set. 
    i.e. if a tone is currently being played, then the audio conenction will occur
    once the tone has completed.
      
*/
bool AudioConnect(Task audio_plugin,  Sink audio_sink , AUDIO_SINK_T sink_type , Task codec_task , uint16 volume, uint32 rate  , bool stereo , AUDIO_MODE_T mode , const void * params);

/*!
    @brief Disconnects an audio stream previously connected using AudioConnect
    
    The underlying audio_plugin previously connected using AudioConnect() 
    will be asked to perform the disconnect.
    
    Calls to AudioDisconnect when no plugin is connected will be ignored
    
*/
void AudioDisconnect(void);

/*!
    @brief Updates the volume of any currently connected audio connection
    
    @param volume The new volume to pass to the currently connected audio plugin
    @param codec_task The codec task used for the underlying audio connection
    
    The volume is set by the underlying audio plugin and the behaviour is specific
    to the implementation of the plugin.
    
    Some plugins may interpret this as a codec gain
    Others may choose to ignore this value etc 
    
    Note : the initial volume setting is passed in as part of AudioConnect 
*/    
void AudioSetVolume(uint16 volume , Task codec_task);

/*!
    @brief Updates the mode of any currently connected audio conenction
    
    @param mode The mode to set the audio connection to
                This behaviour is plugin dependent
    @param params Used to specify plugin specific parameters - See plugin vendor for details
    
    The mode can be used to change the conenction behaviour of an underlying plugin and this 
    behaviour is not supported by all plugin vendors.
    
    This call is ignored if no plugin is currently connected
    
    Note : The mode & params are passed in as part of AudioConnect   
*/
bool AudioSetMode(AUDIO_MODE_T mode , const void * params);

/*!
    @brief Plays a tone 
    
    @param tone The tone to be played 
    @param can_queue Whether or not to queue the requested tone
    @param codec_task The codec task to connect the tone to
    @param tone_volume The volume at which to play the tone (0 = play tone at current volume)
    @param stereo Whether or not a stereo connection is to be used (channel A or channel A & B)                    
    
    Tone playback can be used when an audio connection is present or not. (plugin connected or not)
    
    When a plugin is not connected, the standard tone playback plugin is used - csr_tone_plugin
    This allows tones to be played direct to the codec task (pcm) without the use of a DSP
    
    When a plugin is connected, the tone playback request will be passed to the currently connected plugin
    The underlying plugin is then responsible for connecting up the audio tone.
    Some plugins may choose to ignore tone requests in certain modes 
        
    For Text-To-Speech style plugins, play tone could be used to pass in TTS specific data
    This could be used after conenction of the audio plugin using AudioConnect
    see - csr_simple_text_to_speech_plugin for implementation details
    
    Tone queing can be achieved using the can_queue parameter. If this is selectyed, then
    tones will be queued using the audio flag AUDIO_BUSY.
    Tones will be played back in the order they are requested.       
*/

void AudioPlayTone ( const audio_note * tone , bool can_queue , Task codec_task, uint16 tone_volume , bool stereo ) ;

/*!
    @brief Stops a currently playing tone
    
    If a tone is currently connected to either the default tone plugin (csr_tone_plugin)
    or to any other connceted plugin then, the tone can be stopped part way through.
    
    In general, this is used to end ring tones prematurely to allow fast call setup times.
    
    Note : The implementation of AudioStopTone is plugin specific.
    Some plugins may choose to ignore the request to stop playback of a tone
*/
void AudioStopTone ( void ) ;



/*!
    @brief Plays text-to-speech

    @param plugin TTS plugin to use

    @param id Identifier used by the TTS plug-in to determine which TTS to
	      play, eg name caller ID

    @param data Pointer to the payload for this particular id, eg string containing the
	        caller's name. If not needed data should be set to zero. It is the
	        responsibility of the TTS plugin to free this data when
	        it has finished with it
	
    @param size_data The number of bytes pointed to by above data

    @param can_queue If this is TRUE, and TTS cannot currently be played 
	             (eg tone is playing), then it will be played on 
	             completion of the currently playing tone

    @param tts_volume The volume at which to play TTS. A non-zero value will 
                      cause the TTS to played at this volume level.
*/
void AudioPlayTTS ( Task plugin , uint16 id , uint8 * data , uint16 size_data , AUDIO_TTS_LANGUAGE_T language , bool can_queue , Task codec_task, uint16 tts_volume , bool stereo );

/*!
    @brief Stop TTS from playing
*/
void AudioStopTTS ( Task plugin );

#endif
