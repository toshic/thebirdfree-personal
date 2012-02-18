/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_sbc_encoder_plugin.c
DESCRIPTION
    an audio plugin
NOTES
*/

#include <codec.h>
#include <stdlib.h>
#include <print.h>
#include <stream.h> /*for the audio_note*/

#include "audio_plugin_if.h" /*the messaging interface*/
#include "csr_sbc_encoder_plugin.h"
#include "csr_sbc_encoder.h"

/*
	Currently the stereo DSP applications don't need to know what the type of the audio sink is,
	so this parameter of the message structure is ignored.  It has been ommited from he internal
	APIs to save a little stack space on the function calls.  This will need to change if the
	DSP applications are modified to have an interest in the type of audio sink.
*/

	/*the task message handler*/
static void message_handler (Task task, MessageId id, Message message) ;

	/*the local message handling functions*/
static void handleAudioMessage ( Task task , MessageId id, Message message ) 	;
static void handleInternalMessage ( Task task , MessageId id, Message message ) 	;
	
	/*the plugin task*/
const TaskData csr_sbc_encoder_plugin = { message_handler };

/****************************************************************************
DESCRIPTION
	The main task message handler
*/
static void message_handler ( Task task, MessageId id, Message message ) 
{
	if ( (id >= AUDIO_MESSAGE_BASE ) && (id <= AUDIO_MESSAGE_TOP) )
	{
		handleAudioMessage (task , id, message ) ;
	}
	else
	{
		handleInternalMessage (task , id , message ) ;
	}
}	

/****************************************************************************
DESCRIPTION

	messages from the audio library are received here. 
	and converted into function calls to be implemented in the 
	plugin module
*/ 
static void handleAudioMessage ( Task task , MessageId id, Message message ) 	
{
	switch (id)
	{
		case (AUDIO_PLUGIN_CONNECT_MSG ):
		{
			AUDIO_PLUGIN_CONNECT_MSG_T * connect_message = (AUDIO_PLUGIN_CONNECT_MSG_T *)message ;
	
			if (AUDIO_BUSY)
			{ 		/*Queue the connect message until the audio task is available*/
				MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_CONNECT_MSG ) ; 
				
				message->audio_sink = connect_message->audio_sink ;
				message->codec_task = connect_message->codec_task ;
				message->volume     = connect_message->volume ;
				message->rate		= connect_message->rate;
				message->mode 		= connect_message->mode ;
				message->stereo     = connect_message->stereo ;
				message->params     = connect_message->params ;
				
			    MessageSendConditionally ( task, AUDIO_PLUGIN_CONNECT_MSG , message , (const uint16 *)&AUDIO_BUSY ) ;
			} 
			else
			{		/*connect the audio*/
				CsrSbcEncoderPluginConnect(   connect_message->audio_sink , 
    									  connect_message->codec_task ,
    									  connect_message->volume , 
    									  connect_message->rate ,
    									  connect_message->stereo ,
    	     							  connect_message->mode   , 
                                          connect_message->params ) ;
			}			
		}	
		break ;
		
		case (AUDIO_PLUGIN_DISCONNECT_MSG ):
		{
			MessageCancelAll ( task , AUDIO_PLUGIN_CONNECT_MSG ) ;
			
			if (AUDIO_BUSY)
			{
				MessageSendConditionally ( task, AUDIO_PLUGIN_DISCONNECT_MSG , 0 ,(const uint16 *)&AUDIO_BUSY ) ;
    		}
			else
			{
				CsrSbcEncoderPluginDisconnect() ;
			}
		}	
		break ;
		
		case (AUDIO_PLUGIN_SET_MODE_MSG ):
		{
            AUDIO_PLUGIN_SET_MODE_MSG_T * mode_message = (AUDIO_PLUGIN_SET_MODE_MSG_T *)message ;			
			
            if (AUDIO_BUSY)
            {
                MAKE_AUDIO_MESSAGE ( AUDIO_PLUGIN_SET_MODE_MSG) ;
                message->mode   = mode_message->mode ;
                message->params = mode_message->params ;
        
        		MessageSendConditionally ( task, AUDIO_PLUGIN_SET_MODE_MSG , message ,(const uint16 *)&AUDIO_BUSY ) ;    	
            }
            else
            {
                CsrSbcEncoderPluginSetMode(mode_message->mode , mode_message->params) ;
            }
		}
		break ;
		
		case (AUDIO_PLUGIN_SET_VOLUME_MSG ): 
		{
			AUDIO_PLUGIN_SET_VOLUME_MSG_T * volume_message = (AUDIO_PLUGIN_SET_VOLUME_MSG_T *)message ;			
			
			if (AUDIO_BUSY)
			{
			     MAKE_AUDIO_MESSAGE (AUDIO_PLUGIN_SET_VOLUME_MSG ) ;
			     message->volume = volume_message->volume ;
			     
		         MessageSendConditionally ( task, AUDIO_PLUGIN_SET_VOLUME_MSG , message ,(const uint16 *)&AUDIO_BUSY ) ;  
            }
            else
            {
                CsrSbcEncoderPluginSetVolume ( volume_message->volume ) ;
            }			
		}		
		break ;
		
		case (AUDIO_PLUGIN_PLAY_TONE_MSG ):
		{
			AUDIO_PLUGIN_PLAY_TONE_MSG_T * tone_message = (AUDIO_PLUGIN_PLAY_TONE_MSG_T *)message ;
			
			if (AUDIO_BUSY) 
			{	
				if ( tone_message->can_queue) /*then re-queue the tone*/
				{				
					MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_PLAY_TONE_MSG ) ; 
					
					message->tone        = tone_message->tone       ;
					message->can_queue   = tone_message->can_queue  ;
					message->codec_task  = tone_message->codec_task ;
					message->tone_volume = tone_message->tone_volume;
					message->stereo      = tone_message->stereo     ;
	
					PRINT(("SBCDSP: Tone Q\n"));
					
					MessageSendConditionally ( task , AUDIO_PLUGIN_PLAY_TONE_MSG, message ,(const uint16 *)&AUDIO_BUSY ) ;			
				}
			}
			else
			{
				PRINT(("SBCDSP: Tone start\n"));				
				AUDIO_BUSY = (TaskData*) &csr_sbc_encoder_plugin;					
				CsrSbcEncoderPluginPlayTone ( tone_message->tone, tone_message->codec_task  , 					  
								        tone_message->tone_volume, tone_message->stereo      ) ;		     				
			}
							     
		}
		break ;
		
		case (AUDIO_PLUGIN_STOP_TONE_MSG ):
		{
			if (AUDIO_BUSY)
			{
				CsrSbcEncoderPluginStopTone() ;
			}
		}
		break ;		
		
		default:
		{
		}
		break ;
	}
}

/****************************************************************************
DESCRIPTION
	Internal messages to the task are handled here
*/ 
static void handleInternalMessage ( Task task , MessageId id, Message message ) 	
{
	PRINT(("SBCDSP: tone complete\n")) ;
		/*the tone has completed*/  
	AUDIO_BUSY = NULL ;
    
    CsrSbcEncoderPluginToneComplete() ; 	
}
