/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_cvsd_8k_cvc_1mic_headset.c

DESCRIPTION
NOTES
*/

#include <codec.h>
#include <pcm.h>
#include <stdlib.h>
#include <panic.h>
#include <voice.h>
#include <print.h>
#include <stdio.h>
#include <stream.h> /*for the audio_note*/
#include "audio_plugin_if.h"        /*for the audio_mode*/
#include "csr_common_no_dsp_if.h"      /*for things common to all CSR_COMMON_NO_DSP systems*/
#include "csr_common_no_dsp_plugin.h"
#include "csr_common_no_dsp.h"

static void DisconnectMic ( void );
static void DisconnectSpeaker ( void );
static void ConnectMic ( void );
static void ConnectSpeaker ( void ) ;

typedef struct sync_Tag
{
    unsigned volume:8 ;
    unsigned mode:8 ;
    uint32 rate ;
        /*! The audio sink being used*/
    Sink audio_sink ;
    Task codec_task ;

}SYNC_t ;

/* The task instance pointer*/
static SYNC_t * SYNC = NULL;


/****************************************************************************
NAME
    CsrCvcPluginConnect

DESCRIPTION
    This function connects cvc to the stream subsystem

RETURNS
    void
*/
void CsrNoDspPluginConnect( NoDspPluginTaskdata *task, Sink audio_sink , AUDIO_SINK_T sink_type, Task codec_task , uint16 volume , uint32 rate , bool stereo , AUDIO_MODE_T mode , const void * params )
{
    uint32 lrate = 0 ;  /* DAC rate */
  
    if (! SYNC)
    {
        SYNC = PanicUnlessNew ( SYNC_t ) ;
    }

    SYNC->codec_task = codec_task ;
    SYNC->audio_sink = audio_sink ;
    
    /* @See http://wiki/ADPCM_Software_Specification */
    switch(task->no_dsp_plugin_variant)
    {
        case(CVSD_NO_DSP):
            lrate = rate  ;
            break;
#ifdef INCLUDE_NO_DSP_AURISTREAM            
        case(AURI_2BIT_NO_DSP):
            lrate = rate * 4 ;
            if (VoiceCodecSet ( audio_sink , VOICE_CODEC_ADPCM_2BITS_PER_SAMPLE ) == 0 ) Panic() ;

            PRINT(("PCM RATE [%d] CODEC[%d]\n", (int) lrate , VOICE_CODEC_ADPCM_2BITS_PER_SAMPLE)) ;
            break;
        case(AURI_4BIT_NO_DSP):
            lrate = rate * 2 ;
            if (VoiceCodecSet ( audio_sink , VOICE_CODEC_ADPCM_4BITS_PER_SAMPLE ) == 0 ) Panic() ;

            PRINT(("PCM RATE [%d] CODEC[%d]\n", (int) lrate , VOICE_CODEC_ADPCM_4BITS_PER_SAMPLE)) ;
            break;
#endif /* INCLUDE_NO_DSP_AURISTREAM */
    }

    CodecSetOutputGainNow( codec_task, 0 , left_and_right_ch );


	StreamDisconnect(StreamPcmSource(0), StreamPcmSink(0));
	StreamDisconnect(StreamPcmSource(1), StreamPcmSink(1));

    /*clear all routing to the PCM* subsysytem*/
    PcmClearAllRouting() ;

    if (stereo)
        PcmRateAndRoute( 0, PCM_NO_SYNC, (uint32)lrate, (uint32)lrate, VM_PCM_INTERNAL_A_AND_B ) ;
    else
        PcmRateAndRoute( 0, PCM_NO_SYNC, (uint32)lrate, (uint32)lrate, VM_PCM_INTERNAL_A ) ;

    /* store rate */
    SYNC->rate = lrate;

        /*connects the speaker and mic*/
    CsrNoDspPluginSetMode ( mode , params ) ;

    CsrNoDspPluginSetVolume( volume );
 }

/****************************************************************************
NAME
    CsrCvcPluginDisconnect

DESCRIPTION
    Disconnect CSR_COMMON_NO_DSP and power off the Kalimba DSP core

RETURNS
    void
*/
void CsrNoDspPluginDisconnect( NoDspPluginTaskdata *task )
{
    CodecSetOutputGainNow( SYNC->codec_task, 0 , left_and_right_ch );

    DisconnectSpeaker() ;
    DisconnectMic() ;

    CodecSetOutputGainNow( SYNC->codec_task, SYNC->volume , left_and_right_ch );

    /*todo: is this responsible for disposing of the stream?*/
}

