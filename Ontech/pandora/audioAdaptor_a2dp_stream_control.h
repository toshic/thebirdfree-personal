/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles the A2DP audio routing.
*/

#ifndef AUDIOADAPTOR_A2DP_STREAM_CTL_H
#define AUDIOADAPTOR_A2DP_STREAM_CTL_H

#include "audioAdaptor_private.h"


void a2dpStreamStartDsp(bool isA2DPstreaming);

void a2dpStreamSetDspIdle(void);

void a2dpStreamConnectA2dpAudio (Sink media_sink);

void a2dpStreamStartA2dp(void);

void a2dpStreamCeaseA2dpStreaming(void);

void a2dpStreamStartMediaStreamHoldoff (void);

void a2dpStreamStopMediaStreamHoldoff (void);

void a2dpStreamSetAudioStreamingState (mvdStreamingState streaming_state);

void a2dpStreamRestartAudioStream(void);


#endif /* AUDIOADAPTOR_A2DP_STREAM_CTL_H */
