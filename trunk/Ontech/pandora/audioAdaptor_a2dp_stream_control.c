/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles the A2DP audio routing.
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_init.h"
#include "audioAdaptor_a2dp_stream_control.h"
#include "audioAdaptor_statemanager.h"
#include "audioAdaptor_a2dp_slc.h"
#include "audioAdaptor_event_handler.h"
#include "audioAdaptor_aghfp_slc.h"

#include <a2dp.h>
#include <ps.h> 
#include <file.h>
#include <kalimba.h>
#include <kalimba_standard_messages.h>
#include <string.h>
#include <panic.h>
#include <stdlib.h>
#include <transform.h>
#include <audio.h>
#include <codec.h>


/* Compile time switches */
#define MEDIA_STREAM_HOLDOFF                  /* Workaround for f/w issue where audio transmission can stall on reconnection after OOR */
#define A2DP_MEDIA_STREAM_HOLDOFF_DELAY 5000  /* F/W requires 5secs between opening of signalling and media channel */

    
const char sbc_encoder[]        = "sbc_encoder/sbc_encoder.kap";
#ifdef INCLUDE_MP3_ENCODER_PLUGIN
const char mp3_encoder[]        = "mp3_encoder/mp3_encoder.kap";
#endif
const char faststream_encoder[] = "sbc_encoder/sbc_encoder.kap";


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME 
    a2dpStreamStartDsp

DESCRIPTION
    To load the corresponding DSP kap file for the selected audio plugin, and set 
    the encoder mode.
    
    As the same dsp kap file is used for both USB source and Analogue source, it is 
    better to load the kap file in application instead of plugin library.
 
*/
void a2dpStreamStartDsp(bool isA2DPstreaming)
{    
#ifdef KAL_MSG
    uint16 codec_type = CodecSbc;
    FILE_INDEX index  = FILE_NONE;
    
    DEBUG_A2DP(("a2dpStreamStartDsp %d\n",isA2DPstreaming));

    /* Power off the kalimba first before load the Kap file again */
    KalimbaPowerOff() ;
     
    /* This will be changed if one kap file supports both usb and analogue source types */
    if (isA2DPstreaming)
    {
        Task audio_plugin = initA2dpPlugin(the_app->a2dp_active_seid);
    
        the_app->a2dp_audio_plugin = audio_plugin;
    
        /* Select the corresponding kap file */
        if ( (audio_plugin == (TaskData *)&csr_sbc_encoder_plugin) )
        {
            index = FileFind(FILE_ROOT, sbc_encoder, sizeof(sbc_encoder)-1); 
            codec_type = CodecSbc;
        }
        else if ( (audio_plugin == (TaskData *)&csr_faststream_source_plugin) )
        {
            index = FileFind(FILE_ROOT, faststream_encoder, sizeof(faststream_encoder)-1);
            
            if(the_app->bidirect_faststream)
                codec_type = CodecFaststream_bd;
            else
                codec_type = CodecFaststream;
        }
    #ifdef INCLUDE_MP3_ENCODER_PLUGIN    
        else if ( (audio_plugin == (TaskData *)&csr_mp3_encoder_plugin) )
        {
            index = FileFind(FILE_ROOT, mp3_encoder, sizeof(mp3_encoder)-1); 
            codec_type = CodecMp3;
        }
    #endif
    }
    else
    {    
        /* we should load the source type, and can not set as the default type: SourceUsb*/
        the_app->aghfp_audio_plugin  = initScoPlugin();
        
        /* Select the corresponding kap file */
        index = FileFind(FILE_ROOT, sbc_encoder, sizeof(sbc_encoder)-1);
    }

    /* Load the DSP Kap file */
    if (index == FILE_NONE)
        Panic();
    if (!KalimbaLoad(index))
        Panic();    

    DEBUG_A2DP(("KalimbaLoad %s\n",sbc_encoder));
    
    /* Register the Kalimba message */
    MessageKalimbaTask(&the_app->task);
    
    /* send the codec type to kalimba, this is necessary as codecs share kap files */
    if (codec_type != CodecMp3) 
    {
        if (!KalimbaSendMessage(KALIMBA_CODEC_TYPE_MESSAGE, codec_type,0,0,0))
        {
            Panic();
        }
    }
#else
    DEBUG_A2DP(("a2dpStreamStartDsp %d\n",isA2DPstreaming));
#endif	
}


