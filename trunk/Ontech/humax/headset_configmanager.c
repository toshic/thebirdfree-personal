/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
*/

/*!
@file    headset_configmanager.c
@brief    Configuration manager for the headset - resoponsible for extracting user information out of the PSKEYs and initialising the configurable nature of the headset components.
*/


#include "headset_auth.h"
#include "headset_configmanager.h"
#include "headset_config.h"
#include "headset_debug.h"
#include "headset_events.h"
#include "headset_led_manager.h"
#include "headset_statemanager.h"
#include "headset_private.h"

#include <bdaddr.h>
#include <csrtypes.h>
#include <ps.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <panic.h>

#ifdef DEBUG_CONFIG
#define CONF_DEBUG(x) DEBUG(x)
#else
#define CONF_DEBUG(x) 
#endif

#define HCI_PAGESCAN_INTERVAL_DEFAULT  (0x800)
#define HCI_PAGESCAN_WINDOW_DEFAULT   (0x12)
#define HCI_INQUIRYSCAN_INTERVAL_DEFAULT  (0x800)
#define HCI_INQUIRYSCAN_WINDOW_DEFAULT   (0x12)


#define DEFAULT_VOLUME_MUTE_REMINDER_TIME_SEC 10


/****************************************************************************
    LOCAL FUNCTIONS
*/
static void 	configManagerSetupSupportedFeatures	(uint16 pConfigID);
static void     configManagerTimeouts               (uint16 pConfigID);
static void     configManagerFeatures               (uint16 pConfigID);
static void     configManagerSsr                    (uint16 pConfigID);
static void 	configManagerPowerTables			(void);

/****************************************************************************
  FUNCTIONS
*/

/*****************************************************************************/
void configManagerInit(void)  
{ 
	uint16 lConfigID  = get_config_id ( PSKEY_CONFIGURATION_ID ) ;
	
		/* Read and configure the headset features */
    configManagerFeatures(lConfigID);
	
         /* Read and configure the timeouts */
    configManagerTimeouts(lConfigID);
	
    	/* Read and configure the Sniff Subrate parameters */
    configManagerSsr(lConfigID);
	
		/* Read and configure HFP features */
	configManagerSetupSupportedFeatures(lConfigID);
	
		/* Read and configure HFP and A2DP power tables */
	configManagerPowerTables();			   
}


/*****************************************************************************/ 
static void configManagerSetupSupportedFeatures(uint16 pConfigID)
{
	ConfigRetrieve( pConfigID, PSKEY_HFP_FEATURES , &theHeadset.HFP_features , sizeof( HFP_features_type ) ) ;

    {
           /*make sure the mask we have read in is within range*/
        uint16 lAllPktTypes = ( sync_hv1  | sync_hv2  |  sync_hv3 | 
                                sync_ev3  | sync_ev4  |  sync_ev5 | 
                                sync_2ev3 | sync_3ev3 | sync_2ev5 | sync_3ev5 ) ;
                                 
        uint16 lPSPktTypes = theHeadset.HFP_features.supportedSyncPacketTypes ;
                
        theHeadset.HFP_features.supportedSyncPacketTypes =  (lAllPktTypes & lPSPktTypes) ;
		CONF_DEBUG(("Co: HFPsuppFeatures[%x] HFP version[%d] packet types[%x] eSCO_Parameters_Enabled[%x] bandwidth[%lx] max_latency[%x] voice_settings[%x] retx_effort[%x]\n" , 
					theHeadset.HFP_features.HFP_Supported_Features,
					theHeadset.HFP_features.HFP_Version,
					theHeadset.HFP_features.supportedSyncPacketTypes,
					theHeadset.HFP_features.eSCO_Parameters_Enabled,
					theHeadset.HFP_features.bandwidth,
					theHeadset.HFP_features.max_latency,
					theHeadset.HFP_features.voice_settings,
					theHeadset.HFP_features.retx_effort)) ;
    }
}

/*****************************************************************************/ 
void configManagerSetupVolumeGains(uint16 pConfigID, uint16 *gains, uint16 size)
{
	/* ConfigRetrieve cannot fail since we have volume levels in the default config. */
	(void)ConfigRetrieve(pConfigID, PSKEY_VOLUME_GAINS, gains, size);
}

