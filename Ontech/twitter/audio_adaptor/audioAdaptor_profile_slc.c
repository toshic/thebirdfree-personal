/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    General profile connection functionality.
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_aghfp_slc.h"
#include "audioAdaptor_a2dp_slc.h"
#include "audioAdaptor_avrcp_slc.h"
#include "audioAdaptor_events.h"
#include "audioAdaptor_scan.h"
#include "audioAdaptor_profile_slc.h"
#include "audioAdaptor_event_handler.h"
#include "audioAdaptor_dev_instance.h"
#include "audioAdaptor_statemanager.h"
#include "audioAdaptor_a2dp_stream_control.h"
#include "audioAdaptor_init.h"

#include <string.h>
#include <panic.h>
#include <stdlib.h>


#define MAX_PROFILE_CONNECT_PERIOD_MS  20000
#define MAX_CONNECT_PERIOD_TIME_MS     10500
#define MAX_RECONNECT_PERIOD_TIME_MS   15000        


/****************************************************************************
  LOCAL FUNCTIONS
*/

/****************************************************************************
NAME
    startAppProfileConnectTimer

DESCRIPTION
    Start the profile connection timer. This is the max time allowed for a profile to connect.
*/
static void startAppProfileConnectTimer (devInstanceTaskData *inst)
{
    DEBUG_CONN(("APP_PROFILE_CONNECT_TIME Start\n"));
    MessageCancelAll(&inst->task, APP_PROFILE_CONNECT_TIMER);
    MessageSendLater(&inst->task, APP_PROFILE_CONNECT_TIMER, 0, MAX_PROFILE_CONNECT_PERIOD_MS);
}


/****************************************************************************
NAME
    stopAppProfileConnectTimer

DESCRIPTION
    Stop the profile connection timer.
*/
static void stopAppProfileConnectTimer (devInstanceTaskData *inst)
{
    DEBUG_CONN(("APP_PROFILE_CONNECT_TIME Cancelled during profile %x connection\n", the_app->connecting_profile));
    MessageCancelAll(&inst->task, APP_PROFILE_CONNECT_TIMER);
    the_app->connecting_profile = ProfileNone;
}

#if 0
/****************************************************************************
NAME
    startAppConnectTimer

DESCRIPTION
    Start the application connection timer. This is the max time allowed
    for the remote device to connect before the local device takes over connection.
*/
static void startAppConnectTimer (devInstanceTaskData *inst)
{
    MessageCancelAll(&inst->task, APP_CONNECT_TIMER);
    MessageSendLater(&inst->task, APP_CONNECT_TIMER, 0, MAX_CONNECT_PERIOD_TIME_MS);
    inst->connect_timer_expired = FALSE;
}
#endif

/****************************************************************************
NAME
    stopAppConnectTimer

DESCRIPTION
    Stops the application connection timer.
*/
static void stopAppConnectTimer (devInstanceTaskData *inst)
{
    MessageCancelAll(&inst->task, APP_CONNECT_TIMER);
    inst->connect_timer_expired = FALSE;
}


/****************************************************************************
NAME
    getNumberDevicesConnected

DESCRIPTION
    Returns how many separate devices are connected with at least one profile.
    
*/
static uint16 getNumberDevicesConnected(void)
{
    devInstanceTaskData *inst;
    uint16 i, count = 0;        
 
    /* Look at all possible connections */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {                       
            if (inst->available_profiles_connected)
                count++;
        }
    }

    return count;
}


/****************************************************************************
NAME
    appDisconnectCfm

DESCRIPTION
    A device has completely disconnected.
    
*/
static void appDisconnectCfm (devInstanceTaskData *inst, bool status)
{
    DEBUG_CONN(("appDisconnectCfm status=%u\n",!status));
    
    if (profileSlcCheckPoweredOff(inst))
        return;
    
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        case AppStateConnecting:
        case AppStateStreaming:
        case AppStateInCall:
        {
            /* Successfully disconnected from all profiles */
            
            /* Only set state to idle if all devices are disconnected */
            if (profileSlcAreAllDevicesDisconnected())
                setAppState(AppStateIdle);        
                    
            (the_app->comm_action == CommActionDiscover) ? scanMakeDiscoverable() : scanMakeConnectable();
            
            if (the_app->disconnect_requested)
            {                          
                /* Wait for all devices to be disconnected */
                if (profileSlcAreAllDevicesDisconnected())
                {
                    the_app->disconnect_requested = FALSE;
                    if (the_app->auto_connect_in_progress)
                    {                 
                        kickCommAction(CommActionConnect);
                        the_app->pending_comm_action = CommActionConnect;                  
                    }
                    else
                    {
                        kickCommAction(the_app->comm_action);
                    }
                }
            }
            else
            {    
                kickCommAction(the_app->comm_action);
            }
            
            break;
        }
        default:
        {
            DEBUG_CONN((" - IGNORED\n"));
            break;
        }
    }
}


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    profileSlcConnectReq