/****************************************************************************
NAME 
    a2dpStreamSetDspIdle

DESCRIPTION
    To set the dsp to the idle state, we can not power off the dsp
    when we suspend the audio streaming under USB input.
 
*/
void a2dpStreamSetDspIdle(void)
{
    DEBUG_A2DP(("a2dpStreamSetDspIdle\n"));
}


/****************************************************************************
NAME 
    a2dpStreamConnectA2dpAudio

DESCRIPTION
    To connnect the audio plugin for a2dp audio streaming;
 
*/
void a2dpStreamConnectA2dpAudio(Sink media_sink)
{
    uint16 i, counter = 0;
    Sink media_sink_arr[MAX_NUM_DEV_CONNECTIONS];
    
    DEBUG_A2DP(("a2dpStreamConnectA2dpAudio\n"));
    
    MessageCancelAll(&the_app->task, APP_CONNECT_A2DP_AUDIO);
    
    if (the_app->active_encoder == EncoderAv)
    {
        /* This indicates streaming is already active and now a second audio connection has
           to be routed. Use the AudioSetMode function to start the second audio stream. 
        */
        dualstream_mode_params *dual_mode = (dualstream_mode_params *)PanicUnlessMalloc(sizeof(dualstream_mode_params));
        dual_mode->connect_sink = TRUE; /* Indicate the audio is being connected */
        dual_mode->media_sink = media_sink; /* Supply the second media sink that is being connected */
        AudioSetMode(AUDIO_MODE_CONNECTED, (void*)dual_mode);

        DEBUG_A2DP(("AudioSetMode connect_sink:0x%x \n",(uint16)media_sink));
    }
    else
    {
        /* No audio is currently active, so route the audio for all active streams */
        for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
        {
            if ((the_app->dev_inst[i] != NULL) && (the_app->dev_inst[i]->a2dp_state == A2dpStateStreaming))
            {
                DEBUG_A2DP(("*** streaming found i:%d a2dp:0x%x sink:0x%x ***\n",i,(uint16)the_app->dev_inst[i]->a2dp, (uint16)the_app->dev_inst[i]->a2dp_media_sink));
                media_sink_arr[counter++] = the_app->dev_inst[i]->a2dp_media_sink;
            }
        }
    
        /* Pass the second media sink as a parameter into the AudioConnect function that the audio plugin
           can recognise. 
        */
        if (counter > 1)
        {
            the_app->a2dp_data.codecData.media_sink_b = media_sink_arr[1];
        }
        else
        {
            the_app->a2dp_data.codecData.media_sink_b = 0;
        }
        
        /* Connect the audio plugin */
        AudioConnect(the_app->a2dp_audio_plugin , 
                 media_sink_arr[0] , 
                 AUDIO_SINK_AV ,
                 the_app->codecTask,
                 0x0a , 
                 the_app->a2dp_sample_rate ,  
                 (TRUE), 
                 AUDIO_MODE_CONNECTED, 
                 (source_codec_data_type *) &the_app->a2dp_data.codecData ) ;
        
        the_app->active_encoder = EncoderAv;
        
        DEBUG_A2DP(("AudioConnect devices:%d\n",counter));
    }
}


