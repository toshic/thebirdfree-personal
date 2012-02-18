/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_faststream_source.c
DESCRIPTION
    plugin implentation which routes the sco audio though the dsp
NOTES
*/

#include <codec.h>
#include <pcm.h>
#include <stdlib.h>
#include <panic.h>
#include <print.h>
#include <file.h>
#include <stream.h> /*for the audio_note*/
#include <kalimba.h>
#include <kalimba_standard_messages.h>
#include <message.h> 
#include <Transform.h> 
                                                                                                                    
#include "audio_plugin_if.h" /*for the audio_mode*/
#include "csr_faststream_source.h"
#include "csr_faststream_source_plugin.h"


typedef enum
{
	SourceUsb = 0,
	SourceAnalog
} SourceType;

#define KALIMBA_ENCODER_SELECT			0x7300

#define MAX_AUDIO_SINKS	2

typedef struct sync_Tag
{
    Sink media_sink[MAX_AUDIO_SINKS];    
    Task codec_task;    
}FASTSTREAM_t ;

/*the synchronous audio data structure*/
static FASTSTREAM_t * FASTSTREAM = NULL ;

/****************************************************************************
DESCRIPTION
	This function connects a synchronous audio stream to the pcm subsystem
*/ 
void CsrFaststreamSourcePluginConnect( Sink audio_sink , Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params ) 
{
	typedef struct
	{
		unsigned source_type:4;
		unsigned reserved:4;
		uint8 content_protection;
		uint32 voice_rate;
		unsigned bitpool:8;
		unsigned format:8;
        uint16 packet_size;
		Sink media_sink_b;
	} codec_data_type;

	codec_data_type *codecData = (codec_data_type *) params;

	
    FASTSTREAM = (FASTSTREAM_t*)PanicUnlessMalloc (sizeof (FASTSTREAM_t) ) ;
    
    FASTSTREAM->media_sink[0] = audio_sink ;
    FASTSTREAM->codec_task = codec_task ;
	FASTSTREAM->media_sink[1] = codecData->media_sink_b;
	
    StreamDisconnect(StreamKalimbaSource(2), 0);
    StreamDisconnect(StreamSourceFromSink(audio_sink), audio_sink);

	/*
      FastStream does not use RTP.
      L2CAP frames enter/leave via port 2
    */

	/* Configure encoding format */
	if (!KalimbaSendMessage(KALIMBA_MSG_SBCENC_SET_PARAMS, codecData->format, 0, 0, 0))
		/* If message fails to get through, abort */
		Panic();

	/* Pass bit pool value to DSP */
	if (!KalimbaSendMessage(KALIMBA_MSG_SBCENC_SET_BITPOOL, codecData->bitpool, 0, 0, 0))
		/* If message fails to get through, abort */
		Panic();

	/* Connect the write port 2 to audio_sink */	
    PanicFalse(StreamConnect(StreamKalimbaSource(2),audio_sink));
    
    PRINT(("Audio Plugin: Connection sink:0x%x\n",(uint16)audio_sink));
    
    if (FASTSTREAM->media_sink[1])
	{
        /* There is a 2nd audio stream so initialise this connection */
        StreamDisconnect(StreamKalimbaSource(3), 0);
        StreamDisconnect(StreamSourceFromSink(FASTSTREAM->media_sink[1]), FASTSTREAM->media_sink[1]);
        PanicFalse(StreamConnect(StreamKalimbaSource(3), FASTSTREAM->media_sink[1]));
        PRINT(("Audio Plugin: Connection sink:0x%x\n", (uint16)FASTSTREAM->media_sink[1]));
    }
	    
	if(codecData->source_type == SourceUsb)
	{
		/* Select the source type */
		if (codecData->voice_rate)
		{
			(void) StreamConnect(StreamSourceFromSink(audio_sink), StreamKalimbaSink(2));
		}
		else
		{
			/* there's no mic channel, so discard any incoming data */
			StreamConnectDispose(StreamSourceFromSink(audio_sink));
            if (FASTSTREAM->media_sink[1])
	        {
                StreamConnectDispose(StreamSourceFromSink(FASTSTREAM->media_sink[1]));
            }
		}

		PanicFalse(KalimbaSendMessage(KALIMBA_ENCODER_SELECT, 0x0001, 0, 0, 0));
	}
	else
	{
		/* For analogue input source */
		StreamDisconnect(StreamPcmSource(0), StreamPcmSink(0));
		StreamDisconnect(StreamPcmSource(1), StreamPcmSink(1));
        
		(void)PcmClearAllRouting();

		if (CodecGetCodecType(codec_task) == codec_wm8731)
		{
			/* configure slot 0 and 1 to be left and right channel
			and synchronise the offsets for stereo playback */
			(void)PcmRateAndRoute(0, PCM_NO_SYNC, (uint32) rate, (uint32) rate, VM_PCM_EXTERNAL_I2S);
			(void)PcmRateAndRoute(1, 0, (uint32) rate, (uint32) rate, VM_PCM_EXTERNAL_I2S);
		}
		else
		{
			/* configure slot 0 and 1 to be left and right channel
			and synchronise the offsets for stereo playback */
			(void)PcmRateAndRoute(0, PCM_NO_SYNC, (uint32) rate, (uint32) rate, VM_PCM_INTERNAL_A);
			(void)PcmRateAndRoute(1, 0, (uint32) rate, (uint32) rate, VM_PCM_INTERNAL_B);
		}
		        
		/* plug Left ADC into port 0 */
		(void)StreamConnect(StreamPcmSource(0),StreamKalimbaSink(0)); 

		/* plug Right ADC into port 1 */
		(void)StreamConnect(StreamPcmSource(1),StreamKalimbaSink(1)); 
		
		/* Select the source type */
		PanicFalse(KalimbaSendMessage(KALIMBA_ENCODER_SELECT, 0x0002, 0, 0, 0)); 
		
		if(!KalimbaSendMessage(KALIMBA_MSG_GO,0,0,0,0))
		{
			PRINT(("FASTSTREAM: Message KALIMBA_MSG_GO failed!\n"));
			Panic();
		}
	}
}

