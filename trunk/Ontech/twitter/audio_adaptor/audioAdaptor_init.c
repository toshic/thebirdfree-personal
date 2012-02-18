/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Initialisation code.
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_aghfp_slc.h"
#include "audioAdaptor_a2dp_slc.h"
#include "audioAdaptor_avrcp_slc.h"
#include "audioAdaptor_init.h"
#include "audioAdaptor_a2dp.h"
#include "audioAdaptor_statemanager.h"
#include "audioAdaptor_a2dp_msg_handler.h"
#include "audioAdaptor_avrcp_msg_handler.h"
#include "audioAdaptor_buttons.h"
#include "handle_pbap.h"
#include <csr_sco_loopback_plugin.h>
#include <csr_common_no_dsp_plugin.h>

#include <ps.h>
#include <string.h>
#include <panic.h>
#include <stdlib.h>
#include <pio.h>
#include <codec.h>
#include <charger.h>
#include <pbaps.h>


#define NUM_SOURCE_TYPE  2 /* 0: USB, 1: ANALOG */

#ifdef USER_CONFIGURE_CODEC
    static const sep_config_type sbc_sep_usb = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 0, sizeof(sbc_caps_source_usb), sbc_caps_source_usb };
    static const sep_config_type sbc_sep_analogue = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 0, sizeof(sbc_caps_source_analogue), sbc_caps_source_analogue };

    static const sep_config_type faststream_sep_usb = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 14, sizeof(faststream_caps_source_usb), faststream_caps_source_usb };
    static const sep_config_type faststream_sep_analogue = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 14, sizeof(faststream_caps_source_analogue), faststream_caps_source_analogue };
    
    static const sep_config_type faststream_sep_bidirection_usb = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 14, sizeof(faststream_caps_bidirection_source_usb), faststream_caps_bidirection_source_usb };
    static const sep_config_type faststream_sep_bidirection_analogue = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 14, sizeof(faststream_caps_bidirection_source_analogue), faststream_caps_bidirection_source_analogue };

#else

    static const sep_config_type sbc_sep_usb = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 0, sizeof(sbc_caps_source_usb), sbc_caps_source_usb };
    static const sep_config_type sbc_sep_analogue = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 0, sizeof(sbc_caps_source_analogue), sbc_caps_source_analogue };

    static const sep_config_type faststream_sep_usb = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 14, sizeof(faststream_caps_source_usb), faststream_caps_source_usb };
    static const sep_config_type faststream_sep_analogue = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 14, sizeof(faststream_caps_source_analogue), faststream_caps_source_analogue };

    static const sep_config_type faststream_sep_bidirection_usb = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 14, sizeof(faststream_caps_bidirection_source_usb), faststream_caps_bidirection_source_usb };
    static const sep_config_type faststream_sep_bidirection_analogue = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 14, sizeof(faststream_caps_bidirection_source_analogue), faststream_caps_bidirection_source_analogue };
    
#endif


typedef struct
{
    unsigned                seid:8;     /* The unique ID for the SEP. */
    unsigned                bit:8;      /* The bit position in PSKEY_CODEC_ENABLED to enable the codec. */
    const sep_config_type   *default_config;    /* The SEP config data. These configs are defined above. */
    const sep_config_type   *optional_config;    /* The SEP config data. These configs are defined above. */
    TaskData                *plugin;    /* The audio plugin to use. */
} optional_codec_type;


/* ************************************************************************************
    
    Tables which indicates which A2DP codecs are avaiable on the audio adaptors; and 
    their seid values, and corresponding plugins.

------------------------------------------------------------------------------------- */

static const optional_codec_type optionalCodecList[NUM_SOURCE_TYPE][NUM_OPTIONAL_CODECS] = 
{   
    /* USB Stream End Points */
    {
    #ifdef INCLUDE_MP3_ENCODER_PLUGIN
        {MP3_SEID, MP3_CODEC_BIT, &mp3_sep_usb, 0, (TaskData *)&csr_mp3_encoder_plugin},
    #endif
        {FASTSTREAM_SEID, FASTSTREAM_CODEC_BIT, &faststream_sep_usb, &faststream_sep_bidirection_usb, (TaskData *)&csr_faststream_source_plugin}      
    },
    
    /* Analogue Stream End Points */
    {
    #ifdef INCLUDE_MP3_ENCODER_PLUGIN
        {MP3_SEID, MP3_CODEC_BIT, &mp3_sep_analogue, 0, (TaskData *)&csr_mp3_encoder_plugin},
    #endif
        {FASTSTREAM_SEID, FASTSTREAM_CODEC_BIT, &faststream_sep_analogue, &faststream_sep_bidirection_analogue, (TaskData *)&csr_faststream_source_plugin}
    }
};



/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME 
    initA2dp

DESCRIPTION
    Initialises the A2DP profile.
    
*/
void initA2dp (void)
{
    sep_data_type seps[NUM_SEPS];
    uint8 number_of_seps = 0;

	the_app->pskey_sbc_codec_config = 0x0132;  
	the_app->sink_type = AUDIO_SINK_SCO;
	
    seps[number_of_seps].sep_config = &sbc_sep_analogue;
    seps[number_of_seps].in_use = FALSE; 
    number_of_seps++;
    
    A2dpInit(&the_app->task, A2DP_INIT_ROLE_SOURCE, NULL, number_of_seps, seps);
}