/****************************************************************************
NAME
    CsrCvcPluginSetVolume

DESCRIPTION
    Tell CSR_COMMON_NO_DSP to update the volume.

RETURNS
    void
*/
void CsrNoDspPluginSetVolume( uint16 volume )
{
    if (!SYNC)
        Panic() ;

    SYNC->volume = volume;

    /*Set the output Gain immediately*/
    CodecSetOutputGainNow( SYNC->codec_task, SYNC->volume , left_and_right_ch );
}

/****************************************************************************
NAME
    CsrCvcPluginSetMode

DESCRIPTION
    Set the CSR_COMMON_NO_DSP mode

RETURNS
    void
*/
void CsrNoDspPluginSetMode ( AUDIO_MODE_T mode , const void * params )
{
    SYNC->mode = mode ;

    switch (SYNC->mode)
    {
        case AUDIO_MODE_MUTE_SPEAKER     :
        {
            PRINT(("NODSP: SetMode MUTE SPEAKER\n"));
            DisconnectSpeaker();
        }
        break ;
        case AUDIO_MODE_CONNECTED :
        {
            PRINT(("NODSP: Set Mode CONNECTED\n"));
            ConnectSpeaker();
            ConnectMic();
        }
        break ;
        case AUDIO_MODE_MUTE_MIC :
        {
            PRINT(("NODSP: Set Mode MUTE MIC"));
            DisconnectMic() ;
        }
        break ;
        case AUDIO_MODE_MUTE_BOTH :
        {
            PRINT(("NODSP: Set Mode MUTE BOTH\n"));
            DisconnectMic() ;
            DisconnectSpeaker() ;
        }
        break ;
        default :
        {    /*do not send a message and return false*/
            PRINT(("NODSP: Set Mode Invalid [%x]\n" , mode )) ;
        }
        break ;
    }
}

/****************************************************************************
NAME
    CsrCvcPluginPlayTone

DESCRIPTION

    queues the tone if can_queue is TRUE and there is already a tone playing

RETURNS
    false if a tone is already playing

*/
void CsrNoDspPluginPlayTone (NoDspPluginTaskdata *task, audio_note * tone , Task codec_task , uint16 tone_volume , bool stereo)
{
    Source lSource ;
    Sink lSink = NULL ;

    if (!SYNC)
        Panic() ;

    CodecSetOutputGainNow( SYNC->codec_task, 0 , left_and_right_ch );

    if (stereo)
        PcmRateAndRoute( 0, PCM_NO_SYNC, 8000, 8000, VM_PCM_INTERNAL_A_AND_B ) ;
    else
        PcmRateAndRoute( 0, PCM_NO_SYNC, 8000, 8000, VM_PCM_INTERNAL_A ) ;

    if ( SYNC->audio_sink )
    {
        DisconnectSpeaker() ;
    }
        /* connect the tone to the DACs*/
    lSink = StreamPcmSink(0) ;
        /*request an indicaton that the tone has completed / been disconnected*/
    MessageSinkTask ( lSink ,  (TaskData*) task ) ;
        /*connect the tone*/
    lSource = StreamAudioSource ( tone ) ;
    PanicFalse( StreamConnectAndDispose( lSource , lSink ) );

    if (tone_volume)
    {
        CodecSetOutputGainNow( SYNC->codec_task, tone_volume, left_and_right_ch );
    }
    else
    {
         CodecSetOutputGainNow( SYNC->codec_task, SYNC->volume, left_and_right_ch );
    }
}

/****************************************************************************
NAME
    CsrCvcPluginStopTone

DESCRIPTION
    Stop a tone from playing

RETURNS
    whether or not the tone was stopped successfully
*/
void CsrNoDspPluginStopTone ( void )
{
    if (!SYNC)
        Panic() ;

    DisconnectSpeaker() ;

    CsrNoDspPluginSetMode ( SYNC->mode , NULL );
}


/****************************************************************************
DESCRIPTION
    Reconnects the audio after a tone has completed
*/
void CsrNoDspPluginToneForceComplete ( void )
{
    if(AUDIO_BUSY)
    {
        AUDIO_BUSY = NULL ;
        MessageSinkTask ( StreamPcmSink(0) , NULL ) ;
        DisconnectSpeaker();
        DisconnectMic();
    }
    
    /* ensure volume is set to correct level after playing tone */
    CsrNoDspPluginSetVolume( SYNC->volume );
}