/****************************************************************************
DESCRIPTION
	Disconnect Sync audio
*/
void CsrFaststreamSourcePluginDisconnect( void ) 
{   
    uint16 i;
    
    if (!FASTSTREAM)
        Panic() ;
    
    for (i = 0; i < MAX_AUDIO_SINKS; i++)
	{
		if (FASTSTREAM->media_sink[i] != 0)
		{										
			/* Disconnect the Kalimba source from the media sink */
            StreamDisconnect(StreamKalimbaSource(i+2), FASTSTREAM->media_sink[i]);
            
            PRINT(("Audio Plugin: Disconnect media i:%d sink:0x%x\n",i,(uint16)FASTSTREAM->media_sink[i]));
                
            /* clear the audio sink */	
	        FASTSTREAM->media_sink[i] = 0;			            
		}
	}
          
    free (FASTSTREAM);
    FASTSTREAM = NULL ;        
}

/****************************************************************************
DESCRIPTION
	Indicate the volume has changed
*/
void CsrFaststreamSourcePluginSetVolume( uint16 volume ) 
{
	CodecSetOutputGainNow(FASTSTREAM->codec_task, volume, left_and_right_ch);	
}

/****************************************************************************
DESCRIPTION
	Set the mode
*/
void CsrFaststreamSourcePluginSetMode ( AUDIO_MODE_T mode , const void * params ) 
{   
    typedef struct
	{
		bool connect_sink;
		Sink media_sink;
	} faststream_mode_params;
	
	faststream_mode_params *faststream_mode = (faststream_mode_params *) params;
	uint16 i;
	
    if (!FASTSTREAM)
       	Panic() ;
           
    PRINT(("Audio Plugin: Set Mode\n"));
	
	if (faststream_mode->connect_sink)
	{
		/* A second audio sink has been connected */
		for (i = 0; i < MAX_AUDIO_SINKS; i++)
		{
			if (FASTSTREAM->media_sink[i] == 0)
			{
				/* store the audio sink */	
				FASTSTREAM->media_sink[i] = faststream_mode->media_sink;	
						
				(void) StreamConnect(StreamKalimbaSource(i+2), FASTSTREAM->media_sink[i]);
                
                PRINT(("Audio Plugin: New connection i:%d sink:0x%x\n",i,(uint16)FASTSTREAM->media_sink[i]));
			}
		}
	}
	else
	{
		/* An audio sink has been disconnected */
		for (i = 0; i < MAX_AUDIO_SINKS; i++)
		{
			if (FASTSTREAM->media_sink[i] == faststream_mode->media_sink)
			{										
				/* Disconnect the Kalimba source from the media sink */
                StreamDisconnect(StreamKalimbaSource(i+2), FASTSTREAM->media_sink[i]);
                
                PRINT(("Audio Plugin: Disconnect media i:%d sink:0x%x\n",i,(uint16)FASTSTREAM->media_sink[i]));
                
                /* clear the audio sink */	
				FASTSTREAM->media_sink[i] = 0;			                
			}
		}
	}
    free(faststream_mode);
}



/****************************************************************************
DESCRIPTION
    plays a tone using the audio plugin    
*/
void CsrFaststreamSourcePluginPlayTone ( audio_note * tone, Task codec_task, uint16 tone_volume , bool stereo) 
{    
    Source lSource ;  
    Sink lSink ; 
	
    if (!FASTSTREAM)
        Panic() ;
    
    PRINT(("FASTSTREAM: Tone Start\n")) ;

    lSink = StreamKalimbaSink(3) ;
    
        /*request an indication that the tone has completed / been disconnected*/
    MessageSinkTask ( lSink , (TaskData*) &csr_faststream_source_plugin ) ;

        /*connect the tone*/
    lSource = StreamAudioSource ( (const audio_note *) (tone) ) ;    
        /*mix the tone to the FASTSTREAM*/    
    StreamConnect( lSource , lSink ) ; 
	
}

/****************************************************************************
DESCRIPTION
	Stop a tone from currently playing
*/
void CsrFaststreamSourcePluginStopTone ( void ) 
{  
    if (!FASTSTREAM)
        Panic() ;
        
     StreamDisconnect ( 0 , StreamKalimbaSink(3) ) ;
}

/****************************************************************************
DESCRIPTION
	Reconnects the audio after a tone has completed 
*/
void CsrFaststreamSourcePluginToneComplete ( void ) 
{
        /*we no longer want to receive stream indications*/
    MessageSinkTask ( StreamKalimbaSink(3) , NULL) ;
}

    