DESCRIPTION
    Connect to the device with the Bluetooth address stored in the_app->connect_bdaddr.
    
*/
void profileSlcConnectReq (void)
{
    devInstanceTaskData *inst;
    
    DEBUG_CONN(("profileSlcConnectReq 0x%X 0x%X 0x%lX\n", the_app->connect_bdaddr.nap, the_app->connect_bdaddr.uap, the_app->connect_bdaddr.lap));
    
    if (profileSlcAreAnyProfilesConnecting())
    {
        DEBUG_CONN(("    - Profile already connecting\n"));
        return;
    }

    switch ( the_app->app_state )
    {       
        case AppStateIdle:
        case AppStateConnecting:    
        {
            inst = devInstanceFindFromBddr(&the_app->connect_bdaddr, TRUE);
            
            if (inst == NULL)
            {
                DEBUG_CONN((" - Max connections reached\n"));
                /* Kick the stored action, as connection couldn't be initiated */
                eventHandleSendKickCommActionMessage(the_app->comm_action);
                return;
            }                      
            
            /* Connecting all profiles simultaneously can cause IOP issues with a number of existing stereo headsets. */
            /* Check remote_profiles bit mask for support by remote device */
            /* Check responding_profiles bit mask for a previous connect attempt - if remote device has initiated connection that will be */
            /* detected in actual profile connect function */
			if((inst->responding_profiles & ProfileAghfp) == 0)
            {
                if (aghfpSlcConnect(inst))
                {
                    setAppState(AppStateConnecting);
                    the_app->s_connect_attempts  = 0;
                    startAppProfileConnectTimer(inst);
                    the_app->connecting_profile =  ProfileAghfp;
                    break;
                }            
            }
            
			if((inst->responding_profiles & ProfileA2dp) == 0)
            { 
                /* start a2dp profile connection */
                if (a2dpSlcConnect(inst))
                {
                    setAppState(AppStateConnecting);
                    the_app->s_connect_attempts  = 0;
                    startAppProfileConnectTimer(inst);
                    the_app->connecting_profile  = ProfileA2dp;
                    break;
                }                        
            }
			if((inst->responding_profiles & ProfileAvrcp) == 0)
            {
                if (avrcpSlcConnect(inst))
                {
                    setAppState(AppStateConnecting);
                    the_app->s_connect_attempts  = 0;
                    startAppProfileConnectTimer(inst);
                    the_app->connecting_profile  = ProfileAvrcp;
                    break;
                }            
            }
            
            /* Connection to this device couldn't be initiated */
            {
                MAKE_APP_MESSAGE(APP_CONNECT_CFM);
                message->success = FALSE;
                MessageSend(&inst->task, APP_CONNECT_CFM, message);
                MessageSend(&inst->task, APP_INTERNAL_DESTROY_REQ, 0);
            }
            
            break;
        }    
        case AppStateStreaming:
        case AppStateInCall:
        default:
        {
            DEBUG_CONN((" - IGNORED\n"));
            break;
        }
    }
}        


