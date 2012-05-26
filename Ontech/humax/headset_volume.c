/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_volume.c
@brief    Module responsible for Vol control 
*/

#include "headset_debug.h"
#include "headset_led_manager.h"
#include "headset_pio.h"
#include "headset_volume.h"
#include "headset_statemanager.h"
#include "headset_tones.h"
#include "headset_configmanager.h"

#include <stdlib.h>
#include <audio.h>
#include <ps.h>

#ifdef DEBUG_VOLUME
#define VOL_DEBUG(x) DEBUG(x)
#else
#define VOL_DEBUG(x) 
#endif


#define MIC_VOLUME	(10)


/*****************************************************************************/
void VolumeInit ( int16 pConfigID ) 
{
	configManagerSetupVolumeGains(pConfigID, (uint16 *)&theHeadset.config->gVolLevels, sizeof(vol_table_t));

	theHeadset.gHfpVolumeLevel = theHeadset.config->gVolLevels.defaultHfpVolLevel ; 
   	theHeadset.gAvVolumeLevel = theHeadset.config->gVolLevels.defaultA2dpVolLevel ; 
	VOL_DEBUG(("VOL: Init Volume, currHFP[%d] currA2DP[%d] defHFP[%d] defA2DP[%d] maxA2DP[%d] toneVol[%d]\n",
			   theHeadset.gHfpVolumeLevel,theHeadset.gAvVolumeLevel,theHeadset.config->gVolLevels.defaultHfpVolLevel,
			   theHeadset.config->gVolLevels.defaultA2dpVolLevel,theHeadset.config->gVolLevels.maxA2dpVolLevel,theHeadset.config->gVolLevels.toneVol));
	
	theHeadset.gMuted = FALSE;
}


/*****************************************************************************/
void VolumeToggleMute ( void )
{
    VOL_DEBUG(("VOL: Mute T [%c]\n" , (theHeadset.gMuted ? 'F':'T') )) ;
    
        /*if then old state was muted*/
    if (theHeadset.gMuted )
    {
        MessageSend( &theHeadset.task , EventMuteOff , 0 ) ;
    }
    else
    {
        MessageSend( &theHeadset.task , EventMuteOn , 0 ) ;
    }
}


/*****************************************************************************/
void VolumeMuteOn ( void )
{
    VOL_DEBUG(("VOL: Mute\n")) ;    
		
	if (theHeadset.dsp_process == dsp_process_sco)
	{
		if (theHeadset.features.MuteSpeakerAndMic)
    	{
        	AudioSetMode ( AUDIO_MODE_MUTE_BOTH , NULL ) ;
    	}
    	else
    	{
        	AudioSetMode ( AUDIO_MODE_MUTE_MIC , NULL) ;
    	}
	}
    
    theHeadset.gMuted = TRUE ;
    
	if (theHeadset.Timeouts.MuteRemindTime_s)
	    MessageSendLater( &theHeadset.task , EventMuteReminder , 0 ,D_SEC(theHeadset.Timeouts.MuteRemindTime_s) ) ;
}


/*****************************************************************************/
void VolumeMuteOff ( void )
{
	AUDIO_MODE_T lMode;
	
    VOL_DEBUG(("VOL: UnMute\n")) ;        
    theHeadset.gMuted = FALSE   ;
 
    MessageCancelAll( &theHeadset.task , EventMuteReminder ) ;
	
	if ( (stateManagerGetHfpState() == headsetActiveCall) || !theHeadset.features.audio_plugin )
		lMode = AUDIO_MODE_CONNECTED ; 
	else
        lMode = AUDIO_MODE_MUTE_SPEAKER ;
    
	if (theHeadset.dsp_process == dsp_process_sco)
	{
    	AudioSetMode ( lMode , NULL ) ;
	}
}


