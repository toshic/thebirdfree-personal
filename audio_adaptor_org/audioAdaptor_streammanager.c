/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Acts on stream open and closure. 
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_events.h"
#include "audioAdaptor_streammanager.h"
#include "audioAdaptor_event_handler.h"
#include "audioAdaptor_statemanager.h"
#include "audioAdaptor_a2dp_stream_control.h"

#include <string.h>
#include <panic.h>
#include <stdlib.h>


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    streamManagerOpenNotify

DESCRIPTION
    Handle new stream open notification.
    
*/
void streamManagerOpenNotify (void)
{
    DEBUG_STREAM(("streamManagerOpenNotify AppState:(%x)\n", the_app->app_state));
    switch ( the_app->app_state )
    {
        case AppStateConnecting:
        {
            /* Ignore in these states.  The fact that a media channel has been opened */
            /* for streaming will be detected when all profiles connect.              */
            break;
        }
        case AppStateIdle:
        {
            if ((the_app->active_encoder == EncoderAv) && (the_app->audio_streaming_state == StreamingInactive) && the_app->audio_streaming_timeout)
            {    /* No USB audio, mark status as pending closure after specified timeout */
                a2dpStreamSetAudioStreamingState(StreamingPending);
            }
            setAppState(AppStateStreaming);
            kickCommAction(the_app->comm_action);
            break;
        }
        default:
        {
            DEBUG_STREAM((" - IGNORED\n"));
            break;
        }
    }
}

        
/****************************************************************************
NAME
    streamManagerOpenComplete

DESCRIPTION
    Handle new stream open completion.
    
*/
void streamManagerOpenComplete (bool status)
{
    DEBUG_STREAM(("streamManagerOpenComplete AppState:(%x)\n", the_app->app_state));
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        case AppStateStreaming:
        {
            if (status)
            {
                if ((the_app->active_encoder == EncoderAv) && (the_app->audio_streaming_state == StreamingInactive) && the_app->audio_streaming_timeout)
                {    /* No USB audio, mark status as pending closure after specified timeout */
                    a2dpStreamSetAudioStreamingState(StreamingPending);
                }
                setAppState(AppStateStreaming);
                kickCommAction(the_app->comm_action);
            }
            else
            {
                setAppState(AppStateIdle);
                eventHandleCancelCommAction();
            }
            break;
        }
        default:
        {
            DEBUG_STREAM((" - IGNORED\n"));
            break;
        }
    }
}


/****************************************************************************
NAME
    streamManagerCloseComplete

DESCRIPTION
    Handle new stream close completion.
    
*/
void streamManagerCloseComplete (void)
{
    DEBUG_STREAM(("streamManagerCloseComplete AppState:(%x)\n", the_app->app_state));
    switch ( the_app->app_state )
    {
        case AppStateStreaming:   
        {
            if ((the_app->a2dp_source == SourceUsb) && (the_app->audio_streaming_state == StreamingPending))
            {
                a2dpStreamSetAudioStreamingState(StreamingInactive);
            }
            setAppState(AppStateIdle);
            if (the_app->comm_action != CommActionStream)
                kickCommAction(the_app->comm_action);
            break;
        }
        default:
        {
            DEBUG_STREAM((" - IGNORED\n"));
            break;
        }
    }
}
