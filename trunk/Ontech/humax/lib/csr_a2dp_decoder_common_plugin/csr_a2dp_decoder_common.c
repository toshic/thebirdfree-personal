/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_decoder.c
DESCRIPTION
    plugin implentation which routes the sco audio though the dsp
NOTES
*/

#include <audio.h>
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
#include <ps.h>
#include <Transform.h>
#include <string.h>

#include "audio_plugin_if.h" /*for the audio_mode*/
#include "csr_a2dp_decoder_common_plugin.h"
#include "csr_a2dp_decoder_common.h"
#include "csr_a2dp_decoder_common_if.h"

static void MusicConnectAudio (A2dpPluginTaskdata *task, bool stereo );

/* dsp message structure*/
typedef struct
{
    uint16 id;
    uint16 a;
    uint16 b;
    uint16 c;
    uint16 d;
} DSP_REGISTER_T;

typedef struct sync_Tag
{
    Sink media_sink ;
    Task codec_task ;
    /*! mono or stereo*/
    unsigned stereo:1;
    /*! The current mode */
    unsigned mode:8 ;
    /*! The current volume level*/
    uint16 volume ;
    uint16 params;
    uint32 rate;
	Task app_task;
}DECODER_t ;

typedef struct
{
	uint8 content_protection;
	uint32 voice_rate;
	unsigned bitpool:8;
	unsigned format:8;
	uint16 clock_mismatch;
} plugin_codec_data_type;

    /*the synchronous audio data structure*/
static DECODER_t * DECODER = NULL ;

static bool pskey_read = FALSE;
static uint16 val_pskey_max_mismatch = 0;
static uint16 val_clock_mismatch = 0;

/*  The following PS Key can be used to define a non-default maximum clock mismatch between SRC and SNK devices.
    If the PS Key is not set, the default maximum clock mismatch value will be used.
    The default value has been chosen to have a very good THD performance and to avoid audible pitch shifting
    effect even during harsh conditions (big jitters, for example). While the default covers almost all phones
    and other streaming sources by a big margin, some phones could prove to have a larger percentage clock drift.
*/
#define PSKEY_MAX_CLOCK_MISMATCH    0x2258 /* PSKEY_DSP0 */


/****************************************************************************
DESCRIPTION
	This function connects a synchronous audio stream to the pcm subsystem
*/
void CsrA2dpDecoderPluginConnect( A2dpPluginTaskdata *task, Sink audio_sink , Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params , Task app_task )
{
	FILE_INDEX index = FILE_NONE;
	char* kap_file = NULL ;

	/* Only need to read the PS Key value once */
    if (!pskey_read)
    {
        if (PsFullRetrieve(PSKEY_MAX_CLOCK_MISMATCH, &val_pskey_max_mismatch, sizeof(uint16)) == 0)
            val_pskey_max_mismatch = 0;
        pskey_read = TRUE;
    }

    switch ((A2DP_DECODER_PLUGIN_TYPE_T)task->a2dp_plugin_variant)
	{
	case SBC_DECODER:
		kap_file = "sbc_decoder/sbc_decoder.kap";
      break;
	case MP3_DECODER:
		kap_file = "mp3_decoder/mp3_decoder.kap";
		break;
	case AAC_DECODER:
		kap_file = "aac_decoder/aac_decoder.kap";
		break;
	case FASTSTREAM_SINK:
		kap_file = "faststream_sink/faststream_sink.kap";
		break;
	default:
		Panic();
		break;
	}


   /*ensure that the messages received are from the correct kap file*/
   (void) MessageCancelAll( (TaskData*) task, MESSAGE_FROM_KALIMBA);
   MessageKalimbaTask( (TaskData*) task );


	index = FileFind(FILE_ROOT,(const char *) kap_file ,strlen(kap_file));

	if (index == FILE_NONE)
		Panic();
	if (!KalimbaLoad(index))
		Panic();

    DECODER = (DECODER_t*)PanicUnlessMalloc (sizeof (DECODER_t) ) ;

    DECODER->media_sink = audio_sink ;
    DECODER->codec_task = codec_task ;
    DECODER->volume     = volume;
    DECODER->mode       = mode;
    DECODER->stereo     = stereo;
    DECODER->params     = (uint16) params;
    DECODER->rate       = rate;
    DECODER->app_task	= app_task;

	if ((A2DP_DECODER_PLUGIN_TYPE_T)task->a2dp_plugin_variant == AAC_DECODER)
	{
		/* Workaround for AAC+ sources that negotiate sampling frequency at half the actual value */
		if (rate < 32000)
			DECODER->rate = rate * 2;
	}

   CodecSetOutputGainNow(DECODER->codec_task, 0, left_and_right_ch);

   StreamDisconnect(StreamPcmSource(0), StreamPcmSink(0));
   StreamDisconnect(StreamPcmSource(1), StreamPcmSink(1));

   PanicFalse(PcmClearRouting(0));
   PanicFalse(PcmClearRouting(1));

   /* For sinks disconnect the source in case its currently being disposed. */
   StreamDisconnect(StreamSourceFromSink(audio_sink), 0);

	PRINT(("DECODER: CsrA2dpDecoderPluginConnect completed\n"));
}