/*****************************************************************************/ 
void configManagerReset(void) 
{
	uint16 i;
	
    CONF_DEBUG(("CO: Reset\n")) ;
        
	theHeadset.gHfpVolumeLevel = theHeadset.config->gVolLevels.defaultHfpVolLevel ; 
   	theHeadset.gAvVolumeLevel = theHeadset.config->gVolLevels.defaultA2dpVolLevel ; 
    
    /* Delete the Connection Libs Paired Device List */
    ConnectionSmDeleteAllAuthDevices ( 0 );
	
	/* Remove devices attributes */
	for (i=0; i<MAX_PAIRED_DEVICES; i++)
		(void)PsStore ( PSKEY_ATTRIBUTE_BASE + i , 0 , 0 ) ;
}
 

/****************************************************************************
NAME 
    configManagerTimeouts

DESCRIPTION
    Read and configure the timeouts.
 
*/ 
static void configManagerTimeouts (uint16 pConfigID)
{	
    /* Get the timeout values */
	ConfigRetrieve(pConfigID, PSKEY_TIMEOUTS, &theHeadset.Timeouts, sizeof(Timeouts_t) ) ;
	
	    
    CONF_DEBUG(("Co : Pairing Timeout[%d] Mute Remind[%d] Auto Switch out[%d] Disable power off[%d] Supervision HFP[%d] Supervision A2DP[%d] Refresh Encryption timer[%d] Inquiry[%d]\n" 
											, theHeadset.Timeouts.PairModeTimeout_s
											, theHeadset.Timeouts.MuteRemindTime_s
    										, theHeadset.Timeouts.AutoSwitchOffTime_s
											, theHeadset.Timeouts.DisablePowerOffAfterPowerOnTime_s
											, theHeadset.Timeouts.LinkSupervisionTimeoutHfp_s
											, theHeadset.Timeouts.LinkSupervisionTimeoutA2dp_s
											, theHeadset.Timeouts.EncryptionRefreshTimeout_m
											, theHeadset.Timeouts.InquiryTimeout_s)) ;
}


/****************************************************************************
NAME 
    configManagerFeatures

DESCRIPTION
    Read and configure the features block.
 
*/ 
static void configManagerFeatures (uint16 pConfigID)
{	
    /* Get the timeout values */
	ConfigRetrieve(pConfigID, PSKEY_FEATURES, &theHeadset.features, sizeof(Features_t) ) ;
	
	CONF_DEBUG(("Co : Features config: Auto AVRCP[%d] MITM[%d] WAE[%d] DebugKeys[%d] ConnectActionOnPowerOn[%d] LinkLossRetries[%d] ExitPairingAction[%d] PlayHfpTonesAtFixedVolume[%d]\n" ,
                                                theHeadset.features.autoSendAvrcp ,                                               
                                                theHeadset.features.forceMitmEnabled,
                                                theHeadset.features.writeAuthEnable,
                                                theHeadset.features.debugKeysEnabled,
												theHeadset.features.ConnectActionOnPowerOn,
												theHeadset.features.LinkLossRetries,
												theHeadset.features.exitPairingModeAction,
												theHeadset.features.PlayHfpTonesAtFixedVolume
												));
	
	CONF_DEBUG(("Co : Features config: MuteToneFixedVolume[%d] QueueEventTones[%d] MuteSpeakerAndMic[%d] UseLowPowerAudioCodecs[%d] QueueLEDEvents[%d] LedTimeMultiplier[%d] UseHFPprofile[%d] UseA2DPprofile[%d] UseAVRCPprofile[%d] mono[%d] UseAVRCPforVolume[%d] UseSCOforAudioTransfer[%d] OverideMute[%d] MuteLocalVolAction[%d] DisableRoleSwitching[%d]\n" ,
                                                theHeadset.features.MuteToneFixedVolume,
												theHeadset.features.QueueEventTones,
												theHeadset.features.MuteSpeakerAndMic,
												theHeadset.features.UseLowPowerAudioCodecs,
												theHeadset.features.QueueLEDEvents,
												theHeadset.features.LedTimeMultiplier,
												theHeadset.features.UseHFPprofile,
												theHeadset.features.UseA2DPprofile,
												theHeadset.features.UseAVRCPprofile,
												theHeadset.features.mono,
												theHeadset.features.UseAVRCPforVolume,
												theHeadset.features.UseSCOforAudioTransfer,
												theHeadset.features.OverideMute,
												theHeadset.features.MuteLocalVolAction,
												theHeadset.features.DisableRoleSwitching
												));
	
	CONF_DEBUG(("Co : Features config: audio_plugin[%d] AutoPowerOnAfterInitialisation[%d] LinkPolicyA2dpSigBeatsSLC[%d] PowerOnConnectToDevices[%d] ManualConnectToDevices[%d] AutoAnswerOnConnect[%d] LNRCancelsVoiceDialIfActive[%d] EndCallWithNoSCOtransfersAudio[%d] UseHWmicBias[%d] SecurePairing[%d] DiscoIfPDLLessThan[%d]\n",
												theHeadset.features.audio_plugin,
												theHeadset.features.AutoPowerOnAfterInitialisation,
												theHeadset.features.LinkPolicyA2dpSigBeatsSLC,
												theHeadset.features.PowerOnConnectToDevices,
												theHeadset.features.ManualConnectToDevices,
												theHeadset.features.AutoAnswerOnConnect,
												theHeadset.features.LNRCancelsVoiceDialIfActive,
												theHeadset.features.EndCallWithNoSCOtransfersAudio,
												theHeadset.features.UseHWmicBias,
												theHeadset.features.SecurePairing,
												theHeadset.features.DiscoIfPDLLessThan
												));
	
	CONF_DEBUG(("Co : Features config: PairIfPDLLessThan[%d]\n",
												theHeadset.features.PairIfPDLLessThan
												));
                                                                                
}

