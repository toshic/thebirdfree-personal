/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles messages received from the A2DP library.
*/


#include "audioAdaptor_private.h"
#include "audioAdaptor_init.h"
#include "audioAdaptor_profile_slc.h"
#include "audioAdaptor_streammanager.h"
#include "audioAdaptor_a2dp.h"
#include "audioAdaptor_a2dp_slc.h"
#include "audioAdaptor_a2dp_stream_control.h"
#include "audioAdaptor_a2dp_msg_handler.h"
#include "audioAdaptor_dev_instance.h"
#include "audioAdaptor_event_handler.h"
#include "audioAdaptor_statemanager.h"

#include <ps.h>
#include <a2dp.h>
#include <string.h>
#include <panic.h>
#include <stdlib.h>
#include <audio.h>
#include <codec.h>
#include <kalimba.h>

#define POS_CODEC_SUPPORT_VBR                 7
#define POS_CODEC_SAMPLE_RATE_INDEX           4

#define NUM_SBC_SAMPLE_RATE_SUPPORTED         4
#define NUM_FASTSTREAM_SAMPLE_RATE_SUPPORTED  3
#define NUM_MP3_SAMPLE_RATE_SUPPORTED         6
#define NUM_MP3_BIT_RATE_INDEX_SUPPORTED      15 

#define NUM_CHANNEL_MODE_SUPPORTED            4

#define MAX_A2DP_REOPEN_TRIES                 1

/* Lower power table for the A2DP connection. */
static const lp_power_table a2dp_active[] =
{
    /* mode,        min_interval, max_interval, attempt, timeout, duration */
    {lp_active,        0,              0,            0,         0,        10},    /* Active mode for 10 sec */
    {lp_passive,       0,              0,            0,         0,        4},    /* Passive mode for 4 secs */
    {lp_sniff,        32,            200,            1,         8,        0}     /* Enter sniff mode (20-125mS) */
};

static const uint8 channel_mode_mask[]     = {0xf1, 0xf2, 0xf4, 0xf8};

static const uint8 sample_rate_mask_sbc[]  = {0x1f, 0x2f, 0x4f, 0x8f};
static const uint8 sample_rate_mask_fs[]   = {0x21, 0x22, 0x20};
static const uint8 sample_rate_mask_mp3[]  = {0xc1, 0xc2, 0xc4, 0xc8, 0xd0, 0xe0};

static const uint8 cap6_table_mp3[] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x81, 0x82, 0x84, 0x88, 0x90, 0xa0, 0xc0};
static const uint8 cap7_table_mp3[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* As the Index in A2dp library is different from that for MP3 encoder Kalimba Library */
static const uint8 sample_rate_index_table_mp3[]  = {1, 0, 2, 4, 3, 5};

/* The max bitpools for the different audio qualities */
static const uint16 max_bitpool_array[] = {SBC_BITPOOL_LOW_QUALITY, SBC_BITPOOL_MEDIUM_QUALITY, SBC_BITPOOL_GOOD_QUALITY, SBC_BITPOOL_HIGH_QUALITY};
/* The max bitpools for the different audio qualities under pool link conditions */
static const uint16 max_bitpool_poor_link_array[] = {SBC_BITPOOL_LOW_QUALITY-10, SBC_BITPOOL_MEDIUM_QUALITY, SBC_BITPOOL_GOOD_QUALITY-15, SBC_BITPOOL_HIGH_QUALITY};


/****************************************************************************
  LOCAL FUNCTIONS
*/
      
/****************************************************************************
NAME
    a2dpUnhandledMessage

DESCRIPTION
    For debug purposes, so any unhandled A2DP messages are discovered.     

*/
static void a2dpUnhandledMessage(devInstanceTaskData *theInst, uint16 id)
{
    DEBUG_A2DP(("UNHANDLED A2DP MESSAGE: inst:[0x%x] id:[0x%x] a2dp_state:[%d]\n",(uint16)theInst, id, theInst->a2dp_state));
}


/****************************************************************************
NAME
    areDevicesInScatternet

DESCRIPTION
    Find out if the audio adaptor isn't the master of a link.     

*/
static bool areDevicesInScatternet(void)
{
    uint16 i;    
    devInstanceTaskData *inst;
 
    /* Look at all possible connections */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {          
            /* If there is at least one connection in which the audio adaptor isn't Master
               then assume scatternet.
            */
            if (inst->role == hci_role_slave)
                return TRUE;
        }
    }
    return FALSE;
}


/****************************************************************************
NAME
    getMinAndMaxBitpools

DESCRIPTION
    Retrieve the maximum and minimum bitpools of all connected devices.     

*/
static void getMinAndMaxBitpools(uint16 *min_bitpool, uint16 *max_bitpool)
{
    uint16 i;    
    devInstanceTaskData *inst;
    
    /* Look at all possible connections */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {          
            /* Get the min and max bitpools of this device and update
               the min and max bitpool that can be supported by all devices */
            if (inst->sbc_min_bitpool > *min_bitpool)
                *min_bitpool = inst->sbc_min_bitpool;
            if (inst->sbc_max_bitpool < *max_bitpool)
                *max_bitpool = inst->sbc_max_bitpool;
        }
    }
}


/****************************************************************************
NAME
    getLowestAudioQuality

DESCRIPTION
    Find the lowest audio quality of all connected devices.     

*/
static a2dpAudioQuality getLowestAudioQuality(void)
{
    uint16 i;    
    devInstanceTaskData *inst;
    a2dpAudioQuality quality = A2DP_AUDIO_QUALITY_UNKNOWN;
    
    /* Look at all possible connections */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {          
            /* See if the quality of this link is lower */
            if (inst->a2dp_audio_quality < quality)
                quality = inst->a2dp_audio_quality;
            /* read the remote features if the quality is unknown */
            if (inst->a2dp_audio_quality == A2DP_AUDIO_QUALITY_UNKNOWN)
            {	
    	        ConnectionReadRemoteSuppFeatures(&the_app->task, inst->a2dp_sig_sink);
            }
        }
    }
    return quality;
}

static void chooseSbcAudioQuality(devInstanceTaskData *theInst)
{
    /* The streams are compatible but may need to adjust bitrate */                
    uint16 min_bitpool;
    uint16 max_bitpool;
    uint16 new_bitpool;
    uint16 poor_link_bitpool;
    a2dpAudioQuality quality;
    bool scatternet = areDevicesInScatternet();
    
    min_bitpool = theInst->sbc_min_bitpool;
    max_bitpool = theInst->sbc_max_bitpool;
    /* find the min and max bitpools supported by all devices */
    getMinAndMaxBitpools(&min_bitpool, &max_bitpool);
    /* get the lowest audio quality based on device supported features */
    quality = getLowestAudioQuality();
    /* reduce the quality further if scatternet */
    if (scatternet && (quality > A2DP_AUDIO_QUALITY_LOW) && (quality < A2DP_AUDIO_QUALITY_UNKNOWN))
        quality--;
    /* work out the new bitpool to use */
    if (quality != A2DP_AUDIO_QUALITY_UNKNOWN)
    {
        /* get the bitpool value for this audio quality level */
        new_bitpool = max_bitpool_array[quality];
        /* get the bitpool value for this audio quality level under poor link conditions */
        poor_link_bitpool = max_bitpool_poor_link_array[quality];
    }
    else
    {
        /* the audio quality is unknown so just assume the quality */
        new_bitpool = max_bitpool_array[A2DP_AUDIO_QUALITY_HIGH];
        poor_link_bitpool = max_bitpool_poor_link_array[A2DP_AUDIO_QUALITY_HIGH];
    }
    if (new_bitpool < min_bitpool)
        new_bitpool = min_bitpool;
    if (new_bitpool > max_bitpool)
        new_bitpool = max_bitpool;
    if (poor_link_bitpool < min_bitpool)
        poor_link_bitpool = min_bitpool;
    if (poor_link_bitpool > max_bitpool)
        poor_link_bitpool = max_bitpool;
    /* send new bitpool to DSP */
    if (new_bitpool != the_app->a2dp_data.codecData.bitpool)
    {
#ifdef KAL_MSG
        DEBUG_A2DP(("    set new bitpool [%d]\n", new_bitpool));
        KalimbaSendMessage(KALIMBA_SET_BITPOOL_MESSAGE, new_bitpool, 0, 0, 0);
        KalimbaSendMessage(KALIMBA_SET_POOR_LINK_BITPOOL_MESSAGE, poor_link_bitpool, 0, 0, 0);
#endif		
        the_app->a2dp_data.codecData.bitpool = new_bitpool;
    }
}