/****************************************************************************
DESCRIPTION
	Disconnect Sync audio
*/
void CsrA2dpDecoderPluginDisconnect( void )
{
    if (!DECODER)
        Panic() ;

        /*disconnect the pcm streams*/
    StreamDisconnect(StreamPcmSource(0), StreamPcmSink(0));
	StreamDisconnect(StreamPcmSource(1), StreamPcmSink(1));

	   /* For sinks disconnect the source in case its currently being disposed. */
    StreamDisconnect(StreamSourceFromSink(DECODER->media_sink ), 0);
    StreamConnectDispose (StreamSourceFromSink(DECODER->media_sink)) ;

    KalimbaPowerOff() ;

    free (DECODER);
    DECODER = NULL ;
}

/****************************************************************************
DESCRIPTION
	Indicate the volume has changed
*/
void CsrA2dpDecoderPluginSetVolume(A2dpPluginTaskdata *task, uint16 volume )
{
    if (volume > 0xf)
      volume = 0xf;
    if (volume < 0)
       volume = 0;
      
    KalimbaSendMessage(MUSIC_VOLUME_MSG, 0, 0, volume, volume);      
}


/****************************************************************************
DESCRIPTION
	Sets the audio mode
*/
void CsrA2dpDecoderPluginSetMode ( AUDIO_MODE_T mode , A2dpPluginTaskdata *task , const void * params )
{
    if (!DECODER)
       	Panic() ;

    DECODER->mode = mode;

    PRINT(("DECODER: Set Mode\n"));

	 switch (mode)
    {
		case AUDIO_MODE_MUTE_SPEAKER:
	   	{
         KalimbaSendMessage (MUSIC_SETMODE_MSG , MUSIC_SYSMODE_FULLPROC , 1, 0, 0);
			PRINT(("DECODER: Set Mode SYSMODE_FULLPROC eq 2\n"));
       	}
	   	break ;
	   	case AUDIO_MODE_CONNECTED:
       	{
         KalimbaSendMessage (MUSIC_SETMODE_MSG , MUSIC_SYSMODE_PASSTHRU , 0, 0, 0 ) ;
   	   PRINT(("DECODER: Set Mode SYSMODE_PASSTHRU\n"));
       	}
	   	break ;
	      case AUDIO_MODE_MUTE_MIC:
	   	{
       	KalimbaSendMessage (MUSIC_SETMODE_MSG , MUSIC_SYSMODE_FULLPROC , 0, 0, 0);
			PRINT(("DECODER: Set Mode SYSMODE_FULLPROC eq 1\n"));
	   	}
	   	break ;
	   	case AUDIO_MODE_MUTE_BOTH:
    	{
      	KalimbaSendMessage (MUSIC_SETMODE_MSG , MUSIC_SYSMODE_FULLPROC , 2, 0, 0);
		/*	PRINT(("DECODER: Set Mode SYSMODE_FULLPROC eq 3\n"));*/
    	}
    	break;
    	default:
    	{
    		PRINT(("DECODER: Set Mode Invalid [%x]\n" , mode ));
		}
		break ;
   	}
}


/****************************************************************************
DESCRIPTION
    plays a tone using the audio plugin
*/
void CsrA2dpDecoderPluginPlayTone ( A2dpPluginTaskdata *task, audio_note * tone, Task codec_task, uint16 tone_volume , bool stereo)
{
    Source lSource ;
    Sink lSink ;

    if (!DECODER)
        Panic() ;

    PRINT(("DECODER: Tone Start\n")) ;

    lSink = StreamKalimbaSink(3) ;

    /*request an indication that the tone has completed / been disconnected*/
    MessageSinkTask ( lSink , (TaskData*) task ) ;

    /*connect the tone*/
    lSource = StreamAudioSource ( (const audio_note *) (tone) ) ;
    
    /*mix the tone to the SBC*/
    StreamConnectAndDispose( lSource , lSink ) ;

}