/****************************************************************************
NAME
    profileSlcConnectComplete

DESCRIPTION
    A connect attempt to a certain device has completed.
    
*/
void profileSlcConnectComplete (devInstanceTaskData *inst, bool status)
{
    DEBUG_CONN(("profileSlcConnectComplete status=%u\n", !status));
    
    if (profileSlcCheckPoweredOff(inst))
        return;
    
    if (!status)
    {    /* Failed to connect to any profiles */        
        the_app->disconnect_requested = FALSE;
        inst->responding_profiles = ProfileNone;
        
        switch ( the_app->app_state )
        {
            case AppStateInquiring:
            case AppStateSearching:
            {
                /* if already inquiring or searching then take no action */
                break;
            }
            default:
            {
                if (the_app->inquiring)
                {
                    if (profileSlcAreAllDevicesDisconnected())
                        scanMakeDiscoverable();
                    setAppState(AppStateIdle);
                    kickCommAction(CommActionInquire);
                }
                else
                {
                    setAppState(AppStateIdle);
                    scanMakeConnectable();
                    if ( the_app->auto_connect_in_progress )
                    {
                        {    /* Failed to automatically connect to devices */
                            the_app->auto_connect_in_progress = FALSE;                
                            eventHandleCancelCommAction();         
                            a2dpStreamRestartAudioStream();
                        }
                    }
                    else
                    {            
                        eventHandleCancelCommAction();
                        a2dpStreamRestartAudioStream();
                    }
                }
            }
        }
    }
    else
    {    /* Successfully connected to one or more available profiles */
        inst->available_profiles_connected = TRUE;
        
        if (getNumberDevicesConnected() >= MAX_NUM_DEV_CONNECTIONS)
            scanMakeUnconnectable();
            
        MessageSendLater(&inst->task, APP_REFRESH_ENCRYPTION_REQ, 0, D_MIN(EPR_TIMEOUT));
        
        switch ( the_app->app_state )
        {
            case AppStateStreaming:
            case AppStateInCall:
            {
                eventHandleCancelCommAction();
                /* fall through */
            }
            case AppStateIdle:
            case AppStateConnecting:
            {
                setAppState(AppStateIdle);
                scanStopAppInquiryTimer();
                if ( the_app->auto_connect_in_progress )
                {
                    the_app->auto_connect_in_progress = FALSE;
                }
                if (a2dpSlcIsMediaOpen() && (the_app->audio_streaming_state != StreamingActive))
                {    
                    /* Ensure an A2dp media channel is opened on connection */
                    a2dpStreamSetAudioStreamingState(StreamingPending);
                }
                
                if (the_app->voip_call_active)
                {    
                    /* Voip call detected during connection, open a call */
                    kickCommAction(CommActionCall);
                }
                else if (the_app->audio_streaming_state != StreamingInactive)
                {
                    /* Audio streaming detected during connection, open an audio connection */
                    kickCommAction(CommActionStream);
                }
                break;
            }
            default:
            {
                DEBUG_CONN((" - IGNORED\n"));
                break;
            }
        }
    }
}


/****************************************************************************
NAME
    profileSlcConnectCfm

DESCRIPTION
    A profile connection attempt has ended.
    
*/
void profileSlcConnectCfm (devInstanceTaskData *inst, mvdProfiles profile, bool remote_connecting)
{
    bool all_responded;
    mvdProfiles connected_profiles = ProfileNone;
    
    if (profileSlcCheckPoweredOff(inst))
        return;
        
    inst->responding_profiles |= profile;    

   /* decide whether all supported profile has been responded */
   all_responded = ( (inst->responding_profiles & (ProfileAghfp | ProfileAghsp | ProfileAvrcp | ProfileA2dp)) == 
                  ( (inst->remote_profiles & (ProfileAghfp | ProfileAghsp | ProfileAvrcp | ProfileA2dp)) & 
                    (the_app->supported_profiles & (ProfileAghfp | ProfileAghsp | ProfileAvrcp | ProfileA2dp))) );
                 
   if (getAghfpState(inst) >= AghfpStateConnected)
   {
      connected_profiles |= ProfileAghfp;
   }
   if (getA2dpState(inst) >= A2dpStateOpen)
   {
      connected_profiles |= ProfileA2dp;
   }
   if (getAvrcpState(inst) == AvrcpStateConnected)
   {
      connected_profiles |= ProfileAvrcp;
   }

   DEBUG_CONN(("profileSlcConnectCfm conn'd:%x resp'd:%x all_resp'd:%x\n",connected_profiles, inst->responding_profiles, all_responded));

   /* Stop the profile connection timer */
   stopAppProfileConnectTimer( inst );    

    if ( connected_profiles==ProfileNone && all_responded ) 
    {    /* All profiles supported by remote device have responded, but none have connected */
        stopAppConnectTimer(inst);            
        profileSlcConnectComplete(inst, FALSE);
    }
    else if ( connected_profiles!=ProfileNone && all_responded )
    {    /* At least one profile is connected and all others have responded */            
        stopAppConnectTimer(inst);
        
        DEBUG_CONN(("connect_attempts=%u\n",the_app->connect_attempts));
/* don't retry to connect */
#if 0
        if ( ++the_app->connect_attempts<MAX_APP_CONNECT_ATTEMPTS && (connected_profiles != (inst->remote_profiles & the_app->supported_profiles)) ) /*&& (the_app->app_state==AppStatePaging || the_app->app_state==AppStatePaged) )*/
        {   
            /* Only keep profiles in responding bitmask that have connected */
            inst->responding_profiles = connected_profiles;
            
            /* Try to connect to the unconnected profiles again */
            profileSlcConnectReq();
        }
        else
#endif			
        {
            the_app->connect_attempts = 0;
            profileSlcConnectComplete(inst, TRUE);
        }
    }
    else        
    {    
        /* change the state to idle if the dongle was connecting */
        if (the_app->app_state == AppStateConnecting)
            setAppState(AppStateIdle);
            
        /* Still some outstanding profiles to connect/respond */
        if (remote_connecting)
        {   /* Start timer to take over connection process if necessary */
            /* when the remote device is connecting back                */
/*            startAppConnectTimer(inst);*/
        }
        else
        {   /* Kick off connection of next profile */
#if 1
            profileSlcConnectReq();
#else       /* this condition is only for test remote reaction */  
            MessageCancelAll(&inst->task, APP_LATE_CONNECT_TIMER);
            MessageSendLater(&inst->task, APP_LATE_CONNECT_TIMER, 0, D_SEC(5));
#endif            
        }
    }
}


