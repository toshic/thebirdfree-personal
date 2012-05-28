/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
*/

/*!
@file    headset_a2dp_stream_control.c
@brief    Implementation of A2DP streaming controls.
*/

/****************************************************************************
    Header files
*/


#include "headset_a2dp_stream_control.h"
#include "headset_avrcp_event_handler.h"
#include "headset_debug.h"
#include "headset_hfp_slc.h"
#include "headset_init.h"
#include "headset_statemanager.h"
#include "headset_volume.h"

#include <audio.h>
#include <bdaddr.h>


#ifdef DEBUG_A2DP_STREAM_CONTROL
    #define STREAM_DEBUG(x) DEBUG(x)    
#else
    #define STREAM_DEBUG(x) 
#endif   


/****************************************************************************
    LOCAL FUNCTION PROTOTYPES
*/



/****************************************************************************
  FUNCTIONS
*/


/**************************************************************************/
void streamControlCeaseA2dpStreaming(bool send_suspend)
{				  
	STREAM_DEBUG(("streamControlCeaseA2dpStreaming %d\n",send_suspend));
	
	MessageCancelAll(&theHeadset.task, APP_RESUME_A2DP);
	
	if (theHeadset.dsp_process == dsp_process_a2dp)
	{		
		AudioDisconnect();
		theHeadset.dsp_process = dsp_process_none;
		STREAM_DEBUG(("CeaseStreaming - disconnect audio\n"));
	
    	if (A2dpGetMediaSink(theHeadset.a2dp) && send_suspend && (stateManagerIsA2dpStreaming()))
    	{
			STREAM_DEBUG(("CeaseStreaming - suspend audio\n"));
        	A2dpSuspend(theHeadset.a2dp);
			if (!IsA2dpSourceAnAg())
			{
				if (stateManagerGetA2dpState() == headsetA2dpPaused)
				{
					/* Ensure music does not resume erroneously when the call ends */
					theHeadset.sendPlayOnConnection = FALSE;
				}
				else
				{
					/* Only send Pause if headset is currently playing music */
					if (theHeadset.features.autoSendAvrcp)
     					avrcpSendPause();
     				/* Ensure music is resumed when the call ends */
     				theHeadset.sendPlayOnConnection = TRUE;
 				}
				stateManagerEnterA2dpPausedState();
			}
    	}
	}
}


/**************************************************************************/
void streamControlConnectA2dpAudio(void)
{	
	bool result;
	AUDIO_MODE_T mode = (AUDIO_MODE_T)theHeadset.eqMode; /* Set the correct EQ mode initially */
	Task audio_plugin;
	uint32 rate = 0;
	
	STREAM_DEBUG(("streamControlConnectA2dpAudio vol index[%d] vol gain[%d]\n",theHeadset.gAvVolumeLevel, theHeadset.config->gVolLevels.volumes[theHeadset.gAvVolumeLevel].a2dpGain));
	
	audio_plugin = InitA2dpPlugin(theHeadset.seid);
	
	switch (theHeadset.a2dp_rate)
	{
	case a2dp_rate_48_000k:
		rate = 48000;
		break;
	case a2dp_rate_44_100k:
		rate = 44100;
		break;
	case a2dp_rate_32_000k:
		rate = 32000;
		break;
	case a2dp_rate_24_000k:
		rate = 24000;
		break;
	case a2dp_rate_22_050k:
		rate = 22050;
		break;
	case a2dp_rate_16_000k:
	default:
		rate = 16000;
		break;
	}
	
	if (theHeadset.dsp_process == dsp_process_sco)
		AudioDisconnect();
	
	/* The current clock mismatch must be passed to the A2DP plugin */
	theHeadset.a2dp_data.codecData.clock_mismatch = theHeadset.clock_mismatch_rate;
	
	result = AudioConnect(  audio_plugin ,	
		 				     A2dpGetMediaSink(theHeadset.a2dp) , 
		 				     AUDIO_SINK_AV ,
							 theHeadset.theCodecTask,
							 15,
						   	 rate,
						   	 theHeadset.features.mono ? FALSE : TRUE ,
		 					 mode ,  
							 (sink_codec_data_type *) &theHeadset.a2dp_data.codecData,
							 &theHeadset.task); 	
	
	theHeadset.dsp_process = dsp_process_a2dp;
}


/**************************************************************************/
void streamControlResumeA2dpStreaming(uint32 user_delay)
{				  
	STREAM_DEBUG(("streamControlResumeA2dpStreaming\n"));
	MessageCancelAll(&theHeadset.task, APP_RESUME_A2DP);
	if (!user_delay)
   		MessageSendLater(&theHeadset.task, APP_RESUME_A2DP, 0, A2DP_RESTART_DELAY);           
	else
		MessageSendLater(&theHeadset.task, APP_RESUME_A2DP, 0, user_delay);           
}


/**************************************************************************/
void streamControlCancelResumeA2dpStreaming(void)
{
	STREAM_DEBUG(("streamControlCancelResumeA2dpStreaming\n"));
	MessageCancelAll(&theHeadset.task, APP_RESUME_A2DP);
}


/**************************************************************************/
void streamControlBeginA2dpStreaming(void)
{				  
	if (HfpGetAudioSink(theHeadset.hfp_hsp) || (!A2dpGetMediaSink(theHeadset.a2dp)))
		return;
	
	STREAM_DEBUG(("streamControlBeginA2dpStreaming\n"));
	if (!stateManagerIsA2dpStreaming() && theHeadset.a2dpSourceSuspended)
	{
		STREAM_DEBUG(("Begin Streaming - start A2DP\n"));
		streamControlStartA2dp();			
	}
	
	if (stateManagerIsA2dpStreaming() && (theHeadset.dsp_process != dsp_process_a2dp))
	{
		STREAM_DEBUG(("Begin Streaming - connect audio\n"));
		streamControlConnectA2dpAudio();
	}
}


/**************************************************************************/
void streamControlStartA2dp(void)
{
	if (A2dpGetMediaSink(theHeadset.a2dp) && theHeadset.a2dpSourceSuspended && !HfpGetAudioSink(theHeadset.hfp_hsp))
    {
		STREAM_DEBUG(("streamControlStartA2dp\n"));
        A2dpStart(theHeadset.a2dp);
		if (!IsA2dpSourceAnAg() && (theHeadset.sendPlayOnConnection))
		{
			if (theHeadset.features.autoSendAvrcp)
     			avrcpSendPlay();
     		theHeadset.sendPlayOnConnection = FALSE;
 		}
		theHeadset.a2dpSourceSuspended = FALSE;
	}
}


/*****************************************************************************/
bool IsA2dpSourceAnAg(void)
{
    bdaddr bdaddr_sig, bdaddr_slc;
    
    if (!SinkGetBdAddr(A2dpGetSignallingSink(theHeadset.a2dp), &bdaddr_sig))
		return FALSE;
    if (!SinkGetBdAddr(HfpGetSlcSink(theHeadset.hfp_hsp), &bdaddr_slc))
		return FALSE;
    if (BdaddrIsSame(&bdaddr_sig, &bdaddr_slc))
        return TRUE;
    
    return FALSE;
}