/****************************************************************************
NAME 
    initAvrcp

DESCRIPTION
    Initialises the AVRCP profile.
 
*/
void initAvrcp (void)
{
    avrcp_init_params avrcp_config;
        
    avrcp_config.device_type = avrcp_target;
            
    AvrcpInitLazy(&the_app->task, &the_app->task, &avrcp_config);
}


/****************************************************************************
NAME 
    initSeidConnectPriority

DESCRIPTION
    To decide the priority of SEIDs (Stream End Point IDs) for A2dp connection.
 
RETURNS
      The size of seid list. 
    
*/
uint16 initSeidConnectPriority(uint8 *seid_list)
{
    uint16 size_seids = 0;
    
    seid_list[size_seids] = SBC_SEID;
    size_seids += 1;
    
    return size_seids;
}


/****************************************************************************
NAME 
    initScoPlugin

DESCRIPTION
    To decide the plugin based on the configuration of PSKey.
 
RETURNS
     The plugin to be connected for SCO audio.
    
*/
Task initScoPlugin(void)
{
    /* we can add here to support more plugins */
    
/*    return (TaskData *)&sco_loopback_plugin;*/
	return (TaskData *)&csr_cvsd_usb_no_dsp_plugin;
/*	return (TaskData *)&csr_cvsd_no_dsp_plugin;*/
}


/****************************************************************************
NAME 
    initA2dpPlugin

DESCRIPTION
    To decide the plugin based on the SEID of A2dp connection.
 
RETURNS
    The plugin to be connected for audio streaming.
    
*/
Task initA2dpPlugin(uint8 seid)
{
    if (a2dpSeidIsSbc(seid))      
    {
        return (TaskData *)&csr_sbc_encoder_plugin;
    }
    
    /* No plugin found so Panic */
    Panic();
    
    return 0;
}


/****************************************************************************
NAME 
    initCodec

DESCRIPTION
    To initialise the codec employed in A2dp profile.
 
*/
void initCodec(void)
{
#ifdef ENABLE_EXTERNAL_ADC
    /* Init the Wolfson codec with default params. */
    CodecInitWolfson( &the_app->task, 0 );
#else
    CodecInitCsrInternal( &the_app->task ) ;
#endif
    
}


/****************************************************************************
  NAME 
      initUserFeatures

DESCRIPTION
     To initialise the features employed in audio adaptor.
 
*/
void initUserFeatures (void) 
{    
    uint16 pio_get;
    
    /* Read local supported features to determine data rates */
    PanicFalse(PsFullRetrieve(0x00EF, &the_app->local_supported_features, 4));
 
    /* Initialise the Codec Library for analogue mode */
    initCodec();
    
    /* If the button's PIO line is high at boot time (now), then entry to DFU mode
       is being requested. */
    pio_get = PioGet();
    the_app->dfu_requested = ((pio_get & PIO_MFB) == PIO_MFB);
          
}


/****************************************************************************
NAME 
    initApp

DESCRIPTION
    To start the application initialisation, get the supported profiles and 
    start connection initialisation.
 
*/
void initApp (void)
{
    /* Retrieve supported profiles */
    the_app->initialised_profiles = 0;
	the_app->vgs = 15;
	the_app->vgm = 10;

	strcpy(the_app->pin,"0000");
    /* Initialise the connection library */
    ConnectionInit(&the_app->task);
}
    

/****************************************************************************
NAME 
    initProfile

DESCRIPTION
    The request of profile initialisation for all supported profiles.
 
*/
void initProfile (void)
{
     aghfpSlcInitHf();
}


/****************************************************************************
NAME 
    initProfileCfm

DESCRIPTION
    The confirmation of profile initialisation for all supported profiles.
 
*/
void initProfileCfm (mvdProfiles profile, bool success)
{
    if (success)
    {
        switch (profile)
        {
             case ProfileAghfp:
            {
                if (!(the_app->initialised_profiles & ProfileAghfp))
                {    /* HFP has been initialised */
                    AghfpSetServiceState(the_app->aghfp, TRUE);
                    the_app->initialised_profiles |= ProfileAghfp;
					initA2dp(); 
                }
                else
                {    /* Should never get here */
                    Panic();
                }
                break;
            }
            case ProfileA2dp:
            {
                if (!(the_app->initialised_profiles & ProfileA2dp))
                {
                    the_app->initialised_profiles |= ProfileA2dp;
					initAvrcp(); 
                }
                else
                {    /* Should never get here */
                    Panic();
                }
                break;
            }
            case ProfileAvrcp:
            {
                if (!(the_app->initialised_profiles & ProfileAvrcp))
                {
                    the_app->initialised_profiles |= ProfileAvrcp;
                }
                else
                {    /* Should never get here */
                    Panic();
                }

				 /* All profile profiles have been initialised */
				MessageSendLater(&the_app->task, APP_INIT_CFM, 0, D_SEC(1));
				 initPbap();
				 
                break;
            }
            default:
            {
                /* Should never get here */
                Panic();
                break;
            }
        }
     }
    else
    {    /* Initialisation of a library failed - this should not happen so we have a major issue */
        Panic();
    }
}
