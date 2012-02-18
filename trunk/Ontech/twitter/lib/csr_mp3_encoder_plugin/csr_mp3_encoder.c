/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_mp3_encoder_usb.c
DESCRIPTION
    plugin implentation which routes USB audio though the dsp MP3 encoder
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
#include "csr_mp3_encoder.h"
#include "csr_mp3_encoder_plugin.h"

#include <stdio.h> 

#define   KALIMBA_ENCODER_SELECT			         0x7300
#define   KALIMBA_MP3ENC_UPDATE_SETTING_MESSAGE      0x7060

#define   MP3ENC_MINBITRATE_64K

typedef enum
{
	SourceUsb = 0,
	SourceAnalog
} SourceType;

typedef struct sync_Tag
{
    Sink media_sink ;    
    Task codec_task ;    
}MP3_t ;

    /*the synchronous audio data structure*/
static MP3_t * MP3 = NULL ;

static const uint16 rate_table1[] = {   0, 32,  40,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320};
static const uint16 rate_table2[] = {   0,  8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160};

/****************************************************************************
DESCRIPTION
	This function connects a synchronous audio stream to the pcm subsystem
*/ 
void CsrMp3EncoderPluginConnect( Sink audio_sink , Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params ) 
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
		unsigned isVBRSupported:1;
		unsigned mp3_sample_rate_index:3;
		unsigned mp3_bit_rate_index:4;
		uint16 packet_size;
	} mp3_codec_data_type;
	
	/* moved to plugin */
	uint16 mp3_setting; 
	uint16 mp3_rate;
	
	Transform t;
	
	mp3_codec_data_type *mp3_codecData = ( mp3_codec_data_type *) params;

	if (!mp3_codecData)
		Panic();

	/* Configure the MP3 encoder */
	if(mp3_codecData->mp3_sample_rate_index >= 3)
        mp3_rate =  rate_table2[mp3_codecData->mp3_bit_rate_index];
	else
		mp3_rate =  rate_table1[mp3_codecData->mp3_bit_rate_index];
	
	
	if(mp3_codecData->isVBRSupported)
	{
	#ifdef MP3ENC_MINBITRATE_64K
		
		if(mp3_rate >= 64)
		{
			if(mp3_codecData->mp3_sample_rate_index >= 3)
				mp3_setting = mp3_codecData->mp3_sample_rate_index + 0x10 + ((stereo) ? (0x20):(0x00)) + 0x40 + 0x0800 + 0xE000;
			else
				mp3_setting = mp3_codecData->mp3_sample_rate_index + 0x10 + ((stereo) ? (0x20):(0x00)) + 0x40 + 0x0500 + 0xE000;
		}
		else
		{
			mp3_setting = mp3_codecData->mp3_sample_rate_index + 0x10 + ((stereo) ? (0x20):(0x00)) + 0x40 + 0x0100 + 0xE000;
		}
	#else
		
		mp3_setting = mp3_codecData->mp3_sample_rate_index + 0x10 + ((stereo) ? (0x20):(0x00)) + 0x40 + 0x0100 + 0xE000;
		
	#endif
		
	}
	else
	{
		mp3_setting = mp3_codecData->mp3_sample_rate_index + 0x10 + ((stereo) ? (0x20):(0x00)) + 0x40 + (mp3_codecData->mp3_bit_rate_index << 8 ) + (mp3_codecData->mp3_bit_rate_index << 12 ) ;
	}
	
	PanicFalse(KalimbaSendMessage(KALIMBA_MP3ENC_UPDATE_SETTING_MESSAGE, mp3_setting, mp3_rate, 0, 0));
		
	
    MP3 = (MP3_t*)PanicUnlessMalloc (sizeof (MP3_t) ) ;
    
    MP3->media_sink = audio_sink ;
    MP3->codec_task = codec_task ;

	/* Initialise the RTP MP3 encoder */
	t = TransformRtpMp3Encode(StreamKalimbaSource(2), audio_sink);

	/* Configure the RTP transform to generate the selected packet size */
	TransformConfigure(t, VM_TRANSFORM_RTP_MP3_ENCODE_PACKET_SIZE, mp3_codecData->packet_size);

	/* Transform should not manage timings. */
	TransformConfigure(t, VM_TRANSFORM_RTP_MP3_ENCODE_MANAGE_TIMING, FALSE);

	/* Start the transform */
	(void) TransformStart(t);

	/* disard any data sent by the SNK */
    StreamConnectDispose(StreamSourceFromSink(audio_sink));
	
	/* select the source type */
	if(mp3_codecData->source_type == SourceUsb)
	{
		/* Select the source type */
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
			PRINT(("SBC: Message KALIMBA_MSG_GO failed!\n"));
			Panic();
		}
	}
	
    /* connect the USB with the port */	
/*	(void) StreamConnect(PanicNull(StreamUsbEndPointSource(end_point_iso_in)), StreamKalimbaSink(0));
*/
	/* Start encode */
/*    if(!KalimbaSendMessage(KALIMBA_MSG_GO,0,0,0,0))
	{
		PRINT(("MP3: Message KALIMBA_MSG_GO failed!\n"));
		Panic();
	}
*/
}

/****************************************************************************
DESCRIPTION
	Disconnect Sync audio
*/
void CsrMp3EncoderPluginDisconnect( void ) 
{   
    if (!MP3)
        Panic() ;
            
    StreamDisconnect(StreamKalimbaSource(2), 0);
            
    free (MP3);
    MP3 = NULL ;        
}

/****************************************************************************
DESCRIPTION
	Indicate the volume has changed
*/
void CsrMp3EncoderPluginSetVolume( uint16 volume ) 
{
}

/****************************************************************************
DESCRIPTION
	Set the mode
*/
void CsrMp3EncoderPluginSetMode ( AUDIO_MODE_T mode , const void * params ) 
{   
    if (!MP3)
       	Panic() ;
}



/****************************************************************************
DESCRIPTION
    plays a tone using the audio plugin    
*/
void CsrMp3EncoderPluginPlayTone ( audio_note * tone, Task codec_task, uint16 tone_volume , bool stereo) 
{    
    if (!MP3)
        Panic() ;
    
    PRINT(("MP3: Tone Start\n")) ;

    AUDIO_BUSY = FALSE;	
}

/****************************************************************************
DESCRIPTION
	Stop a tone from currently playing
*/
void CsrMp3EncoderPluginStopTone ( void ) 
{  
    if (!MP3)
        Panic() ;
        
    AUDIO_BUSY = FALSE;	
}

/****************************************************************************
DESCRIPTION
	Reconnects the audio after a tone has completed 
*/
void CsrMp3EncoderPluginToneComplete ( void ) 
{
    if (!MP3)
        Panic() ;
        
    AUDIO_BUSY = FALSE;	
}

    