/****************************************************************************
NAME 
    configManagerSsr

DESCRIPTION
    Read and configure the Sniff Subrate parameters.
 
RETURNS
    void
*/ 
static void configManagerSsr(uint16 pConfigID)
{
    subrate_data ssr_params;

    /* get the parameters either from PS or constant space */
    ConfigRetrieve(pConfigID, PSKEY_SSR_PARAMS, &ssr_params, sizeof(subrate_data));
    /* store them in headset for later use */
    theHeadset.config->ssr_data.streaming_params.max_remote_latency = ssr_params.streaming_params.max_remote_latency;
    theHeadset.config->ssr_data.streaming_params.min_remote_timeout = ssr_params.streaming_params.min_remote_timeout;
    theHeadset.config->ssr_data.streaming_params.min_local_timeout = ssr_params.streaming_params.min_local_timeout;
    theHeadset.config->ssr_data.signalling_params.max_remote_latency = ssr_params.signalling_params.max_remote_latency;
    theHeadset.config->ssr_data.signalling_params.min_remote_timeout = ssr_params.signalling_params.min_remote_timeout;
    theHeadset.config->ssr_data.signalling_params.min_local_timeout = ssr_params.signalling_params.min_local_timeout;

    CONF_DEBUG(("Co : SSR config: Str - MRL[%d]MRT[%d]MLT[%d] Sig - MRL[%d]MRT[%d]MLT[%d]\n",
                                theHeadset.config->ssr_data.streaming_params.max_remote_latency,
                                theHeadset.config->ssr_data.streaming_params.min_remote_timeout,
                                theHeadset.config->ssr_data.streaming_params.min_local_timeout,
                                theHeadset.config->ssr_data.signalling_params.max_remote_latency,
                                theHeadset.config->ssr_data.signalling_params.min_remote_timeout,
                                theHeadset.config->ssr_data.signalling_params.min_local_timeout));
}


/****************************************************************************
NAME 
    configManagerPowerTables

DESCRIPTION
    Read and configure the HFP and A2DP power tables.
 
RETURNS
    void
*/
static void configManagerPowerTables(void)
{
	uint16 size_table = (sizeof(lp_power_table) * MAX_POWER_TABLE_ENTRIES) + sizeof(uint16);
	power_table *PowerTable = (power_table *)PanicUnlessMalloc(size_table);
	uint16 size_ps_key = PsFullRetrieve(0x360, PowerTable, size_table);
	
	CONF_DEBUG(("Co: size_ps_key[%d] slc[%x] sco[%x] sig[%x] stream[%x]\n",size_ps_key,PowerTable->SLCentries,PowerTable->SCOentries,PowerTable->A2DPSigEntries,PowerTable->A2DPStreamEntries));
 	
    if ( size_ps_key && 
         (size_ps_key == ( (sizeof(lp_power_table)*PowerTable->SLCentries) + 
                           (sizeof(lp_power_table)*PowerTable->SCOentries) + 
						   (sizeof(lp_power_table)*PowerTable->A2DPSigEntries) +
						   (sizeof(lp_power_table)*PowerTable->A2DPStreamEntries) +
                            sizeof(uint16) ) )
       )
 	{
		memmove(theHeadset.user_power_table, PowerTable, size_ps_key);
		CONF_DEBUG(("Co: Read user power tables\n" ));
	}
	else
	{
		theHeadset.user_power_table = 0;
		CONF_DEBUG(("Co: Default power tables\n" ));
	}
	
	free(PowerTable);
}