/****************************************************************************
NAME
    profileSlcAcceptConnectInd

DESCRIPTION
    Act on an accepted incoming connection.
    
*/
void profileSlcAcceptConnectInd (devInstanceTaskData *inst)
{
    inst->paired_list_read = TRUE;
    the_app->connect_attempts = 0;
    
    /* cancel any active inquiry */
    scanCancelInquiryScan();
}


/****************************************************************************
NAME
    profileSlcDisconnectInd

DESCRIPTION
    Act on a profile disconnection.
    
*/
void profileSlcDisconnectInd (devInstanceTaskData *inst, mvdProfiles profile)
{
    bool disconnected;

    switch (profile)
    {
        case ProfileAghsp:
        case ProfileAghfp:
        {
            break;
        }
        case ProfileA2dp:
        {
            break;
        }
        case ProfileAvrcp:
        {
            break;
        }
        default:
        {
            break;
        }
    }
    
    disconnected = profileSlcAreAllProfilesDisconnected(inst);
    
    if (disconnected)
    {    /* All profiles have disconnected from the remote device. */
        
        inst->available_profiles_connected = FALSE;
        
        appDisconnectCfm(inst, TRUE);
    }
}


/****************************************************************************
NAME
    profileSlcStartConnectionProcess

DESCRIPTION
    Start of connect procedure following power on.
    
*/
void profileSlcStartConnectionProcess (void)
{
    setAppState(AppStateIdle);
    
    /* Start the DSP */ 
    a2dpStreamStartDsp(FALSE);
    
#if 1   
     {    /* Attempt to connect to last known device */
        the_app->auto_connect_in_progress = TRUE;
        scanMakeConnectable();
        kickCommAction(CommActionConnect);
    }
 #endif
    
    scanMakeConnectable();
    
}


/****************************************************************************
NAME
    profileSlcCancelConnectionAttempt

DESCRIPTION
    Cancel a connection attempt due to unresponsive device.
    
*/
void profileSlcCancelConnectionAttempt(devInstanceTaskData *inst)
{
    stopAppConnectTimer(inst);            
    profileSlcConnectComplete(inst, FALSE);
}


/****************************************************************************
NAME
    profileSlcAreAnyProfilesConnecting

DESCRIPTION
    Test to see if there are any incoming or outgoing connections active.
    
*/
bool profileSlcAreAnyProfilesConnecting(void)
{
    devInstanceTaskData *inst;
    uint16 i;        
 
    /* Look at all possible connections */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {           
            /* See if HF is connecting */
            if ((getAghfpState(inst) == AghfpStatePaging) || (getAghfpState(inst) == AghfpStatePaged))
                return TRUE;
            /* See if A2DP is connecting */
            if ((getA2dpState(inst) == A2dpStateOpening) || (getA2dpState(inst) == A2dpStatePaged))
                return TRUE; 
            /* See if AVRCP is connecting */
            if ((getAvrcpState(inst) == AvrcpStatePaging) || (getAvrcpState(inst) == AvrcpStatePaged))
                return TRUE; 
        }
    }

    return FALSE;
}