/****************************************************************************
DESCRIPTION
	Stop a tone from currently playing
*/
void CsrA2dpDecoderPluginStopTone ( void )
{
    if (!DECODER)
        Panic() ;

     StreamDisconnect ( 0 , StreamKalimbaSink(3) ) ;
}

/****************************************************************************
DESCRIPTION
	Reconnects the audio after a tone has completed
*/
void CsrA2dpDecoderPluginToneComplete ( void )
{
    /*we no longer want to receive stream indications*/
    MessageSinkTask ( StreamKalimbaSink(3) , NULL) ;
}

/****************************************************************************
DESCRIPTION
	handles the internal cvc messages /  messages from the dsp
*/
void CsrA2dpDecoderPluginInternalMessage( A2dpPluginTaskdata *task ,uint16 id , Message message )
{
	switch(id)
	{
        case MESSAGE_FROM_KALIMBA:
		{
			const DSP_REGISTER_T *m = (const DSP_REGISTER_T *) message;
	        PRINT(("DECODER: msg id[%x] a[%x] b[%x] c[%x] d[%x]\n", m->id, m->a, m->b, m->c, m->d));

            switch ( m->id )
			{
              case MUSIC_READY_MSG:
                {
					if (DECODER)
					{
                    	KalimbaSendMessage(MUSIC_LOADPARAMS_MSG, MUSIC_PS_BASE, 0, 0, 0);

                    	/*A2dp is now loaded, signal that tones etc can be scheduled*/
                    	AUDIO_BUSY = NULL ;

                    	PRINT(("DECODER: DECODER_READY \n"));

                     CsrA2dpDecoderPluginSetMode(DECODER->mode, task, 0);

                    	MusicConnectAudio (task, DECODER->stereo);
					}
                }
                break;
			    case MUSIC_CODEC_MSG:
	            {
                    uint16 lOutput_gain_l = m->a;
                    uint16 lOutput_gain_r = m->b;

					if (DECODER)
					{
                    	CodecSetOutputGainNow(DECODER->codec_task, lOutput_gain_l, left_ch);
                    	CodecSetOutputGainNow(DECODER->codec_task, lOutput_gain_r, right_ch);
					}
	            }
                break;
				case KALIMBA_MSG_SOURCE_CLOCK_MISMATCH_RATE:
				{
					MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_DSP_MSG);
					message->id = KALIMBA_MSG_SOURCE_CLOCK_MISMATCH_RATE;
					message->value = m->a;
					MessageSend(DECODER->app_task, AUDIO_PLUGIN_DSP_MSG, message);
					break;
				}
            }
		}
		break;

        default:
        break ;
	}
}