/****************************************************************************
NAME 
    a2dpStreamStartA2dp

DESCRIPTION
    To start A2dp streaming for any open media channels.
 
*/
void a2dpStreamStartA2dp(void)
{
    devInstanceTaskData *inst;
    uint16 i;
    
    DEBUG_A2DP(("a2dpStreamStartA2dp\n"));
    
    /* Attempt to start streaming on all devices with open media channel */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {
            if (inst->a2dp != NULL)
            {
                switch (getA2dpState(inst))
                {
                    case A2dpStateOpen:
                    {
        #if defined MEDIA_STREAM_HOLDOFF
                        if (!the_app->a2dp_media_stream_holdoff)
                        {
                            setA2dpState(inst, A2dpStateStarting);
                            A2dpStart(inst->a2dp);
                        }
                        else
                        {
                            the_app->a2dp_media_stream_requested = TRUE;
                        }
        #else
                        setA2dpState(inst, A2dpStateStarting);
                        A2dpStart(inst->a2dp);
        #endif
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }
}


/****************************************************************************
NAME 
    a2dpStreamCeaseA2dpStreaming

DESCRIPTION
    To stop A2dp streaming for any media channels currently streaming.
 
*/
void a2dpStreamCeaseA2dpStreaming(void)
{
    devInstanceTaskData *inst;
    uint16 i;
    
    DEBUG_A2DP(("a2dpStreamCeaseA2dpStreaming\n"));
    
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {
            if (inst->a2dp != NULL)
            {
                switch (getA2dpState(inst))
                {
                    case A2dpStateStreaming:
                    {                                              
                        setA2dpState(inst, A2dpStateSuspending);
                        A2dpSuspend(inst->a2dp);
                        break;
                    }                    
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }
    
    /* Stop the audio processing if currently handling A2DP */
    if (the_app->active_encoder == EncoderAv)
    {
         AudioDisconnect();
         the_app->active_encoder = EncoderNone;
         a2dpStreamSetDspIdle();
         DEBUG_A2DP(("AudioDisconnect\n"));
    }                        
}


/****************************************************************************
NAME 
    a2dpStreamStartMediaStreamHoldoff

DESCRIPTION
    Start the media stream holdoff timer, during which no streaming is initiated.
 
*/
#if defined MEDIA_STREAM_HOLDOFF
void a2dpStreamStartMediaStreamHoldoff (void)
{
    DEBUG_A2DP(("a2dpStreamStartMediaStreamHoldoff\n"));
    the_app->a2dp_media_stream_holdoff = TRUE;
    MessageCancelAll(&the_app->task, APP_A2DP_MEDIA_STREAM_HOLDOFF);
    MessageSendLater(&the_app->task, APP_A2DP_MEDIA_STREAM_HOLDOFF, 0, A2DP_MEDIA_STREAM_HOLDOFF_DELAY);
}
#endif


/****************************************************************************
NAME 
    a2dpStreamStopMediaStreamHoldoff

DESCRIPTION
    Stops the media stream holdoff timer, so streaming can be initiated.
 
*/
#if defined MEDIA_STREAM_HOLDOFF
void a2dpStreamStopMediaStreamHoldoff (void)
{
    DEBUG_A2DP(("a2dpStreamStopMediaStreamHoldoff\n"));
    the_app->a2dp_media_stream_requested = FALSE;    /* Only ever used by media stream hold off timer */
    the_app->a2dp_media_stream_holdoff = FALSE;
    MessageCancelAll(&the_app->task, APP_A2DP_MEDIA_STREAM_HOLDOFF);
}
#endif


/****************************************************************************
NAME
    a2dpStreamSetAudioStreamingState

DESCRIPTION
    Sets a new audio streaming state.
    
*/
void a2dpStreamSetAudioStreamingState (mvdStreamingState streaming_state)
{
    DEBUG_EVENT(("a2dpStreamSetAudioStreamingState(%u)\n", (uint16)streaming_state));
    
    /* Set new streaming state and always cancel any outstanding timer messages */
    the_app->audio_streaming_state = streaming_state;
    MessageCancelAll(&the_app->task, APP_AUDIO_STREAMING_TIMER);
}


/****************************************************************************
NAME
    restartAudioStream

DESCRIPTION
    Kick off the creation of streaming or call audio if it is required.
    
*/
void a2dpStreamRestartAudioStream(void)
{
    if (a2dpSlcIsMediaOpen())
    {
        if ((the_app->audio_streaming_state != StreamingActive))                        
        {    
            /* Ensure an A2dp media channel is opened on connection */
            a2dpStreamSetAudioStreamingState(StreamingPending);
        }
        if (the_app->audio_streaming_state != StreamingInactive)
        {
             /* Audio streaming detected during connection, open an audio connection */
             eventHandleSendKickCommActionMessage(CommActionStream); 
        }
    }
    if (((aghfpSlcGetConnectedHF()) != NULL) && the_app->voip_call_active)
    {
        /* Voip call detected during connection, open a call */
        eventHandleSendKickCommActionMessage(CommActionCall); 
    }
}
