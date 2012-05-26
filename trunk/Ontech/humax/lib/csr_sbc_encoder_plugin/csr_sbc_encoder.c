/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_sbc_encoder.c
DESCRIPTION
    plugin implentation which routes USB audio though the dsp SBC encoder
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
#include <transform.h> 
                                                                                                                    
#include "audio_plugin_if.h" /*for the audio_mode*/
#include "csr_sbc_encoder.h"
#include "csr_sbc_encoder_plugin.h"


#define   KALIMBA_ENCODER_SELECT			         0x7300

#define MAX_AUDIO_SINKS	2

typedef enum
{
	SourceUsb = 0,
	SourceAnalog
} SourceType;

typedef struct sync_Tag
{
    Sink media_sink[MAX_AUDIO_SINKS];    
	Transform t[MAX_AUDIO_SINKS];
    Task codec_task;    
	uint16 packet_size;
}SBC_t ;

    /*the synchronous audio data structure*/
static SBC_t * SBC = NULL ;

/****************************************************************************
DESCRIPTION
	This function connects a synchronous audio stream to the pcm subsystem
*/ 
void CsrSbcEncoderPluginConnect( Sink audio_sink , Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params ) 
{
	/* DSP Application loading and Transform starting is handled by a call to A2dpAudioCodecEnable
		in the application, so should not be done here. */

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
	} sbc_codec_data_type;

	sbc_codec_data_type *sbc_codecData = (sbc_codec_data_type *) params;

	if (!sbc_codecData)
		Panic();

    SBC = (SBC_t*)PanicUnlessMalloc (sizeof (SBC_t) ) ;
    
    SBC->media_sink[0] = audio_sink ;
    SBC->codec_task = codec_task ;
	SBC->media_sink[1] = sbc_codecData->media_sink_b;
	SBC->packet_size = sbc_codecData->packet_size;
    
    StreamDisconnect(StreamKalimbaSource(2), 0);

	/* Initialise the RTP SBC encoder */
	SBC->t[0] = TransformRtpSbcEncode(StreamKalimbaSource(2), audio_sink);

	/* Configure the RTP transform to generate the selected packet size */
	TransformConfigure(SBC->t[0], VM_TRANSFORM_RTP_SBC_ENCODE_PACKET_SIZE, sbc_codecData->packet_size);

	/* Transform should not manage timings. */
	TransformConfigure(SBC->t[0], VM_TRANSFORM_RTP_SBC_ENCODE_MANAGE_TIMING, FALSE);

	/* Start the transform */
	(void) TransformStart(SBC->t[0]);
    
    PRINT(("Audio Plugin: TransformStart sink:0x%x\n",(uint16)audio_sink));
	
	if (SBC->media_sink[1])
	{
		/* There is a 2nd audio stream so initialise this transform */
        StreamDisconnect(StreamKalimbaSource(3), 0);
		
		/* Initialise the RTP SBC encoder */
		SBC->t[1] = TransformRtpSbcEncode(StreamKalimbaSource(3), SBC->media_sink[1]);

		/* Configure the RTP transform to generate the selected packet size */
		TransformConfigure(SBC->t[1], VM_TRANSFORM_RTP_SBC_ENCODE_PACKET_SIZE, sbc_codecData->packet_size);

		/* Transform should not manage timings. */
		TransformConfigure(SBC->t[1], VM_TRANSFORM_RTP_SBC_ENCODE_MANAGE_TIMING, FALSE);

		/* Start the transform */
		(void) TransformStart(SBC->t[1]);
        
        PRINT(("Audio Plugin: TransformStart sink:0x%x\n",(uint16)SBC->media_sink[1]));
	}

	/* Configure SBC encoding format */
	if (!KalimbaSendMessage(KALIMBA_MSG_SBCENC_SET_PARAMS, sbc_codecData->format, 0, 0, 0))
		/* If message fails to get through, abort */
		Panic();

	/* Pass bit pool value to DSP */
	if (!KalimbaSendMessage(KALIMBA_MSG_SBCENC_SET_BITPOOL, sbc_codecData->bitpool, 0, 0, 0))
		/* If message fails to get through, abort */
		Panic();

	/* disard any data sent by the SNK */
    StreamConnectDispose(StreamSourceFromSink(audio_sink));
	
	if (SBC->media_sink[1])
	{
		/* disard any data sent by the 2nd SNK */
	    StreamConnectDispose(StreamSourceFromSink(SBC->media_sink[1]));
	}
	
	/* select the source type */
	if(sbc_codecData->source_type == SourceUsb)
	{
		PRINT(("Audio Plugin: SourceUsb\n"));
		/* Select the source type */
		PanicFalse(KalimbaSendMessage(KALIMBA_ENCODER_SELECT, 0x0001, 0, 0, 0));
	}
	else
	{
		PRINT(("Audio Plugin: SourceAnalog\n"));
		/* For analogue input source */
		StreamDisconnect(StreamPcmSource(0), StreamPcmSink(0));
		StreamDisconnect(StreamPcmSource(1), StreamPcmSink(1));
        
		(void)PcmClearAllRouting();

		{
			PRINT(("Audio Plugin: codec_internal\n"));
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
			PRINT(("SBC: Message KALIMBA_MSG_GO failed!\n"));
			Panic();
		}
	}

	/* Select the source type */
	/*PanicFalse(KalimbaSendMessage(KALIMBA_SOURCE_SELECT, SOURCE_USB, 0, 0, 0)); */
	
    /*CsrSbcEncoderUsbPluginSetVolume(volume) ;*/
	
	/*(void) StreamConnect(PanicNull(StreamUsbEndPointSource(end_point_iso_in)), StreamKalimbaSink(0));
    */
	/* Start decode */
   /* if(!KalimbaSendMessage(KALIMBA_MSG_GO,0,0,0,0))
	{
		PRINT(("SBC: Message KALIMBA_MSG_GO failed!\n"));
		Panic();
	}
	*/
}

