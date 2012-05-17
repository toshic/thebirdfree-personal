/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Initialisation code.
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_configure.h"
#include "audioAdaptor_aghfp_slc.h"
#include "audioAdaptor_a2dp_slc.h"
#include "audioAdaptor_avrcp_slc.h"
#include "audioAdaptor_usb_audio.h"
#include "audioAdaptor_usb_hid.h"
#include "audioAdaptor_charger.h"
#include "audioAdaptor_init.h"
#include "audioAdaptor_a2dp.h"
#include "audioAdaptor_statemanager.h"
#include "audioAdaptor_a2dp_msg_handler.h"
#include "audioAdaptor_avrcp_msg_handler.h"
#include "audioAdaptor_buttons.h"

#include <ps.h>
#include <string.h>
#include <panic.h>
#include <stdlib.h>
#include <pio.h>
#include <codec.h>
#include <charger.h>


#define NUM_SOURCE_TYPE  2 /* 0: USB, 1: ANALOG */

#ifdef USER_CONFIGURE_CODEC
    static const sep_config_type sbc_sep_usb = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 0, sizeof(sbc_caps_source_usb), sbc_caps_source_usb };
    static const sep_config_type sbc_sep_analogue = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 0, sizeof(sbc_caps_source_analogue), sbc_caps_source_analogue };

    static const sep_config_type faststream_sep_usb = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 14, sizeof(faststream_caps_source_usb), faststream_caps_source_usb };
    static const sep_config_type faststream_sep_analogue = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 14, sizeof(faststream_caps_source_analogue), faststream_caps_source_analogue };
    
    static const sep_config_type faststream_sep_bidirection_usb = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 14, sizeof(faststream_caps_bidirection_source_usb), faststream_caps_bidirection_source_usb };
    static const sep_config_type faststream_sep_bidirection_analogue = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 14, sizeof(faststream_caps_bidirection_source_analogue), faststream_caps_bidirection_source_analogue };

    #ifdef INCLUDE_MP3_ENCODER_PLUGIN
        static const sep_config_type mp3_sep_usb = { MP3_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 0, sizeof(mp3_caps_source_usb), mp3_caps_source_usb };
        static const sep_config_type mp3_sep_analogue = { MP3_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 0, sizeof(mp3_caps_source_analogue), mp3_caps_source_analogue };
    #endif
        
    #ifdef DUAL_STREAM       
        static const sep_config_type sbc_sep_ds_analogue = { SBC_DS_SEID, KALIMBA_DS_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 0, sizeof(sbc_caps_source_analogue), sbc_caps_source_analogue };       
        static const sep_config_type faststream_sep_ds_analogue = { FASTSTREAM_DS_SEID, KALIMBA_DS_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 14, sizeof(faststream_caps_source_analogue), faststream_caps_source_analogue };
    #endif
#else

    static const sep_config_type sbc_sep_usb = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 0, sizeof(sbc_caps_source_usb), sbc_caps_source_usb };
    static const sep_config_type sbc_sep_analogue = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 0, sizeof(sbc_caps_source_analogue), sbc_caps_source_analogue };

    static const sep_config_type faststream_sep_usb = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 14, sizeof(faststream_caps_source_usb), faststream_caps_source_usb };
    static const sep_config_type faststream_sep_analogue = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 14, sizeof(faststream_caps_source_analogue), faststream_caps_source_analogue };

    static const sep_config_type faststream_sep_bidirection_usb = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 14, sizeof(faststream_caps_bidirection_source_usb), faststream_caps_bidirection_source_usb };
    static const sep_config_type faststream_sep_bidirection_analogue = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 14, sizeof(faststream_caps_bidirection_source_analogue), faststream_caps_bidirection_source_analogue };
    
    #ifdef INCLUDE_MP3_ENCODER_PLUGIN
        static const sep_config_type mp3_sep_usb = { MP3_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 0, sizeof(mp3_caps_source_usb), mp3_caps_source_usb };
        static const sep_config_type mp3_sep_analogue = { MP3_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 0, sizeof(mp3_caps_source_analogue), mp3_caps_source_analogue };
    #endif
        
    #ifdef DUAL_STREAM
        static const sep_config_type sbc_sep_ds_analogue = { SBC_DS_SEID, KALIMBA_DS_RESOURCE_ID, sep_media_type_audio, a2dp_source, TRUE, 0, sizeof(sbc_caps_source_analogue), sbc_caps_source_analogue };
        static const sep_config_type faststream_sep_ds_analogue = { FASTSTREAM_DS_SEID, KALIMBA_DS_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 14, sizeof(faststream_caps_source_analogue), faststream_caps_source_analogue };
    #endif

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
    uint8 source_codec_enabled;
    uint8 i;
    
    sep_data_type seps[NUM_SEPS];
    uint8 number_of_seps = 0;
    
    if (PsRetrieve(PSKEY_CODEC_ENABLED, &source_codec_enabled, sizeof(uint8)))
    {
        the_app->a2dpCodecsEnabled = source_codec_enabled;
    }
    