/*****************************************************************************/
void VolumeUp ( void )
{
	uint16 actVol = 0;
	bool avAudio = FALSE;
	
    VOL_DEBUG(("VOL: VolUp\n"));

    if (!VolumeGetHeadsetVolume( &actVol, &avAudio ))
	    return;
    
	if (!avAudio)
	{
		actVol = theHeadset.config->gVolLevels.volumes[theHeadset.gHfpVolumeLevel].hfpIncVol;
		if (actVol >= VOL_MAX_VOLUME_LEVEL)
		{
			actVol = VOL_MAX_VOLUME_LEVEL;
			MessageSend ( &theHeadset.task , EventVolumeMax , 0 );
		}
	}
	else
	{
		actVol++;
		if (actVol >= theHeadset.config->gVolLevels.maxA2dpVolLevel)
		{
			actVol = theHeadset.config->gVolLevels.maxA2dpVolLevel;
			MessageSend ( &theHeadset.task , EventVolumeMax , 0 );
		}
	}
	
	VolumeSetHeadsetVolume(actVol, avAudio, TRUE );
}


/*****************************************************************************/
void VolumeDown ( void )
{
	uint16 actVol = 0;
	bool avAudio = FALSE;
	
    VOL_DEBUG(("VOL: VolDwn\n"))  ;
    
    if (!VolumeGetHeadsetVolume( &actVol, &avAudio ))
	    return;
        
	if (!avAudio)
	{
		actVol = theHeadset.config->gVolLevels.volumes[theHeadset.gHfpVolumeLevel].hfpDecVol;
	}
	else
	{
		if (actVol)
			actVol--;	
	}
	
	if (actVol == 0)
	{
		MessageSend ( &theHeadset.task , EventVolumeMin , 0 );
	}
	
	VolumeSetHeadsetVolume(actVol, avAudio, TRUE );
}


/*****************************************************************************/
void VolumeSendSettingsToAG(bool send_speaker, bool send_mic)
{
	if (theHeadset.HFP_features.HFP_Supported_Features & HFP_REMOTE_VOL_CONTROL)
	{
		if (send_speaker && theHeadset.hfp_hsp)
		{
			HfpSendSpeakerVolume(theHeadset.hfp_hsp, theHeadset.gHfpVolumeLevel);
		}

		if (send_mic && theHeadset.hfp_hsp)
		{
			HfpSendMicrophoneVolume(theHeadset.hfp_hsp, MIC_VOLUME);
		}
	}
}


/*****************************************************************************/
bool VolumeGetHeadsetVolume(uint16 * actVol, bool * avAudio)
{
	if (stateManagerIsA2dpStreaming())
    {
	    *actVol = theHeadset.gAvVolumeLevel;
	    *avAudio = TRUE;
    }
    else if (stateManagerIsHfpConnected())
    {
	    *actVol = theHeadset.gHfpVolumeLevel;
    }
    else
    {
	    VOL_DEBUG(("VOL:      No Active audio\n"));
	    return FALSE;
    }
	
	return TRUE;
}


/*****************************************************************************/
void VolumeSetHeadsetVolume(uint16 actVol, bool avAudio, bool sendVolToAg)
{
	if (avAudio)
	{
	    theHeadset.gAvVolumeLevel = actVol;
		AudioSetVolume(	theHeadset.config->gVolLevels.volumes[actVol].a2dpGain, theHeadset.theCodecTask ) ;
		VOL_DEBUG(("VOL:      Set A2DP vol index[%d] gain[%d]\n", actVol, theHeadset.config->gVolLevels.volumes[actVol].a2dpGain));
	}
	else
	{
		if (!theHeadset.gMuted || theHeadset.features.OverideMute)
		{
			if (theHeadset.gMuted)
			{
				MessageSend(&theHeadset.task, EventMuteOff, 0);
			}
		}
		else if (!theHeadset.features.MuteLocalVolAction)
		{
			return;
		}
		
	    theHeadset.gHfpVolumeLevel = actVol;
		AudioSetVolume(	theHeadset.config->gVolLevels.volumes[actVol].hfpGain, theHeadset.theCodecTask ) ;
		VOL_DEBUG(("VOL:      Set HFP vol index[%d] gain[%d]\n", actVol, theHeadset.config->gVolLevels.volumes[actVol].hfpGain));

		if (sendVolToAg)
		{
			/* Send new local volume to AG */
			VolumeSendSettingsToAG(TRUE, FALSE);	
			VOL_DEBUG(("VOL:      Update AG vol\n"));
		}
	}
	
}