/****************************************************************************
NAME
    profileSlcAreAllDevicesDisconnected

DESCRIPTION
    Test to see if there are no active connections.
    
*/
bool profileSlcAreAllDevicesDisconnected(void)
{
    devInstanceTaskData *inst;
    uint16 i;
  
    /* Look at all possible connections */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {           
            /* See if HF is disconnected */
            if (getAghfpState(inst) != AghfpStateDisconnected)
                 return FALSE; 
            /* See if A2DP is disconnected */
            if (getA2dpState(inst) != A2dpStateDisconnected)
                return FALSE; 
            /* See if AVRCP is disconnected */
            if (getAvrcpState(inst) != AvrcpStateDisconnected)
                return FALSE; 
        }
    }

    return TRUE;
}


/****************************************************************************
NAME
    profileSlcAreAllProfilesDisconnected

DESCRIPTION
    Test to see if there are no active connections to a specified remote device.
    
*/
bool profileSlcAreAllProfilesDisconnected(devInstanceTaskData *inst)
{
    if (inst != NULL)
    {           
        /* See if HF is disconnected */
        if (getAghfpState(inst) != AghfpStateDisconnected)
             return FALSE; 
        /* See if A2DP is disconnected */
        if (getA2dpState(inst) != A2dpStateDisconnected)
            return FALSE; 
        /* See if AVRCP is disconnected */
        if (getAvrcpState(inst) != AvrcpStateDisconnected)
            return FALSE; 
    }

    return TRUE;
}


/****************************************************************************
NAME
    profileSlcAreAnyRemoteDevicesConnecting

DESCRIPTION
    Test to see if there are incoming connections.
    
*/
bool profileSlcAreAnyRemoteDevicesConnecting(void)
{
    devInstanceTaskData *inst;
    uint16 i;        
 
    /* Look at all possible connections */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {           
            /* See if HF is connecting */
            if (getAghfpState(inst) == AghfpStatePaging)
                return TRUE;
            /* See if A2DP is connecting */
            if (getA2dpState(inst) == A2dpStateOpening)
                return TRUE; 
            /* See if AVRCP is connecting */
            if (getAvrcpState(inst) == AvrcpStatePaging)
                return TRUE; 
        }
    }

    return FALSE;
}


/****************************************************************************
NAME
    profileSlcAreAllDevicesNotConnected

DESCRIPTION
    Test to see that all profiles are not currently connected.
    
*/
bool profileSlcAreAllDevicesNotConnected(void)
{
    devInstanceTaskData *inst;
    uint16 i;
    mvdAghfpState aghfp_state;
    mvdA2dpState a2dp_state;
    mvdAvrcpState avrcp_state;
  
    /* Look at all possible connections */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {           
            aghfp_state = getAghfpState(inst);
            a2dp_state = getA2dpState(inst);
            avrcp_state = getAvrcpState(inst);
            
            /* See if HF is disconnected */
            if ((aghfp_state != AghfpStateDisconnected) && (aghfp_state != AghfpStatePaging) && (aghfp_state != AghfpStatePaged))
                 return FALSE; 
            /* See if A2DP is disconnected */
            if ((a2dp_state != A2dpStateDisconnected) && (a2dp_state != A2dpStatePaged) && (a2dp_state != A2dpStateOpening))
                return FALSE; 
            if ((a2dp_state == A2dpStateOpening) && inst->a2dp_sig_sink)
                return FALSE;
            /* See if AVRCP is disconnected */
            if ((avrcp_state != AvrcpStateDisconnected) && (avrcp_state != AvrcpStatePaging) && (avrcp_state != AvrcpStatePaged))
                return FALSE; 
        }
    }

    return TRUE;
}


/****************************************************************************
NAME
    profileSlcCheckPoweredOff

DESCRIPTION
    If the device has been powered off, should make sure there are no connections.
    
*/
bool profileSlcCheckPoweredOff(devInstanceTaskData *inst)
{
    if (!the_app->audioAdaptorPoweredOn)
    {
        DEBUG_CONN(("    Already powered off\n"));
        scanMakeUnconnectable();
        
        if (inst != NULL)
        {
            stopAppProfileConnectTimer(inst);
            stopAppConnectTimer(inst);
        }
        
        eventHandleCancelCommAction();
        scanCancelInquiryScan();
        setAppState(AppStateIdle); 
        setAppState(AppStatePoweredOff); 
        if (!profileSlcAreAllDevicesDisconnected())
        {
            kickCommAction(CommActionDisconnect);
        }
        return TRUE;
    }
    return FALSE;
}