#ifdef USER_CONFIGURE_CODEC
    /* Retrieve the Codec configuration from PS */
    PsRetrieve(PSKEY_SBC_CODEC_CONFIGURATION, &the_app->pskey_sbc_codec_config, sizeof(uint16));
    PsRetrieve(PSKEY_FASTSTREAM_CODEC_CONFIGURATION, &the_app->pskey_faststream_codec_config, sizeof(uint16));
    the_app->bidirect_faststream = (uint8)((the_app->pskey_faststream_codec_config & 0x1000) >> 12); 
    #ifdef INCLUDE_MP3_ENCODER_PLUGIN
        PsRetrieve(PSKEY_MP3_CODEC_CONFIGURATION, &the_app->pskey_mp3_codec_config, sizeof(uint16));
    #endif
#endif    
  
    if (the_app->a2dp_source == SourceUsb)            
    {
        seps[number_of_seps].sep_config = &sbc_sep_usb;
        seps[number_of_seps].in_use = FALSE; 
        number_of_seps++;
#ifdef DUAL_STREAM
        /* DualStream in USB mode is not supported, so Panic application.
           Turn off DualStream support in project properties to correct this issue.
        */
        Panic();
#endif
    }
    else
    {
        seps[number_of_seps].sep_config = &sbc_sep_analogue;
        seps[number_of_seps].in_use = FALSE; 
        number_of_seps++;
#ifdef DUAL_STREAM
        seps[number_of_seps].sep_config = &sbc_sep_ds_analogue;
        seps[number_of_seps].in_use = FALSE; 
        number_of_seps++;
#endif
    }
    
    for (i = 0; i < NUM_OPTIONAL_CODECS; i++)
    {
        if (the_app->a2dpCodecsEnabled & (1<<optionalCodecList[the_app->a2dp_source][i].bit))
        {
            if (a2dpSeidIsFaststream(optionalCodecList[the_app->a2dp_source][i].seid) && the_app->bidirect_faststream)
            {
                /* for bi-directional Faststream, there is a non-default configuration */
                seps[number_of_seps].sep_config = optionalCodecList[the_app->a2dp_source][i].optional_config;
            }
            else
            {
                /* set the configuration for this optional codec */
                seps[number_of_seps].sep_config = optionalCodecList[the_app->a2dp_source][i].default_config;
            }
            seps[number_of_seps].in_use = FALSE;
            number_of_seps++;
#ifdef DUAL_STREAM
            if (a2dpSeidIsFaststream(optionalCodecList[the_app->a2dp_source][i].seid))
            {
                seps[number_of_seps].sep_config = &faststream_sep_ds_analogue;
                seps[number_of_seps].in_use = FALSE; 
                number_of_seps++;
            }
#endif            
        }
    }
    
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
    uint16 i;
    
    for (i = 0; i < NUM_OPTIONAL_CODECS; i++)
    {
        if ((the_app->a2dpCodecsEnabled & (1 << (optionalCodecList[the_app->a2dp_source][i].bit)) ))
        {              
            seid_list[size_seids] = optionalCodecList[the_app->a2dp_source][i].seid;
            size_seids += 1;
#ifdef DUAL_STREAM   
            if (a2dpSeidIsFaststream(optionalCodecList[the_app->a2dp_source][i].seid))
            {
                seid_list[size_seids] = FASTSTREAM_DS_SEID;
                size_seids += 1;
            }
#endif             
        }
    }
    
    seid_list[size_seids] = SBC_SEID;
    size_seids += 1;
    