/****************************************************************************
DESCRIPTION
	Connect the encoded audio input and pcm audio output streams
*/
static void MusicConnectAudio (A2dpPluginTaskdata *task, bool stereo )
{
   plugin_codec_data_type *codecData = (plugin_codec_data_type *) DECODER->params;
   uint32 voice_rate = 0;
   
   val_clock_mismatch = codecData->clock_mismatch;
   
   if ((A2DP_DECODER_PLUGIN_TYPE_T)task->a2dp_plugin_variant == FASTSTREAM_SINK)
	{
      /*
         FastStream does not use RTP.
         L2CAP frames enter/leave via port 2
      */
      
    	/*
         Initialise PCM.
         Output stereo at 44k1Hz or 48kHz, input from left ADC at 16kHz.
      */
      
      /* If no voice rate is set just make the ADC rate equal to 16kHz */
      if (!codecData->voice_rate)
         voice_rate = 16000;
      else
         voice_rate = codecData->voice_rate;
      
      PRINT(("FASTSTREAM: rate=0x%lx voice_rate=0x%lx\n format=0x%x bitpool=0x%x",DECODER->rate,codecData->voice_rate,codecData->format,codecData->bitpool));
      PanicFalse(PcmRateAndRoute(0, PCM_NO_SYNC, (uint32)DECODER->rate, (uint32) voice_rate, VM_PCM_INTERNAL_A));     
      
      /* is it mono playback? */
      if ( !stereo )
      {
         PRINT(("DECODER: Mono\n"));
         /* Connect Kalimba to PCM */
         if (DECODER->rate)
         {
            StreamDisconnect(StreamSourceFromSink(DECODER->media_sink), 0);
            PanicFalse(StreamConnect(StreamKalimbaSource(0),StreamPcmSink(0)));
            PanicFalse(StreamConnect(StreamSourceFromSink(DECODER->media_sink),StreamKalimbaSink(2)));
         }
      }
      else
      {
         PRINT(("DECODER: Stereo\n"));
         PanicFalse(PcmRateAndRoute(1, 0, (uint32)DECODER->rate, (uint32) voice_rate, VM_PCM_INTERNAL_B));
         /* Connect Kalimba to PCM */
         if (DECODER->rate)
         {
            StreamDisconnect(StreamSourceFromSink(DECODER->media_sink), 0);
            PanicFalse(StreamConnect(StreamKalimbaSource(0),StreamPcmSink(0)));
            PanicFalse(StreamConnect(StreamKalimbaSource(1),StreamPcmSink(1)));
            PanicFalse(StreamConnect(StreamSourceFromSink(DECODER->media_sink),StreamKalimbaSink(2)));
         }
      }

      if (codecData->voice_rate)
      {
         StreamDisconnect(0, DECODER->media_sink);
         /* configure parameters */
         PanicFalse(KalimbaSendMessage(KALIMBA_MSG_SBCENC_SET_PARAMS, codecData->format, 0, 0, 0));
         PanicFalse(KalimbaSendMessage(KALIMBA_MSG_SBCENC_SET_BITPOOL, codecData->bitpool, 0, 0, 0));
         PanicFalse(StreamConnect(StreamPcmSource(0), StreamKalimbaSink(0)));
         PanicFalse(StreamConnect(StreamKalimbaSource(2),DECODER->media_sink));
      }
     
   }
   
   else /* Not FastStream CODEC */
	{
		uint8 content_protection = codecData->content_protection;
		uint16 scms_enabled = 0;
		Transform rtp_transform = 0;

		switch ((A2DP_DECODER_PLUGIN_TYPE_T)task->a2dp_plugin_variant)
		{
		case SBC_DECODER:
			rtp_transform = TransformRtpSbcDecode(StreamSourceFromSink(DECODER->media_sink) , StreamKalimbaSink(0));
			break;
		case MP3_DECODER:
			rtp_transform = TransformRtpMp3Decode(StreamSourceFromSink(DECODER->media_sink) , StreamKalimbaSink(0));
			break;
		case AAC_DECODER:
			rtp_transform = TransformRtpAacDecode(StreamSourceFromSink(DECODER->media_sink) , StreamKalimbaSink(0));
			break;
		default:
			break;
		}

		/* Configure the content protection */
		if (content_protection)
			scms_enabled = 1;

		TransformConfigure(rtp_transform, VM_TRANSFORM_RTP_SCMS_ENABLE, scms_enabled);

		/*start the transform decode*/
    	(void)TransformStart( rtp_transform ) ;   

    	/* is it mono playback? */
		if ( !stereo )
		{
			PcmRateAndRoute(0, PCM_NO_SYNC, DECODER->rate, (uint32) 8000, VM_PCM_INTERNAL_A) ;
			PRINT(("DECODER: Mono\n"));
			/* plug port 0 into both DACs */
    		(void) PanicFalse(StreamConnect(StreamKalimbaSource(0),StreamPcmSink(0)));
		}
		else
		{
	    	PRINT(("DECODER: Stereo\n"));

	    	PanicFalse(PcmRateAndRoute(0, PCM_NO_SYNC, DECODER->rate, (uint32) 8000, VM_PCM_INTERNAL_A));
			PanicFalse(PcmRateAndRoute(1, 0, DECODER->rate, (uint32) 8000, VM_PCM_INTERNAL_B));

		    /* plug port 0 into Left DAC */
    		PanicFalse(StreamConnect(StreamKalimbaSource(0),StreamPcmSink(0)));
        	/* plug port 1 into Right DAC */
			PanicFalse(StreamConnect(StreamKalimbaSource(1),StreamPcmSink(1)));
		}
   }
         
    	CsrA2dpDecoderPluginSetVolume(task,DECODER->volume) ;

		/* The DSP must know the sample rate for tone mixing */
		KalimbaSendMessage(MESSAGE_SET_SAMPLE_RATE, DECODER->rate, val_pskey_max_mismatch, val_clock_mismatch, 0);
      PRINT(("DECODER: Send Go message to DSP now\n"));
		if(!KalimbaSendMessage(KALIMBA_MSG_GO,0,0,0,0))
		{
			PRINT(("DECODER: Message KALIMBA_MSG_GO failed!\n"));
			Panic();
		}
      
   }