/****************************************************************************
DESCRIPTION
    Reconnects the audio after a tone has completed
*/
void CsrNoDspPluginToneComplete ( NoDspPluginTaskdata *task )
{
    MessageSinkTask ( StreamPcmSink(0) , NULL ) ;
    
    if (( SYNC->mode != AUDIO_MODE_MUTE_BOTH )&&
        ( SYNC->mode != AUDIO_MODE_MUTE_SPEAKER))
    {
        /* reconnect sco audio if present */
        ConnectSpeaker();
        
        /* check to see if the sco is still valid, if it is not then we will have received the
           message before the tone has completed playing due to some other issue, therefore
           allow tone to continue playing for an additional 1.5 seconds to allow the power off
           tone to be played to completion */
        if(StreamSourceFromSink(SYNC->audio_sink))
        {
           AUDIO_BUSY = NULL ;
           CsrNoDspPluginSetVolume( SYNC->volume );
        }
        else
        {
           MessageSendLater((TaskData*) task, MESSAGE_FORCE_TONE_COMPLETE, 0, 1500);
        }
    }
    else
    {
        AUDIO_BUSY = NULL ;
        CsrNoDspPluginSetVolume( SYNC->volume );
    }

}

/****************************************************************************
DESCRIPTION
    Disconnect the microphone path
*/
static void DisconnectMic ( void )
{
    PRINT(("NODSP: Disconnect Mic\n")) ;
    StreamDisconnect(StreamPcmSource(0), SYNC->audio_sink);
}

/****************************************************************************
DESCRIPTION
    Disconnect the Speaker path
*/
static void DisconnectSpeaker ( void )
{
    PRINT(("NODSP: Disconnect Speaker\n")) ;
    StreamDisconnect(StreamSourceFromSink(SYNC->audio_sink), StreamPcmSink(0));
}

/****************************************************************************
DESCRIPTION
    Connect a SCO to the Microphone
*/
static void ConnectMic ( void )
{
    PRINT(("NODSP: Connect Mic\n")) ;
    if ( SYNC->audio_sink )
    {
		StreamDisconnect(StreamPcmSource(0), 0);
        StreamConnect(StreamPcmSource(0), SYNC->audio_sink );
    }
}

/****************************************************************************
DESCRIPTION
    Connect a SCO to the Speaker
*/
static void ConnectSpeaker ( void )
{
    PRINT(("NODSP: Connect Speaker [%x]\n" , (int)SYNC->audio_sink )) ;
    if ( SYNC->audio_sink )
    {
		StreamDisconnect(0, StreamPcmSink(0));
        StreamConnect( StreamSourceFromSink(SYNC->audio_sink) , StreamPcmSink(0));
    }
}

/****************************************************************************
NAME
    CsrNoDspPluginMicSwitch

DESCRIPTION
    Swap between the microphone inputs for production test

RETURNS
    
*/
void CsrNoDspPluginMicSwitch ( void )
{
    PRINT(("NODSP: MicSwitch [%x]\n" , (int)SYNC->audio_sink )) ;
    if ( SYNC->audio_sink )
    {
        /* disconnect mic */
        StreamDisconnect(StreamPcmSource(0), SYNC->audio_sink);
        /* disconnect speaker */
        StreamDisconnect(StreamSourceFromSink(SYNC->audio_sink), StreamPcmSink(0));
        
        CodecSetOutputGainNow( SYNC->codec_task, 0 , left_and_right_ch );

        /* clear all routing to the PCM subsystem */
        PcmClearAllRouting() ;

        PcmRateAndRoute( 0, PCM_NO_SYNC, (uint32)SYNC->rate, (uint32)SYNC->rate, VM_PCM_INTERNAL_A ) ;
        PcmRateAndRoute( 1, 0, (uint32)SYNC->rate, (uint32)SYNC->rate, VM_PCM_INTERNAL_B ) ;
        
        /*connect second mic*/
        StreamConnect(StreamPcmSource(1), SYNC->audio_sink );
        /*connect speaker*/
        StreamConnect( StreamSourceFromSink(SYNC->audio_sink) , StreamPcmSink(0));

        CodecSetOutputGainNow( SYNC->codec_task, SYNC->volume , left_and_right_ch );

    }
}