#ifdef DUAL_STREAM   
    seid_list[size_seids] = SBC_DS_SEID;
    size_seids += 1;
#endif    

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
    
    return (TaskData *)&csr_cvsd_usb_no_dsp_plugin;
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
    uint16 i;
    
    if (a2dpSeidIsSbc(seid))      
    {
        return (TaskData *)&csr_sbc_encoder_plugin;
    }
    
    for (i = 0; i < NUM_OPTIONAL_CODECS; i++)
    {
        if (optionalCodecList[the_app->a2dp_source][i].seid == seid)
            return optionalCodecList[the_app->a2dp_source][i].plugin;
        
        if (a2dpSeidIsFaststream(optionalCodecList[the_app->a2dp_source][i].seid) && a2dpSeidIsFaststream(seid))
            return optionalCodecList[the_app->a2dp_source][i].plugin;
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
 
    /* Get the input source type */
    configureGetSupportedSourceType();
    
    /* Read PS config into cache memory */
    configureGetConfigCache();
    
    /* Initialise the USB audio and HID */
    if(the_app->a2dp_source == SourceUsb)
    {
        usbAudioInit();
        usbHidInit();
    }
    else
    {
        /* Initialise the Codec Library for analogue mode */
        initCodec();
    }
    
    /* If the button's PIO line is high at boot time (now), then entry to DFU mode
       is being requested. */
    pio_get = PioGet();
    the_app->dfu_requested = ((pio_get & PIO_MFB) == PIO_MFB);
          
    if (the_app->a2dp_source == SourceUsb)
    {
        /* Get USB HID keycode configuration */
        configureHidConfig();
    }
    else
    {
        /* Setup power management of audio adaptor */
        configureGetPowerManagerConfig();

        if (chargerIsConnected())
        {
            MessageSend(&the_app->task, APP_CHARGER_CONNECTED, 0);
            the_app->chargerconnected = TRUE;
        }
    }

    /* Get the audio streaming timeout */
    the_app->audio_streaming_timeout = configureGetAudioStreamingTimeout();
    

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
    configureGetSupportedProfiles();
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
    /* Panic if no profiles have been initialised */
    if (the_app->supported_profiles == ProfileNone)
    {
        Panic();
    }
    
    /* Kick off init of profiles */
    if (the_app->supported_profiles & ProfileAghsp)
    {
        aghfpSlcInitHs();
    }
    else if (the_app->supported_profiles & ProfileAghfp)
    {
        aghfpSlcInitHf();
    }
    
    if (the_app->supported_profiles & ProfileA2dp)
    {
        initA2dp(); 
    }
    
    if (the_app->supported_profiles & ProfileAvrcp)
    {
        initAvrcp(); 
    }
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
            case ProfileAghsp:
            {
                if (!(the_app->initialised_profiles & ProfileAghsp))
                {    /* HSP has been initialised */
                    the_app->initialised_profiles |= ProfileAghsp;
                    
                    if (the_app->supported_profiles & ProfileAghfp)
                    {    /* Kick off initialisation of HFP */
                        aghfpSlcInitHf();
                    }
                }
                else
                {    /* Should never get here */
                    Panic();
                }
                break;
            }
            case ProfileAghfp:
            {
                if (!(the_app->initialised_profiles & ProfileAghfp))
                {    /* HFP has been initialised */
                    AghfpSetServiceState(the_app->aghfp, TRUE);
                    the_app->initialised_profiles |= ProfileAghfp;
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
                break;
            }
            default:
            {
                /* Should never get here */
                Panic();
                break;
            }
        }
        
        if (the_app->initialised_profiles==the_app->supported_profiles)
        {    /* All profile profiles have been initialised */
            MessageSend(&the_app->task, APP_INIT_CFM, 0);
        }
    }
    else
    {    /* Initialisation of a library failed - this should not happen so we have a major issue */
        Panic();
    }
}