/****************************************************************************
DESCRIPTION
	Disconnect Sync audio
*/
void CsrSbcEncoderPluginDisconnect( void ) 
{   
    uint16 i;
    
    if (!SBC)
        Panic() ;
            
  /*  StreamDisconnect(0, StreamKalimbaSink(0));

    KalimbaPowerOff() ;
  */
    for (i = 0; i < MAX_AUDIO_SINKS; i++)
	{
		if (SBC->media_sink[i] != 0)
		{										
			/* Disconnect the Kalimba source from the media sink */
            StreamDisconnect(StreamKalimbaSource(i+2), SBC->media_sink[i]);
            
            PRINT(("Audio Plugin: Disconnect media i:%d sink:0x%x\n",i,(uint16)SBC->media_sink[i]));
                
            /* clear the audio sink */	
	        SBC->media_sink[i] = 0;			            
		}
	}
        
    free (SBC);
    SBC = NULL ;        
}

/****************************************************************************
DESCRIPTION
	Indicate the volume has changed
*/
void CsrSbcEncoderPluginSetVolume( uint16 volume ) 
{
}

/****************************************************************************
DESCRIPTION
	Set the mode
*/
void CsrSbcEncoderPluginSetMode ( AUDIO_MODE_T mode , const void * params ) 
{   
	typedef struct
	{
		bool connect_sink;
		Sink media_sink;
	} sbc_mode_params;
	
	sbc_mode_params *sbc_mode = (sbc_mode_params *) params;
	uint16 i;
	
    if (!SBC)
       	Panic() ;
           
    PRINT(("Audio Plugin: Set Mode\n"));
	
	if (sbc_mode->connect_sink)
	{
		/* A second audio sink has been connected */
		for (i = 0; i < MAX_AUDIO_SINKS; i++)
		{
			if (SBC->media_sink[i] == 0)
			{
				/* store the audio sink */	
				SBC->media_sink[i] = sbc_mode->media_sink;	
						
				/* Initialise the RTP SBC encoder */
				SBC->t[i] = TransformRtpSbcEncode(StreamKalimbaSource(i+2), SBC->media_sink[i]);

				/* Configure the RTP transform to generate the selected packet size */
				TransformConfigure(SBC->t[i], VM_TRANSFORM_RTP_SBC_ENCODE_PACKET_SIZE, SBC->packet_size);

				/* Transform should not manage timings. */
				TransformConfigure(SBC->t[i], VM_TRANSFORM_RTP_SBC_ENCODE_MANAGE_TIMING, FALSE);

				/* Start the transform */
				(void) TransformStart(SBC->t[i]);	
                
                PRINT(("Audio Plugin: Start new Transform i:%d sink:0x%x\n",i,(uint16)SBC->media_sink[i]));
			}
		}
	}
	else
	{
		/* An audio sink has been disconnected */
		for (i = 0; i < MAX_AUDIO_SINKS; i++)
		{
			if (SBC->media_sink[i] == sbc_mode->media_sink)
			{										
				/* Disconnect the Kalimba source from the media sink */
                StreamDisconnect(StreamKalimbaSource(i+2), SBC->media_sink[i]);
                
                PRINT(("Audio Plugin: Disconnect media i:%d sink:0x%x\n",i,(uint16)SBC->media_sink[i]));
                
                /* clear the audio sink */	
				SBC->media_sink[i] = 0;			                
			}
		}
	}
    free(sbc_mode);
}



/****************************************************************************
DESCRIPTION
    plays a tone using the audio plugin    
*/
void CsrSbcEncoderPluginPlayTone ( audio_note * tone, Task codec_task, uint16 tone_volume , bool stereo) 
{    
    if (!SBC)
        Panic() ;
    
    PRINT(("SBC: Tone Start\n")) ;

    AUDIO_BUSY = FALSE;	
}

/****************************************************************************
DESCRIPTION
	Stop a tone from currently playing
*/
void CsrSbcEncoderPluginStopTone ( void ) 
{  
    if (!SBC)
        Panic() ;
        
    AUDIO_BUSY = FALSE;	
}

/****************************************************************************
DESCRIPTION
	Reconnects the audio after a tone has completed 
*/
void CsrSbcEncoderPluginToneComplete ( void ) 
{
    if (!SBC)
        Panic() ;
        
    AUDIO_BUSY = FALSE;	
}

    