/****************************************************************************
NAME
    closeMedia

DESCRIPTION
    Closes any open media connection for the device instance passed
    to the function. Optionally reopens the media channel once the current
    media channel is closed.

*/
static void closeMedia(devInstanceTaskData *theInst)
{
    switch (getA2dpState(theInst))
    {
        case A2dpStateOpen:
        case A2dpStateStarting:
        case A2dpStateStreaming:
        case A2dpStateSuspending:
        {
            /* Close the open media channel */
            setA2dpState(theInst, A2dpStateClosing);
            A2dpClose(theInst->a2dp);
            /* Reopen the media channel if the a2dp_closing flag has been set. */
            if (theInst->a2dp_closing)
            {
                theInst->a2dp_reopen_tries = 0;
                theInst->a2dp_reopen_codec = 0;
                MessageSendConditionally(&theInst->task, APP_MEDIA_CHANNEL_REOPEN_REQ, 0, &theInst->a2dp_closing);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}


/****************************************************************************
NAME
    audioDisconnect

DESCRIPTION
    Stops the DSP processing the A2DP audio by calling into the audio plugin library.

*/
static bool audioDisconnect(devInstanceTaskData *inst, Sink media_sink)
{
    /* Switch DSP to the IDLE mode */
    if (!a2dpSlcIsDifferentMediaStreaming(inst))
    {                
        if (the_app->active_encoder == EncoderAv)
        {
            /* This was the only device streaming A2DP, so set DSP to idle */
            DEBUG_A2DP(("AudioDisconnect\n"));
            AudioDisconnect();
            the_app->active_encoder = EncoderNone;
            a2dpStreamSetDspIdle();
        }

        /* return that all audio has been disconnected */
        return TRUE;
    }
    else if (the_app->active_encoder == EncoderAv)
    {
        /* There is another device streaming A2DP, so just set this audio connection
           to idle */
        dualstream_mode_params *dual_mode = (dualstream_mode_params *)PanicUnlessMalloc(sizeof(dualstream_mode_params));
        dual_mode->connect_sink = FALSE; /* Indicate the audio is being disconnected */
        dual_mode->media_sink = media_sink; /* Supply the media sink being disconnected */
        AudioSetMode(AUDIO_MODE_CONNECTED, (void*)dual_mode);
                
        DEBUG_A2DP(("AudioSetMode disconnect_sink:0x%x \n",(uint16)media_sink));
    }
    
    /* not all audio was disconnected */
    return FALSE;
}

    
/****************************************************************************
NAME
    acceptA2dpConnectInd

DESCRIPTION
    Decide if the incoming A2DP connection can be connected.
    Returns the device instance if successful, otherwise returns NULL.

*/
static devInstanceTaskData *acceptA2dpConnectInd(bdaddr *bd_addr)
{
    devInstanceTaskData *inst;
    uint16 i;
    uint16 active_a2dp_connections = 0;
    
    /* 1 A2DP connection allowed for non-DualStream, or 2 for DualStream */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {            
            switch (getA2dpState(inst))
            {
                case A2dpStateDisconnected:               
                case A2dpStateDisconnecting:
                {
                    /* These states are okay, there's no active connection */
                    break;
                }
                case A2dpStateOpening:
                {                  
                    /* An incoming connection is not currently allowed if there is an outgoing connection */
                    return NULL;                  
                }        
                default:
                {
                    /* Mark that there is already an ongoing connection here */                    
                    active_a2dp_connections++;
                }
            }
        }
    }

    if (active_a2dp_connections < MAX_NUM_DEV_CONNECTIONS)
    {                            
        /* Get the instance for this device */
        inst = devInstanceFindFromBddr(bd_addr, TRUE);    
    
        if (inst != NULL)
        {                     
            /* The incoming connection has been accepted, act on it */
            profileSlcAcceptConnectInd(inst);
            
            return inst;
        }
    }
    
    return NULL;
}


/****************************************************************************
NAME
      handleA2DPInitCfm

DESCRIPTION
     Handles the A2DP library initialisation result.

*/
static void handleA2DPInitCfm(const A2DP_INIT_CFM_T *msg)
{
    if ( msg->status == a2dp_success )
    {
        /* A2DP Library initialisation was a success */
        /* Keep a record of the A2DP SEP pointer. */
        the_app->a2dp_data.sep_entries = msg->sep_list;

        /* Store that the A2DP library has been initialised */
        initProfileCfm(ProfileA2dp, TRUE);
    }
    else
    {
        /* Handle the failure of the A2DP library initialisation */
        initProfileCfm(ProfileA2dp, FALSE);
    }
}


/****************************************************************************
NAME
      handleA2DPSignallingConnectInd

DESCRIPTION
     Handles the A2DP library incoming signalling connection message.

*/
static void handleA2DPSignallingConnectInd(A2DP_SIGNALLING_CHANNEL_CONNECT_IND_T *ind)
{
    /* Check that this incoming connection can be accepted */
    devInstanceTaskData *dev_inst = acceptA2dpConnectInd(&ind->addr);

    if (dev_inst != NULL)
    {      
        /* Update state to indicate remote device is connecting */
        setA2dpState(dev_inst, A2dpStatePaged);   
        /* update the remote profiles supported */
        dev_inst->remote_profiles |= ProfileA2dp;
        /* Accept incoming connection */
        DEBUG_A2DP(("   Accepted A2DP signalling\n"));
        A2dpConnectSignallingChannelResponse(&dev_inst->task, ind->a2dp, TRUE, ind->connection_id, the_app->a2dp_data.sep_entries);
        return;
    }
    
    /* Reject incoming connection, either there is an existing instance, or we failed to create a new instance */
    DEBUG_A2DP(("   Rejected A2DP signalling\n"));
    A2dpConnectSignallingChannelResponse(&the_app->task, ind->a2dp, FALSE, ind->connection_id, the_app->a2dp_data.sep_entries);    
}


/****************************************************************************
NAME
      handleA2DPSignallingConnectCfm

DESCRIPTION
    Handles the A2DP library signalling connection confirmation message.
    
*/
static void handleA2DPSignallingConnectCfm(devInstanceTaskData *inst, A2DP_SIGNALLING_CHANNEL_CONNECT_CFM_T *msg)
{   
    bool remote_connect = FALSE;
    switch (getA2dpState(inst))
    {
        case A2dpStatePaged:
            remote_connect = TRUE;
        case A2dpStateOpening:
        case A2dpStateDisconnected:
        {
            if (msg->status != a2dp_success)
            {
                /* Signalling connection failed so update state to 'disconnected' */
                DEBUG_A2DP(("   Signalling Failed [Status %d]\n", msg->status));
                setA2dpState(inst, A2dpStateDisconnected);
                MessageSend(&inst->task, APP_INTERNAL_DESTROY_REQ, 0);
                return;
            }

            /* If there is already a signalling channel connected, then disconnect this new connection */
            if (inst->a2dp)
            {
                DEBUG_A2DP(("   Already connected, disconnect signalling\n"));
                setA2dpState(inst, A2dpStateDisconnecting);
                A2dpDisconnectAll( msg->a2dp );
                return;
            }

            DEBUG_A2DP(("   Signalling Success\n"));

            /* store the a2dp library instance */
            inst->a2dp = msg->a2dp;
            inst->a2dp_sig_sink = msg->sink;
            
            /* Get the current role */
            ConnectionGetRole(&the_app->task, msg->sink);   
            
            /* If we don't know what audio quality is required, then get remote support
               features */
            if (inst->a2dp_audio_quality == A2DP_AUDIO_QUALITY_UNKNOWN)
            {	
                /* Read the remote supported features of the remote device */
	            ConnectionReadRemoteSuppFeatures(&the_app->task, inst->a2dp_sig_sink);
            }
            
            /* update state to indicate signalling channel connected */
            setA2dpState(inst, A2dpStateConnected);

            /*  Some headsets automatically open media channel
                    after signalling channel connect successfully;
                    some headsets do not. So we need waiting for 3
                    seconds and if no open indication is received,
                    the audio adaptor opens the media channel.
            */
            if(remote_connect)
                MessageSendLater(&inst->task, APP_INTERNAL_CONNECT_MEDIA_CHANNEL_REQ, 0, D_SEC(3));
            else
                a2dpSlcConnect(inst);
           
            break;
        }
        default:
        {
            a2dpUnhandledMessage(inst, A2DP_SIGNALLING_CHANNEL_CONNECT_CFM);
            break;
        }
    }
}


/****************************************************************************
NAME
      handleA2DPOpenInd

DESCRIPTION
    Handles the A2DP library media channel open message (opened by remote device).
    
 */
static void handleA2DPOpenInd(devInstanceTaskData *inst, A2DP_OPEN_IND_T *msg)
{
    Sink sink  = msg->media_sink;
    uint8 seid = msg->seid;
    
    switch (getA2dpState(inst))
    {
        case A2dpStateConnected:
        case A2dpStateOpening:
        {
            /* see if an A2DP connection already exists */
            bool a2dp_open = a2dpSlcIsMediaOpen();
            
            /* remove the media channel connection request from msg queue */
            MessageCancelAll(&inst->task, APP_INTERNAL_CONNECT_MEDIA_CHANNEL_REQ);

            /* update link power table */
            ConnectionSetLinkPolicy(sink, 3, a2dp_active);
            
            /* Get the current role */
            ConnectionGetRole(&the_app->task, msg->media_sink);    
            
            /* store media sink */
            inst->a2dp_media_sink = msg->media_sink;
            /* store the stream end point ID that's in use */
            inst->a2dp_seid = seid;
            DEBUG_A2DP(("    Selected SEID = %d\n", seid));

            /* update state to 'open' */
            setA2dpState(inst, A2dpStateOpen);

        #if defined MEDIA_STREAM_HOLDOFF
            a2dpStreamStartMediaStreamHoldoff(app);
        #endif

            /*  Waiting for all profiles supported by
                audio adaptor to be connected    */
            profileSlcConnectCfm(inst, ProfileA2dp, TRUE);

            /* Only initailise DSP if this is the first stream to open */
            if (!a2dp_open)
            {
                /* Load the Kap file and let DSP run     */
                a2dpStreamStartDsp(TRUE);
            }
            if (inst->a2dp_closing)
            {
                /* If the flag is set to close the media channel, then do this now */
                closeMedia(inst);
            }
            break;
        }
        case A2dpStateDisconnecting:
        {
            DEBUG_A2DP(("  - ignored\n"));
            break;
        }
        default:
        {
            a2dpUnhandledMessage(inst, A2DP_OPEN_IND);
            break;
        }
    }
}


/****************************************************************************
  NAME
      handleA2DPOpenCfm

DESCRIPTION
    Handles the A2DP library media channel open message (opened by local device).
    
*/
static void handleA2DPOpenCfm(devInstanceTaskData *inst, A2DP_OPEN_CFM_T *msg)
{
    a2dp_status_code status = msg->status;
    Sink sink   = msg->media_sink;
    uint8 seid  = msg->seid;
    
    switch (getA2dpState(inst))
    {
        case A2dpStateDisconnected: /* signalling could have disconnected while opening */
        case A2dpStateOpening:
        {
            /* see if an A2DP connection already exists */
            bool a2dp_open = a2dpSlcIsMediaOpen();

            if (status == a2dp_success)
            {
                /* Media open success */
                
                /* update link power table */
                ConnectionSetLinkPolicy(sink, 3, a2dp_active);
                
                /* Get the current role */
                ConnectionGetRole(&the_app->task, msg->media_sink);    
                
                /* store media sink */
                inst->a2dp_media_sink = msg->media_sink;
                /* store the stream end point ID that's in use */
                inst->a2dp_seid = seid;
                DEBUG_A2DP(("    Selected SEID = %d\n", seid));

                setA2dpState(inst, A2dpStateOpen);

            #if defined MEDIA_STREAM_HOLDOFF
                a2dpStreamStartMediaStreamHoldoff(app);
            #endif

                if (!inst->a2dp_reopen)
                {
                    /*  Waiting for all profiles supported by
                    audio adaptor to be connected    */
                    profileSlcConnectCfm(inst, ProfileA2dp, FALSE);
                }
                else if (!the_app->voip_call_active)
                {
                    /* Kick off streaming if no VOIP call active */
                    eventHandleSendKickCommActionMessage(CommActionStream);                    
                }

                /* Only initailise DSP if this is the first stream to open */
                if (!a2dp_open)
                {
                    /* Load the Kap file and let DSP run */
                    a2dpStreamStartDsp(TRUE);
                }                    
            }
            else
            {
                /* Media open failure */
                
                /* Update state based on the signalling connection */
                if (inst->a2dp_sig_sink)
                    setA2dpState(inst, A2dpStateConnected);
                else
                    setA2dpState(inst, A2dpStateDisconnected);
                    
                if (inst->a2dp_reopen)
                {
                    /* A media channel reopen has failed. Try another time if max tries not reached - 
                        incase signalling channel was being disconnected. */
                    DEBUG_A2DP(("    Reopen failed! "));                    
                    if (inst->a2dp_reopen_tries < MAX_A2DP_REOPEN_TRIES)
                    {
                        DEBUG_A2DP(("Try connection again...\n"));
                        inst->a2dp_reopen_tries++;
                        MessageSendLater(&inst->task, APP_MEDIA_CHANNEL_REOPEN_REQ, 0, 1000);
                        MessageCancelAll(&inst->task, APP_INTERNAL_DESTROY_REQ);
                    }
                    else
                    {
                        DEBUG_A2DP(("End connection attempt.\n"));
                        inst->a2dp_reopen = FALSE;
                    }
                }
                else
                {
                    the_app->s_connect_attempts += 1;

                    if( msg->status == a2dp_key_missing )
                    {
                        /* Delete link key from our side and try again */
                        ConnectionSmDeleteAuthDevice(&inst->bd_addr);
                        a2dpSlcConnect(inst);
                    }
                    else
                    {
                    #if defined MEDIA_STREAM_HOLDOFF
                        a2dpStreamStopMediaStreamHoldoff(app);
                    #endif

                        /*  Waiting for all profiles supported by
                        audio adaptor to be connected    */
                        profileSlcConnectCfm(inst, ProfileA2dp, FALSE);
                    }
                }
            }
            
            if (inst->a2dp_closing)
            {
                /* If the flag is set to close the media channel, then do this now */
                closeMedia(inst);
            }
            inst->a2dp_reopen = FALSE;
            
            break;
        }
        case A2dpStateOpen:
        case A2dpStateStarting:
        case A2dpStateStreaming:
        case A2dpStateDisconnecting:
        {
            DEBUG_A2DP(("  - ignored\n"));
            break;
        }
        default:
        {
            a2dpUnhandledMessage(inst, A2DP_OPEN_CFM);
            break;
        }
    }
}


/****************************************************************************
  NAME
      handleA2DPConnectOpenCfm

DESCRIPTION
    Handles the A2DP library signalling and media channel open message (opened by local device).
    
*/
static void handleA2DPConnectOpenCfm(devInstanceTaskData *inst, A2DP_CONNECT_OPEN_CFM_T *msg)
{
    switch (getA2dpState(inst))
    {
        case A2dpStatePaged:       
        case A2dpStateOpening:
        {    
            if (msg->status != a2dp_success)
            {               
                /* Update state based on the signalling connection */
                if (inst->a2dp_sig_sink)
                    setA2dpState(inst, A2dpStateConnected);
                else
                    setA2dpState(inst, A2dpStateDisconnected);

                if (inst->a2dp_reopen)
                {
                    /* A media channel reopen has failed. Try another time if max tries not reached - 
                        incase signalling channel was being disconnected. */
                    DEBUG_A2DP(("    Reopen failed! Try again...\n"));                    
                    if (inst->a2dp_reopen_tries < MAX_A2DP_REOPEN_TRIES)
                    {
                        inst->a2dp_reopen_tries++;
                        MessageSendLater(&inst->task, APP_MEDIA_CHANNEL_REOPEN_REQ, 0, 1000);
                        MessageCancelAll(&inst->task, APP_INTERNAL_DESTROY_REQ);
                    }
                    else
                    {
                        inst->a2dp_reopen = FALSE;
                    }
                }
                else
                {
                    /* Send event to signify that reconnection attempt failed */
                    the_app->s_connect_attempts += 1;

                    if( msg->status == a2dp_key_missing )
                    {
                        /* Delete link key from our side and try again */
                        ConnectionSmDeleteAuthDevice(&inst->bd_addr);
                        a2dpSlcConnect(inst);
                    }
                    else
                    {
                    #if defined MEDIA_STREAM_HOLDOFF
                        a2dpStreamStopMediaStreamHoldoff();
                    #endif

                        profileSlcConnectCfm(inst, ProfileA2dp, FALSE);
                    
                        MessageSend(&inst->task, APP_INTERNAL_DESTROY_REQ, 0);
                    }
                }
            }
            else
            {
                /* see if an A2DP connection already exists */
                bool a2dp_open = a2dpSlcIsMediaOpen();
                
                /* Reset the connection attempt number */
                the_app->s_connect_attempts  = 0;
                
                /* If there is already a signalling channel connected,
                then disconnect this new connection */
                if (inst->a2dp)
                {
                    setA2dpState(inst, A2dpStateDisconnecting);
                    A2dpDisconnectAll(msg->a2dp);
                    return;
                }

                /* do some check to see the address is correct or not! */

                inst->a2dp = msg->a2dp;
                inst->a2dp_sig_sink = msg->signalling_sink;
                inst->a2dp_media_sink = msg->media_sink;
                inst->a2dp_seid = msg->seid;
                DEBUG_A2DP(("    Selected SEID = %d\n", msg->seid));
                   
                /* set link policy */
                ConnectionSetLinkPolicy(msg->media_sink, 3, a2dp_active);
                
                /* Get the current role */
                ConnectionGetRole(&the_app->task, msg->media_sink);  
                
                /* If we don't know what audio quality is required, then get remote support
                   features */
                if (inst->a2dp_audio_quality == A2DP_AUDIO_QUALITY_UNKNOWN)
                {	
                    /* Read the remote supported features of the remote device */
    	            ConnectionReadRemoteSuppFeatures(&the_app->task, inst->a2dp_sig_sink);
                }

                setA2dpState(inst, A2dpStateOpen);

            #if defined MEDIA_STREAM_HOLDOFF
                a2dpStreamStartMediaStreamHoldoff();
            #endif

                if (!inst->a2dp_reopen)
                {
                    profileSlcConnectCfm(inst, ProfileA2dp, TRUE);
                }
                else if (!the_app->voip_call_active)
                {
                    /* Kick off streaming if no VOIP call active */
                    eventHandleSendKickCommActionMessage(CommActionStream);       
                }

                /*
                    For USB input, we should start the DSP and monitor the USB state
                */
                
                /* Only initailise DSP if this is the first stream to open */
                if (!a2dp_open)
                {
                    /* Load the Kap file and let DSP run */
                    a2dpStreamStartDsp(TRUE);               
                }
            }
            
            if (inst->a2dp_closing)
            {
                /* If the flag is set to close the media channel, then do this now */
                closeMedia(inst);
            }
            inst->a2dp_reopen = FALSE;
            
            break;
        }
        case A2dpStateDisconnecting:
        {
            DEBUG_A2DP(("  - ignored\n"));
            break;
        }
        default:
        {
            a2dpUnhandledMessage(inst, A2DP_CONNECT_OPEN_CFM);
            break;
        }
    }
}


/****************************************************************************
  NAME
      handleA2DPStartInd

DESCRIPTION
    Handles the A2DP library start streaming message (started by remote device).
    
*/
static void handleA2DPStartInd(devInstanceTaskData *inst, A2DP_START_IND_T *msg)
{
    switch (getA2dpState(inst))
    {
        case A2dpStateSuspending:
        case A2dpStateClosing:
        case A2dpStateDisconnecting:
        {
            /* Ignore */
            DEBUG_A2DP(("  - ignored\n"));
            return;
        }
        case A2dpStateStarting:
        case A2dpStateOpen:
        {          
            {
                Sink media_sink = inst->a2dp_media_sink;
                bool a2dp_streaming = a2dpSlcIsMediaStreaming();
                
                setA2dpState(inst, A2dpStateStreaming);

                /* For Analogue mode, load the kap file again */
                if (!a2dp_streaming)
                {
                    /* Load the Kap file and let DSP run */
                    a2dpStreamStartDsp(TRUE);
                }
                
                /* Connect the corresponding plugin and start streaming */
                if (media_sink)
                {
                    if (a2dpSlcIsMediaStarting())
                    {
                        /* If another media connection is about to start streaming then
                           delay starting this stream so both can be handled at the same time */
                        MAKE_APP_MESSAGE(APP_CONNECT_A2DP_AUDIO);
                        message->media_sink = media_sink;
                        MessageSendLater(&the_app->task, APP_CONNECT_A2DP_AUDIO, message, 1000);
                    }
                    else
                    {
                        /* Route audio */
                        a2dpStreamConnectA2dpAudio(media_sink);
                    }
                }
                
                streamManagerOpenNotify();
            }
            break;
        }
        default:
        {
            a2dpUnhandledMessage(inst, A2DP_START_IND);
            break;
        }
    }
}


/****************************************************************************
  NAME
      handleA2DPStartCfm

DESCRIPTION
    Handles the A2DP library start streaming message (started by local device).
    
*/
static void handleA2DPStartCfm(devInstanceTaskData *inst, A2DP_START_CFM_T *msg)
{
    switch (getA2dpState(inst))
    {
        case A2dpStateStarting:
        {
            if (msg->status == a2dp_success)
            {
                /* Start streaming succeeded */
                Sink media_sink = inst->a2dp_media_sink;
                bool a2dp_streaming = a2dpSlcIsMediaStreaming();
                
                setA2dpState(inst, A2dpStateStreaming);
                
                /* If a call is now active the stream must be suspended */
                if (the_app->app_state == AppStateInCall)
                {
                    setA2dpState(inst, A2dpStateSuspending);                        
                    A2dpSuspend(inst->a2dp);
                    break;
                }
                
                /* Connect the corresponding plugin and start streaming */

                /* For Analogue mode, load the kap file again */
                if(!a2dp_streaming)
                {
                    /* Load the Kap file and let DSP run */
                    a2dpStreamStartDsp(TRUE);
                }

                if (media_sink)
                {
                    if (a2dpSlcIsMediaStarting())
                    {
                        /* If another media connection is about to start streaming then
                           delay starting this stream so both can be handled at the same time */
                        MAKE_APP_MESSAGE(APP_CONNECT_A2DP_AUDIO);
                        message->media_sink = media_sink;
                        MessageSendLater(&the_app->task, APP_CONNECT_A2DP_AUDIO, message, 1000);
                    }
                    else
                    {
                        /* Route audio */
                        a2dpStreamConnectA2dpAudio(media_sink);
                    }
                }

                streamManagerOpenComplete(TRUE);
            }
            else
            {
                /* Start streaming failed */
                setA2dpState(inst, A2dpStateOpen);

                streamManagerOpenComplete(FALSE);
            }
            break;
        }
        case A2dpStateConnected:
        case A2dpStateClosing:
        case A2dpStateStreaming:  
        case A2dpStateDisconnecting:
        {
            /* Ignore */
            DEBUG_A2DP(("  - ignored\n"));
            break;
        }
        default:
        {
            a2dpUnhandledMessage(inst, A2DP_START_CFM);
            break;
        }
    }
}


/****************************************************************************
  NAME
      handleA2DPSuspendInd

DESCRIPTION
     Handles the A2DP library suspend streaming message (suspended by remote device).
        
*/
static void handleA2DPSuspendInd(devInstanceTaskData *inst, A2DP_SUSPEND_IND_T *msg)
{
    switch (getA2dpState(inst))
    {
        case A2dpStateOpen:
        {
        #if defined MEDIA_STREAM_HOLDOFF
            a2dpStreamStartMediaStreamHoldoff(app);
        #endif
            break;
        }
        case A2dpStateStreaming:
        case A2dpStateStarting:
        case A2dpStateSuspending:
        {
            setA2dpState(inst, A2dpStateOpen);

            /* Attempt disconnect of audio */
            if (audioDisconnect(inst, msg->media_sink))
            {            
                streamManagerCloseComplete();
            }
           
            /* We just paused, try EPR immediately */
            MessageCancelFirst(&inst->task, APP_REFRESH_ENCRYPTION_REQ);
            MessageSend(&inst->task, APP_REFRESH_ENCRYPTION_REQ, 0);
            
            break;
        }  
        case A2dpStateClosing:
        case A2dpStateDisconnecting:
        {
            /* Ignore */
            DEBUG_A2DP(("  - ignored\n"));
            break;
        }
        default:
        {
            a2dpUnhandledMessage(inst, A2DP_SUSPEND_IND);
            break;
        }
    }
}


/****************************************************************************
  NAME
      handleA2DPSuspendCfm

DESCRIPTION
     Handles the A2DP library suspend streaming message (suspended by local device).
*/
static void handleA2DPSuspendCfm(devInstanceTaskData *inst, A2DP_SUSPEND_CFM_T *msg)
{
    switch (getA2dpState(inst))
    {
        case A2dpStateOpen:
        {
        #if defined MEDIA_STREAM_HOLDOFF
            a2dpStreamStartMediaStreamHoldoff(app);
        #endif
            break;
        }
        case A2dpStateSuspending:
        {
            if (msg->status == a2dp_success)
            {
                /* Suspend success */
                setA2dpState(inst, A2dpStateOpen);
               
                if (audioDisconnect(inst, msg->media_sink))
                {
                    streamManagerCloseComplete();
                }
                
                /* We just paused, try EPR immediately */
                MessageCancelFirst(&inst->task, APP_REFRESH_ENCRYPTION_REQ);
                MessageSend(&inst->task, APP_REFRESH_ENCRYPTION_REQ, 0);
            }
            else
            {
                /* Suspend failed so should close media */
                setA2dpState(inst, A2dpStateStreaming);
                closeMedia(inst);
            }            
            break;
        }  
        case A2dpStateClosing:
        case A2dpStateConnected:
        case A2dpStateDisconnecting:
        {
            /* Ignore */
            DEBUG_A2DP(("  - ignored\n"));
            break;
        }
        default:
        {
            a2dpUnhandledMessage(inst, A2DP_SUSPEND_CFM);
            break;
        }
    }
}


/****************************************************************************
  NAME
      handleA2DPSignallingChannelDisconnectInd

DESCRIPTION
     Handles the A2DP library message to indicate a signalling connection is closed.

*/
static void handleA2DPSignallingChannelDisconnectInd(devInstanceTaskData *inst, A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND_T *msg)
{
    /* Reset A2DP connection once A2DP signalling channel closed */
    switch (getA2dpState(inst))
    {
        case A2dpStateConnected:
        case A2dpStateOpening:
        case A2dpStateOpen:
        case A2dpStateDisconnecting:
        case A2dpStateStarting:
        case A2dpStateStreaming:
        case A2dpStateSuspending:
        case A2dpStateClosing:
        {
            /* Set state to disconnected */
            setA2dpState(inst, A2dpStateDisconnected);
            /* reset the a2dp library instance */
            inst->a2dp = NULL;      
            inst->a2dp_sig_sink = 0;
            inst->a2dp_media_sink = 0;
            /* store that no longer responding */
            inst->responding_profiles &= ~ProfileA2dp;
            /* Disconnect notification */
            profileSlcDisconnectInd(inst, ProfileA2dp);
            /* send a message to try and delete this device instance,
                 only if not reopening the connection */
            if (MessageCancelAll(&inst->task, APP_MEDIA_CHANNEL_REOPEN_REQ))
                MessageSend(&inst->task, APP_MEDIA_CHANNEL_REOPEN_REQ, 0);
            else
                MessageSend(&inst->task, APP_INTERNAL_DESTROY_REQ, 0);   
            
            /* Remove the AVRCP connection if it still exists */
            if (inst->avrcp_state == AvrcpStateConnected)
            {
                setAvrcpState(inst, AvrcpStateDisconnecting);  
                AvrcpDisconnect(inst->avrcp);
            }
            break;
        }
        case A2dpStateDisconnected:
        {
            /* Ignore */
            DEBUG_A2DP(("  - ignored\n"));
            break;
        }
        default:
        {
            a2dpUnhandledMessage(inst, A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND);
            break;
        }
    }
}


/****************************************************************************
  NAME
      handleA2DPClose

DESCRIPTION
     Handles the generic A2DP media closed message.
    
*/
static void handleA2DPClose(devInstanceTaskData *inst, Sink media_sink)
{    
#if defined MEDIA_STREAM_HOLDOFF
    a2dpStreamStopMediaStreamHoldoff(app);
#endif    
    
    if (audioDisconnect(inst, media_sink))
    {
        if (the_app->app_state == AppStateStreaming)
            setAppState(AppStateIdle);
    }
}


/****************************************************************************
  NAME
      handleA2DPCloseInd

DESCRIPTION
     Handles the A2DP library message to indicate a media connection is closed.
    
*/
static void handleA2DPCloseInd(devInstanceTaskData *inst, A2DP_CLOSE_IND_T *msg)
{
    handleA2DPClose(inst, msg->media_sink);
   
    switch (getA2dpState(inst))
    {
        case A2dpStateOpen:
        case A2dpStateStarting:
        case A2dpStateStreaming:
        case A2dpStateSuspending:
        case A2dpStateClosing:
        case A2dpStateDisconnecting:
        {           
            inst->a2dp_closing = FALSE;
            inst->a2dp_media_sink = 0;
            setA2dpState(inst, A2dpStateConnected);               
            break;
        }
        case A2dpStateDisconnected:
        {
            break;
        }
        default:
        {
            a2dpUnhandledMessage(inst, A2DP_CLOSE_IND);
            break;
        }
    }
}


/****************************************************************************
  NAME
      handleA2DPCloseCfm

DESCRIPTION
     Handled confirmation of the request to close media streaming on previously opened connection
    after calling function A2dpClose.

*/
static void handleA2DPCloseCfm(devInstanceTaskData *inst, A2DP_CLOSE_CFM_T *msg)
{
    handleA2DPClose(inst, msg->media_sink);
    
    switch (getA2dpState(inst))
    {               
        case A2dpStateClosing:
        case A2dpStateDisconnecting:
        {           
            inst->a2dp_closing = FALSE;
            inst->a2dp_media_sink = 0;
            setA2dpState(inst, A2dpStateConnected);
            break;
        }
        case A2dpStateDisconnected:
        {
            break;
        }
        default:
        {
            a2dpUnhandledMessage(inst, A2DP_CLOSE_CFM);
            break;
        }
    }
}


/****************************************************************************
  NAME
      handleConfigCodec

DESCRIPTION
     If A2DP_CONFIGURE_CODEC_IND is received we need to configure the codec
    based on the remote SEP capabilities;
    
*/
static bool handleConfigCodec(A2DP_CONFIGURE_CODEC_IND_T *msg)
{
    if (msg->codec_service_caps[3] == AVDTP_MEDIA_CODEC_SBC)
    {
        uint8 bitpool;
        uint8 sample_rate_index;
        uint8 channel_mode;

        bitpool            = (uint8)(the_app->pskey_sbc_codec_config & 0x00ff);
        sample_rate_index  = (uint8)((the_app->pskey_sbc_codec_config & 0x0f00) >> 8);
        channel_mode       = (uint8)((the_app->pskey_sbc_codec_config & 0xe000) >> 13);

        /* Sample rate index*/
        if (sample_rate_index < NUM_SBC_SAMPLE_RATE_SUPPORTED)
        {
            msg->codec_service_caps[4] &= sample_rate_mask_sbc[sample_rate_index];
        }
        else
        {
            if ((msg->codec_service_caps[4] & 0x20))         /* choose 44.1kHz */
                msg->codec_service_caps[4] &= 0x2f;
            else if ((msg->codec_service_caps[4] & 0x40))    /* choose 32kHz */
                msg->codec_service_caps[4] &= 0x4f;
            else                                             /* choose 16kHz */
                msg->codec_service_caps[4] &= 0x8f;
        }

        /* Select Channel Mode. Only mono is mandatory at the source. */
        if (channel_mode <= a2dp_joint_stereo)
        {
            msg->codec_service_caps[4] &= channel_mode_mask[channel_mode];
        }
        else
        {
            if ((msg->codec_service_caps[4] & 0x01))           /* choose joint stereo */
                msg->codec_service_caps[4] &= 0xf1;
            else if ((msg->codec_service_caps[4] & 0x02))      /* choose stereo */
                msg->codec_service_caps[4] &= 0xf2;
            else if ((msg->codec_service_caps[4] & 0x04))      /* choose dual channel */
                msg->codec_service_caps[4] &= 0xf4;
            else                                               /* choose mono */
                msg->codec_service_caps[4] &= 0xf8;
        }

           /* Select Block Length. All lengths are mandatory at Source and Sink. */

        if (msg->codec_service_caps[5] & 0x10)          /* choose 16 */
            msg->codec_service_caps[5] &= 0x1f;
        else if (msg->codec_service_caps[5] & 0x20)     /* choose 12 */
            msg->codec_service_caps[5] &= 0x2f;
        else if (msg->codec_service_caps[5] & 0x40)     /* choose 8 */
            msg->codec_service_caps[5] &= 0x4f;
        else                                            /* choose 4 */
            msg->codec_service_caps[5] &= 0x8f;

        /* Select Subbands. 8 subbands is mandatory at both Source and Sink. */

        if (msg->codec_service_caps[5] & 0x04)         /* choose 8 */
            msg->codec_service_caps[5] &= 0xf7;
        else                                           /* choose 4 */
            msg->codec_service_caps[5] &= 0xfb;

        /* Select Allocation Method. Loudness is mandatory at both Source and Sink. */

        if (msg->codec_service_caps[5] & 0x01)         /* choose Loudness */
            msg->codec_service_caps[5] &= 0xfd;
        else                                            /* choose SNR */
            msg->codec_service_caps[5] &= 0xfe;

        /* Select the proper bitpool value, which decide the audio quality */

        if (bitpool >= msg->codec_service_caps[6] && bitpool <= msg->codec_service_caps[7])
            msg->codec_service_caps[7] = bitpool;
        
        if (msg->codec_service_caps[4] == 0 || msg->codec_service_caps[5] == 0)
            return FALSE;

    }
    else if (msg->codec_service_caps[3] == AVDTP_MEDIA_CODEC_NONA2DP)
    {
        uint8  sample_rate_index = 0;

        if (the_app->bidirect_faststream)
            msg->codec_service_caps[10] &= 0x03;
        else
            msg->codec_service_caps[10] &= 0x01;

        /*
            Select FASTSTREAM parameters for optimal performance.
            We support 48kHz or 44.1kHz for USB endpoint and 44.1kHz
            for analogue endpoint;
            Make sure the headset also support these frequencies!!
        */

        sample_rate_index  = (uint8)((the_app->pskey_faststream_codec_config & 0x0f00) >> 8);

        /* Sample rate index*/
        if (sample_rate_index < NUM_FASTSTREAM_SAMPLE_RATE_SUPPORTED)
        {
            msg->codec_service_caps[11] &= sample_rate_mask_fs[sample_rate_index];
        }
        else
        {
            msg->codec_service_caps[11] &= 0x02;
        }

        if (msg->codec_service_caps[10] == 0 || msg->codec_service_caps[11] == 0)
            return FALSE;

    }
    else if (msg->codec_service_caps[3] == AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO)
    {

        uint8  bitrateindex      = 0;
        uint8  sample_rate_index = 0;
        uint8  isVBRsupported    = 0;
        uint8  channel_mode      = 0;

        bitrateindex      = (uint8)(the_app->pskey_mp3_codec_config & 0x00ff);
        sample_rate_index = (uint8)((the_app->pskey_mp3_codec_config & 0x0f00) >> 8);
        isVBRsupported    = (uint8)((the_app->pskey_mp3_codec_config & 0x1000) >> 12);
        channel_mode      = (uint8)((the_app->pskey_mp3_codec_config & 0xe000) >> 13);

        /* layer III */
        msg->codec_service_caps[4] &= 0x3f;

        /* Use CRC protection if available, hence leave bit alone */

        /* Select Channel Mode */
        if (channel_mode <= a2dp_joint_stereo)
        {
            msg->codec_service_caps[4] &= channel_mode_mask[channel_mode];
        }
        else
        {
            if (msg->codec_service_caps[4] & 0x01)             /* choose joint stereo */
                msg->codec_service_caps[4] &= 0xf1;
            else if (msg->codec_service_caps[4] & 0x02)        /* choose stereo */
                msg->codec_service_caps[4] &= 0xf2;
            else if (msg->codec_service_caps[4] & 0x04)        /* choose dual channel */
                msg->codec_service_caps[4] &= 0xf4;
            else                                               /* choose mono */
                msg->codec_service_caps[4] &= 0xf8;
        }

        /* MPF-1 and clear RFA */
        msg->codec_service_caps[5] &= 0x3f;

        /* Sample rate index*/
        if (sample_rate_index < NUM_MP3_SAMPLE_RATE_SUPPORTED)
        {
            msg->codec_service_caps[5] &= sample_rate_mask_mp3[sample_rate_index];
        }
        else
        {
            if (msg->codec_service_caps[5] & 0x02)        /* choose 44k1 */
                msg->codec_service_caps[5] &= 0xc2;
            else if (msg->codec_service_caps[5] & 0x01)   /* choose 48k */
                msg->codec_service_caps[5] &= 0xc1;
            else if (msg->codec_service_caps[5] & 0x04)   /* choose 32k */
                msg->codec_service_caps[5] &= 0xc4;
            else if (msg->codec_service_caps[5] & 0x08)   /* choose 24k */
                msg->codec_service_caps[5] &= 0xc8;
            else if (msg->codec_service_caps[5] & 0x10)   /* choose 22.05k */
                msg->codec_service_caps[5] &= 0xd0;
            else                                          /* choose 16k */
                msg->codec_service_caps[5] &= 0xe0;
        }

        /* If user select CBR, select the CBR; otherwise */
        /* Use VBR if available, hence leave bit alone   */
        if (!isVBRsupported)
            msg->codec_service_caps[6] &= 0x7f;

        /* Bit Rate */
        if (bitrateindex < NUM_MP3_BIT_RATE_INDEX_SUPPORTED)
        {
            msg->codec_service_caps[6] &= cap6_table_mp3[bitrateindex];
            msg->codec_service_caps[7] &= cap7_table_mp3[bitrateindex];
        }
        else
        {
            /*
                If no bit rate index is input, we try and use the rate nearest to 128kbps.
                   In reality, the source won't encode MP3 in realtime
                  so it will probably only have a single bit set
                  representing it's pre-compressed file.
            */
            if (msg->codec_service_caps[6] & 0x02)
            {
                /* choose 128k */
                msg->codec_service_caps[6] &= 0x82;
                msg->codec_service_caps[7] = 0;
            }
            else if (msg->codec_service_caps[6] & 0x01)
            {
                /* choose 112k */
                msg->codec_service_caps[6] &= 0x81;
                msg->codec_service_caps[7] = 0;
            }
            else if (msg->codec_service_caps[6] & 0x04)
            {
                /* choose 160k */
                msg->codec_service_caps[6] &= 0x84;
                msg->codec_service_caps[7] = 0;
            }
            else if (msg->codec_service_caps[7] & 0x80)
            {
                /* choose 96k */
                msg->codec_service_caps[6] &= 0x80;
                    msg->codec_service_caps[7] &= 0x80;
            }
            else if (msg->codec_service_caps[6] & 0x08)
            {
                /* choose 192k */
                msg->codec_service_caps[6] &= 0x88;
                msg->codec_service_caps[7] = 0;
            }
            else if (msg->codec_service_caps[7] & 0x40)
            {
                /* choose 80k */
                msg->codec_service_caps[6] &= 0x80;
                msg->codec_service_caps[7] &= 0x40;
            }
            else if (msg->codec_service_caps[6] & 0x10)
            {
                /* choose 224k */
                msg->codec_service_caps[6] &= 0x90;
                msg->codec_service_caps[7] = 0;
            }
               else if (msg->codec_service_caps[7] & 0x20)
            {
                /* choose 64k */
                msg->codec_service_caps[6] &= 0x80;
                msg->codec_service_caps[7] &= 0x20;
            }
            else if (msg->codec_service_caps[6] & 0x20)
            {
                /* choose 256k */
                msg->codec_service_caps[6] &= 0xa0;
                msg->codec_service_caps[7] = 0;
            }
            else if (msg->codec_service_caps[7] & 0x10)
            {
                /* choose 56k */
                msg->codec_service_caps[6] &= 0x80;
                msg->codec_service_caps[7] &= 0x10;
            }
            else if (msg->codec_service_caps[6] & 0x40)
            {
                /* choose 320k */
                msg->codec_service_caps[6] &= 0xc0;
                msg->codec_service_caps[7] = 0;
            }
            else if (msg->codec_service_caps[7] & 0x08)
            {
                /* choose 48k */
                msg->codec_service_caps[6] &= 0x80;
                msg->codec_service_caps[7] &= 0x08;
            }
            else if (msg->codec_service_caps[7] & 0x04)
            {
                /* choose 40k */
                msg->codec_service_caps[6] &= 0x80;
                   msg->codec_service_caps[7] &= 0x04;
            }
            else if (msg->codec_service_caps[7] & 0x02)
            {
                /* choose 32k */
                   msg->codec_service_caps[6] &= 0x80;
                msg->codec_service_caps[7] &= 0x02;
            }
        }

        if (msg->codec_service_caps[5] == 0 || (msg->codec_service_caps[6] == 0 && msg->codec_service_caps[7] == 0))
            return FALSE;
    }

    return TRUE;
}


/****************************************************************************
  NAME
      handleA2DPConfigureCodecInd

DESCRIPTION
     If the library_selects_settings set TRUE in sep_config_type, no A2DP_CONFIGURE_CODEC_IND
    message will be received by app;

*/
static void handleA2DPConfigureCodecInd(devInstanceTaskData *inst, A2DP_CONFIGURE_CODEC_IND_T *msg)
{
    bool accept = FALSE;
    
    /* At this stage. don't store the a2dp to app structure */
    A2DP *a2dp = ((A2DP_CONFIGURE_CODEC_IND_T *)msg)->a2dp;

    /* choose what should be configured */
    accept = handleConfigCodec((A2DP_CONFIGURE_CODEC_IND_T *)msg);

    A2dpConfigureCodecResponse(a2dp, accept, msg->size_codec_service_caps, msg->codec_service_caps);
}


/****************************************************************************
  NAME
      isMP3Codec

DESCRIPTION
     Reads from the passed in codec capabilities if this is an MP3 codec.
    
*/
static bool isMP3Codec(uint8 *caps, uint16 caps_size, uint8 *codec_caps, uint16 *codec_caps_size)
{
    if (caps_size != 0)
    {
        while (caps_size != 0)
        {
            uint8 service = (caps)[0];
            uint8 losc = (caps)[1];

            if (service == AVDTP_SERVICE_MEDIA_CODEC)
            {
                memcpy(codec_caps, caps + 2, losc);
                *codec_caps_size = losc;

                if (codec_caps[1] == AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO)
                    return TRUE;
                else
                    return FALSE;
            }

            /* move to next entry */
            caps += 2 + losc;
            caps_size -= 2 + losc;
        }
        return FALSE;
    }
    return FALSE;
}


/****************************************************************************
  NAME
      CodecSamplingRateConversion

DESCRIPTION
     To convert the actual sampling rate into the format that the codec library can understand;

*/
static void CodecSamplingRateConversion(uint32 rate, sample_freq *codec_sampling_frequency)
{
    switch (rate)
    {
        case 48000:
        {
            *codec_sampling_frequency = sample48kHz;
            break;
        }
        case 44100:
        {
            *codec_sampling_frequency = sample44_1kHz;
            break;
        }
        case 32000:
        {
            *codec_sampling_frequency = sample32kHz;
            break;
        }
        case 24000:
        {
            *codec_sampling_frequency = sample24kHz;
            break;
        }
        case 22050:
        {
            *codec_sampling_frequency = sample22_25kHz;
            break;
        }
        case 16000:
        default:
        {
            *codec_sampling_frequency = sample16kHz;
            break;
        }
    }
}


/****************************************************************************
  NAME
      handleA2DPCodecSettingsInd

DESCRIPTION
    Handles the A2DP library message with the newly opened media configuration.   
*/
static void handleA2DPCodecSettingsInd(devInstanceTaskData *theInst, A2DP_CODEC_SETTINGS_IND_T *msg)
{
    uint8 *codec_caps      = NULL;
    uint16 codec_caps_size = 0;      
        
    codec_data_type    codecData;
    
    switch (getA2dpState(theInst))
    {
        case A2dpStateConnected:
        case A2dpStateOpening:
        case A2dpStateOpen:
        {
            the_app->a2dp_channel_mode = msg->channel_mode;
            the_app->a2dp_sample_rate  = msg->rate;

            {
                codec_config_params config;
                sample_freq codec_sampling_frequency;
                
                config.inputs = line_input;
                config.outputs = 0;
                config.dac_sample_rate = sampleNotUsed;
                
                CodecSamplingRateConversion(msg->rate, &codec_sampling_frequency);
                config.adc_sample_rate = codec_sampling_frequency;

                CodecConfigure(the_app->codecTask, &config);
            }

            /* store the Stream End Point that is in use */
            the_app->a2dp_active_seid = msg->seid;
            
            /* store the codec configuration so it can be passed to the audio plugins */
            codecData = msg->codecData;
            the_app->a2dp_data.codecData.source_type        = 1;
            the_app->a2dp_data.codecData.content_protection = codecData.content_protection;
            the_app->a2dp_data.codecData.voice_rate  = codecData.voice_rate;
            the_app->a2dp_data.codecData.bitpool     = codecData.bitpool;
            the_app->a2dp_data.codecData.format      = codecData.format;
            the_app->a2dp_data.codecData.packet_size = codecData.packet_size;
            the_app->a2dp_data.codecData.media_sink_b = 0;
            
            DEBUG_A2DP(("CODEC SETTINGS source[%x] voice[%lx] bitpool[%x] format[%x] size[%x]:\n",
                        the_app->a2dp_data.codecData.source_type,
                        the_app->a2dp_data.codecData.voice_rate,
                        the_app->a2dp_data.codecData.bitpool,
                        the_app->a2dp_data.codecData.format,
                        the_app->a2dp_data.codecData.packet_size));

            /* Detect whether the codec is MP3 or not */
            /* For mp3 codec, extract the target bit rate */
            codec_caps = (uint8 *)malloc( msg->configured_codec_caps_size );

            if (codec_caps == NULL)
                Panic();

            if (isMP3Codec(msg->configured_codec_caps, msg->configured_codec_caps_size, codec_caps, &codec_caps_size) )
            {
                uint8 bitrate_index     = 0;
                uint8 sample_rate_index = 0;
                uint8 isVBRSupported    = 0;

                /* Get the sampling rate */
                if (codec_caps[3] > 0)
                {
                    uint8 cap7 = codec_caps[3];

                    while (cap7 >>= 1)
                        sample_rate_index++;

                    sample_rate_index = sample_rate_index_table_mp3[sample_rate_index];
                }

                /* Get the VBR/CBR status */
                if (codec_caps[4] & 0x80)
                    isVBRSupported = 1;
                else
                    isVBRSupported = 0;

                /* Get the Bit rate */
                if (codec_caps[5] > 0)
                {
                    uint8 cap7 = codec_caps[5];

                    while (cap7 >>= 1)
                        bitrate_index++;
                }
                else
                {
                    uint8 cap6 = codec_caps[4] & 0x7f;
                    bitrate_index  = 8;

                    while ((cap6 >>= 1))
                        bitrate_index++;
                }

                /* The parameters are passed to mp3 plugin through codec.format */
                /* The definition of codecData.format for mp3 is different from those for SBC and FASTSTREAM */
                the_app->a2dp_data.codecData.format = ( ((isVBRSupported    & 0x01) << POS_CODEC_SUPPORT_VBR) \
                                                  + ((sample_rate_index & 0x07) << POS_CODEC_SAMPLE_RATE_INDEX) \
                                                  + ((bitrate_index     & 0x0f)) );

                /* For mp3, the packet size is always 668 ? */
                the_app->a2dp_data.codecData.packet_size = 668;
            }

            if (codec_caps != NULL)
                free (codec_caps);
            break;
        }
        case A2dpStateDisconnecting:
        {
            /* Ignore */
            DEBUG_A2DP(("  - ignored\n"));
            break;
        }
        default:
        {
            a2dpUnhandledMessage(theInst, A2DP_CODEC_SETTINGS_IND);
            break;
        }
    }
}



/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    a2dpMsgChooseSbcAudioQuality

DESCRIPTION
    Updates the A2DP audio quality for SBC codec based on the supported features of the links.
    
*/
void a2dpMsgChooseSbcAudioQuality(devInstanceTaskData *theInst)
{   
    chooseSbcAudioQuality(theInst);
}
    

/****************************************************************************
NAME
    a2dpMsgHandleLibMessage

DESCRIPTION
    Handles initial A2DP library messages before an instance of the remote device has been created at the application level.
    Once a device instance has been created the A2DP library messages are handled by the a2dpMsgHandleInstanceMessage handler.
    
*/
void a2dpMsgHandleLibMessage(MessageId id, Message message)
{   
    switch(id)
    {
        case A2DP_INIT_CFM:
        {
            DEBUG_A2DP(("A2DP_INIT_CFM status = %u\n", ((A2DP_INIT_CFM_T *)message)->status));
            handleA2DPInitCfm((A2DP_INIT_CFM_T *) message);
            return;
        }
        case A2DP_SIGNALLING_CHANNEL_CONNECT_IND:
        {
			SendEvent(EVT_A2DP_SIGNAL_CONNECT_IND,0);
            DEBUG_A2DP(("A2DP_SIGNALLING_CHANNEL_CONNECT_IND : \n"));
            handleA2DPSignallingConnectInd((A2DP_SIGNALLING_CHANNEL_CONNECT_IND_T *) message);
            return;
        }
        default:
        {
            DEBUG_A2DP(("A2DP MESSAGE UNEXPECTED id:[0x%x]\n", id));
            return;
        }
    }
}


/****************************************************************************
NAME
    a2dpMsgHandleInstanceMessage

DESCRIPTION
    Handles A2DP library messages associated with a device instance.
    
*/
void a2dpMsgHandleInstanceMessage(devInstanceTaskData *theInst, MessageId id, Message message)
{
    /* Handle A2DP library messages */
    switch (id)
    {
        case A2DP_SIGNALLING_CHANNEL_CONNECT_IND:
        {
			SendEvent(EVT_A2DP_SIGNAL_CONNECT_IND,0);
            DEBUG_A2DP(("A2DP_SIGNALLING_CHANNEL_CONNECT_IND inst:[0x%x]\n", (uint16)theInst));
            handleA2DPSignallingConnectInd((A2DP_SIGNALLING_CHANNEL_CONNECT_IND_T *) message);
            return;
        }
        case A2DP_SIGNALLING_CHANNEL_CONNECT_CFM:
        {
			SendEvent(EVT_A2DP_SIGNAL_CONNECT_CFM,((A2DP_SIGNALLING_CHANNEL_CONNECT_CFM_T *)message)->status);
			the_app->conn_status.a2dp_con = ((A2DP_SIGNALLING_CHANNEL_CONNECT_CFM_T *)message)->status ? 0:1;
			
            DEBUG_A2DP(("A2DP_SIGNALLING_CHANNEL_CONNECT_CFM  status:[%u] inst:[0x%x]\n", ((A2DP_SIGNALLING_CHANNEL_CONNECT_CFM_T *)message)->status, (uint16)theInst));
            handleA2DPSignallingConnectCfm(theInst, (A2DP_SIGNALLING_CHANNEL_CONNECT_CFM_T *) message);
            return;
        }
        case A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND:
        {
			SendEvent(EVT_A2DP_SIGNAL_DISCONNECT_IND,((A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND_T *)message)->status);
			the_app->conn_status.a2dp_con = 0;
			the_app->conn_status.a2dp_play = 0;
			the_app->conn_status.a2dp_open = 0;
			
            DEBUG_A2DP(("\nA2DP_SIGNALLING_CHANNEL_DISCONNECT_IND  status:[%u] inst:[0x%x]\n", ((A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND_T *)message)->status, (uint16)theInst));
            handleA2DPSignallingChannelDisconnectInd(theInst, (A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND_T*)message);
            return;
        }
        case A2DP_OPEN_IND:
        {
			SendEvent(EVT_A2DP_OPEN_IND,0);
			the_app->conn_status.a2dp_open = 1;
			
            DEBUG_A2DP(("A2DP_OPEN_IND inst:[0x%x]\n", (uint16)theInst));
            handleA2DPOpenInd(theInst,  (A2DP_OPEN_IND_T *) message);
            return;
        }
        case A2DP_OPEN_CFM:
        {
			SendEvent(EVT_A2DP_OPEN_CFM,((A2DP_OPEN_CFM_T *)message)->status);
			the_app->conn_status.a2dp_open = ((A2DP_OPEN_CFM_T *)message)->status ? 0:1;
			
            DEBUG_A2DP(("A2DP_OPEN_CFM status:[%u] inst:[0x%x]\n", ((A2DP_OPEN_CFM_T *)message)->status, (uint16)theInst));
            handleA2DPOpenCfm(theInst, (A2DP_OPEN_CFM_T *)message);
            return;
        }
        case A2DP_CONNECT_OPEN_CFM:
        {
			SendEvent(EVT_A2DP_SIGNAL_CONNECT_CFM,0);
			the_app->conn_status.a2dp_con = 1;
			SendEvent(EVT_A2DP_OPEN_CFM,((A2DP_CONNECT_OPEN_CFM_T *)message)->status);
			the_app->conn_status.a2dp_open = ((A2DP_CONNECT_OPEN_CFM_T *)message)->status ? 0:1;

            DEBUG_A2DP(("A2DP_CONNECT_OPEN_CFM status[%u] inst:[0x%x]\n", ((A2DP_CONNECT_OPEN_CFM_T *)message)->status, (uint16)theInst));
            handleA2DPConnectOpenCfm(theInst, (A2DP_CONNECT_OPEN_CFM_T*)message);
            return;
        }
        case A2DP_START_IND:
        {
			SendEvent(EVT_A2DP_START_IND,0);
			the_app->conn_status.a2dp_play = 1;
            DEBUG_A2DP(("A2DP_START_IND inst:[0x%x]\n", (uint16)theInst));
            handleA2DPStartInd(theInst, (A2DP_START_IND_T*)message);
            return;
        }
        case A2DP_START_CFM:
        {
			SendEvent(EVT_A2DP_START_CFM,((A2DP_START_CFM_T *)message)->status);
			the_app->conn_status.a2dp_play = ((A2DP_START_CFM_T *)message)->status ? 0:1;
			
            DEBUG_A2DP(("A2DP_START_CFM status[%u] inst:[0x%x]\n", ((A2DP_START_CFM_T *)message)->status, (uint16)theInst));
            handleA2DPStartCfm(theInst, (A2DP_START_CFM_T*)message);
            return;
        }
        case A2DP_SUSPEND_IND:
        {
			SendEvent(EVT_A2DP_SUSPEND_IND,0);
			the_app->conn_status.a2dp_play = 0;
			
            DEBUG_A2DP(("A2DP_SUSPEND_IND inst:[0x%x]\n", (uint16)theInst));
            handleA2DPSuspendInd(theInst, (A2DP_SUSPEND_IND_T*)message);
            return;
        }
        case A2DP_SUSPEND_CFM:
        {
			SendEvent(EVT_A2DP_SUSPEND_CFM,((A2DP_SUSPEND_CFM_T *)message)->status);
			the_app->conn_status.a2dp_play = ((A2DP_SUSPEND_CFM_T *)message)->status ? 1:0
			
            DEBUG_A2DP(("A2DP_SUSPEND_CFM status[%u] inst:[0x%x]\n", ((A2DP_SUSPEND_CFM_T *)message)->status, (uint16)theInst));
            handleA2DPSuspendCfm(theInst, (A2DP_SUSPEND_CFM_T*)message);
            return;
        }
        case A2DP_CLOSE_IND:
        {
            DEBUG_A2DP(("A2DP_CLOSE_IND status:[%u] inst:[0x%x]\n", ((A2DP_CLOSE_IND_T *)message)->status, (uint16)theInst));
            handleA2DPCloseInd(theInst, (A2DP_CLOSE_IND_T *)message);
            return;
        }
        case A2DP_CLOSE_CFM:
        {
            DEBUG_A2DP(("A2DP_CLOSE_CFM status:[%u] inst:[0x%x]\n", ((A2DP_CLOSE_CFM_T *)message)->status, (uint16)theInst));
            handleA2DPCloseCfm(theInst, (A2DP_CLOSE_CFM_T *)message);
            return;
        }
        case A2DP_CONFIGURE_CODEC_IND:
        {
            DEBUG_A2DP(("A2DP_CONFIGURE_CODEC_IND inst:[0x%x]\n", (uint16)theInst));
            handleA2DPConfigureCodecInd(theInst, (A2DP_CONFIGURE_CODEC_IND_T*)message);
            return;
        }
        case A2DP_CODEC_SETTINGS_IND:
        {
            DEBUG_A2DP(("A2DP_CODEC_SETTINGS_IND inst:[0x%x]\n", (uint16)theInst));
            handleA2DPCodecSettingsInd(theInst, (A2DP_CODEC_SETTINGS_IND_T*)message);
            return;
        }
        case A2DP_ENCRYPTION_CHANGE_IND:
        {
            DEBUG_A2DP(("A2DP_ENCRYPTION_CHANGE_IND\n"));
            return;
        }
        default:
        {
            a2dpUnhandledMessage(theInst, id);           
            return;
        }
    }
}

