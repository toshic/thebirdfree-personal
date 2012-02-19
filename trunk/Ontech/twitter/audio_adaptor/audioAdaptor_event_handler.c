/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handle application events.
*/


#include "audioAdaptor_private.h"
#include "audioAdaptor_aghfp_slc.h"
#include "audioAdaptor_aghfp_call.h"
#include "audioAdaptor_a2dp_slc.h"
#include "audioAdaptor_avrcp_slc.h"
#include "audioAdaptor_events.h"
#include "audioAdaptor_event_handler.h"
#include "audioAdaptor_profile_slc.h"
#include "audioAdaptor_scan.h"
#include "audioAdaptor_init.h"
#include "audioAdaptor_dev_instance.h"
#include "audioAdaptor_statemanager.h"
#include "audioAdaptor_streammanager.h"
#include "audioAdaptor_a2dp_stream_control.h"
#include "audioAdaptor_powermanager.h"
#include "leds.h"
#include "handle_sync.h"

#include <ps.h>
#include <pio.h>
#include <boot.h>
#include <bdaddr.h>
#include <string.h>
#include <panic.h>
#include <stdlib.h>
#include <kalimba.h>
#include <connection.h>


#if defined ENABLE_DEBUG && defined ENABLE_DEBUG_EVENT
    static const char * const s_app_states[] =
    {
        "AppStateUninitialised",
        "AppStateInitialising",
        "AppStateIdle",
        "AppStateInquiring",
        "AppStateSearching",
        "AppStateConnecting",
		"AppStateConnected",
        "AppStateStreaming",
        "AppStateInCall",
        "AppStateEnteringDfu",
        "AppStateLowBattery",
        "AppStatePoweredOff"
    };
    
    static const char * const s_comm_action_names[] =
    {
        "None",
        "Call",
        "EndCall",
        "Stream",
        "EndStream",
        "Connect",        
        "Disconnect",
        "Inquire",
        "Discover",
        "DiscoverWhileConnected"
    };
#endif    



/****************************************************************************
NAME
    appCallOpenReq

DESCRIPTION
    Request to create a call.
    
*/
static void appCallOpenReq (devInstanceTaskData *inst)
{
    DEBUG_EVENT(("appCallOpenReq app_state:%x\n", the_app->app_state));
    switch ( the_app->app_state )
    {
        case AppStateStreaming:
        {
            if ( the_app->active_encoder!=EncoderSco && the_app->active_encoder!=EncoderWbs )
            {
                DEBUG_EVENT((" - IGNORED, ENCODER INVALID\n"));
                break;
            }
            /* else fall through to AppStateConnected case */
        }    
        case AppStateIdle:
        {
            setAppState(AppStateInCall);
            aghfpCallCreate(inst, the_app->call_type);
            break;
        }    
        default:
        {
            DEBUG_EVENT((" - IGNORED\n"));
            break;
        }
    }
}


/****************************************************************************
NAME
    appCallCloseReq

DESCRIPTION
    Request to end a call.
    
*/
static void appCallCloseReq (void)
{
    DEBUG_EVENT(("appCallCloseReq app_state:%x\n", the_app->app_state));
    switch ( the_app->app_state )
    {
        case AppStateInCall:
        {
            aghfpCallEnd();
            break;
        }    
        default:
        {
            DEBUG_EVENT((" - IGNORED\n"));
            break;
        }
    }
}

        
/****************************************************************************
NAME
    checkPendingCommAction

DESCRIPTION
    Returns the pending comm action in comm_action and resets the current and
    pending comm actions.
    
*/
static bool checkPendingCommAction (mvdCommAction *comm_action)
{
    the_app->comm_action = CommActionNone;
    if ( the_app->pending_comm_action!=CommActionNone )
    {
        DEBUG_EVENT(("checkPendingCommAction [%s]\n",s_comm_action_names[the_app->pending_comm_action]));
        *comm_action = the_app->pending_comm_action;
        the_app->pending_comm_action = CommActionNone;
        
        return TRUE;
    }
    
    return FALSE;
}


/****************************************************************************
NAME
    appStreamOpenReq

DESCRIPTION
    Request to stream over A2DP or (e)SCO connection.
    
*/
static void appStreamOpenReq (void)
{
    devInstanceTaskData *inst;
    
    DEBUG_EVENT(("appStreamOpenReq AppState(%s), Active encoder:%x\n",s_app_states[the_app->app_state], the_app->active_encoder));
    
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        {
            if ( the_app->active_encoder == EncoderNone )
            {    /* Not already streaming audio */
                if(a2dpSlcIsMediaOpen())
                {
                    a2dpStreamStartA2dp();
                }
                else if ((inst = aghfpSlcGetConnectedHF()) != NULL)
                {
                    setAppState(AppStateStreaming);
                    aghfpSlcAudioOpen(inst);
                }
                else
                {    /* No suitable profile to use for audio streaming */
                }
            }
            else
            {
                /* There's an active encoder so the app state must be updated to streaming */
                setAppState(AppStateStreaming);
                /* Move any A2DP devices that are in the open state to streaming state */
                a2dpStreamStartA2dp();
            }
            break;
        }    
        default:
        {
            DEBUG_EVENT((" - IGNORED\n"));
            break;
        }    
    }
}


/****************************************************************************
NAME
    appStreamCloseReq

DESCRIPTION
    Request to close any open streams over A2DP or (e)SCO connection.
    
*/
static void appStreamCloseReq (void)
{
    DEBUG_EVENT(("appStreamCloseReq AppState(%s), Active encoder:%x\n",s_app_states[the_app->app_state], the_app->active_encoder));
    
    switch ( the_app->app_state )
    {
        case AppStateStreaming:
        {
            switch ( the_app->active_encoder )
            {
                case EncoderNone:
                {
                    MessageSend(&the_app->task, APP_STREAM_CLOSE_COMPLETE, 0);            
                    break;
                }    
                case EncoderAv:
                case EncoderAnalog:
                {
                    a2dpStreamCeaseA2dpStreaming();
                    break;
                }
                case EncoderSco:
                case EncoderWbs:
                {
                    aghfpSlcAudioClose();
                    break;
                }
            }
            break;
        }    
        default:
        {
            DEBUG_EVENT((" - IGNORED\n"));
            break;
        }
    }
}


/****************************************************************************
NAME
    appDisconnectReq

DESCRIPTION
    Disconnects all active connections.
    
*/
static void appDisconnectReq (void)
{            
    DEBUG_EVENT(("appDisconnectReq AppState(%s)\n",s_app_states[the_app->app_state]));
    
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        {
            the_app->disconnect_requested = TRUE;
            avrcpSlcDisconnectAll();
            a2dpSlcDisconnectAll(); 
            aghfpSlcDisconnectAll();
            break;
        }
        default:
        {
            DEBUG_EVENT((" - IGNORED\n"));
            break;
        }
    }
}


/****************************************************************************
NAME
    commActionNone

DESCRIPTION
    Resets current and pending comm actions.
    
*/
static void commActionNone(void)
{
    /* Cancel any queued action and do nothing */
    the_app->comm_action = CommActionNone;
    the_app->pending_comm_action = CommActionNone;
}


/****************************************************************************
NAME
    commActionCall

DESCRIPTION
    Handles create call comm action.
    
*/
static bool commActionCall(mvdCommAction *comm_action)
{
    devInstanceTaskData *inst = NULL;
    bool command_to_process = FALSE;    
    
    switch (the_app->app_state)
    {
        case AppStateIdle:
        {
            /* See if a HandsFree device is connected. This assumes that a maximum of 1 can be connected. */
            if ((inst = aghfpSlcGetConnectedHF()) != NULL)
            {    /* HF is connected so create a new call */
                the_app->comm_action = CommActionCall;
                appCallOpenReq(inst);
            } 
            /* TODO - can create a connection if one doesn't exist */
            else
            {    /* No last paired device, ignore this command and check for any queued action */
                command_to_process = checkPendingCommAction(comm_action);
            }
            break;    
        }
        case AppStateStreaming:
        {
            if ( ((inst = aghfpSlcGetConnectedHF()) != NULL) && (the_app->active_encoder==EncoderSco || the_app->active_encoder==EncoderWbs) )
            {    /* Create a new call using the existing SCO/WBS audio connection */
                DEBUG_EVENT(("kick Call Open\n"));
                the_app->comm_action = CommActionCall;
                appCallOpenReq(inst);
            }
            else
            {    /* Stop streaming audio so that a call can be created */
                DEBUG_EVENT(("kick Stop streaming\n"));
                the_app->comm_action = CommActionCall;
                appStreamCloseReq();
            }
            break;
        }
        case AppStateInCall:
        {
            /* In correct state, check for any queued action */
            command_to_process = checkPendingCommAction(comm_action);
            break;
        }
        default:
        {
            /* Command issued in wrong state, queue it */
            the_app->pending_comm_action = CommActionCall;
            break;
        }
    }
    
    return command_to_process;
}


/****************************************************************************
NAME
    commActionEndCall

DESCRIPTION
    Handles end call comm action.
    
*/
static bool commActionEndCall(mvdCommAction *comm_action)
{
    bool command_to_process = FALSE;    
    mvdCommAction last_comm_action = the_app->comm_action;
    
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        {
            if ( the_app->voip_call_active && (last_comm_action==*comm_action || last_comm_action==CommActionNone) )
            {    /* New voip call detected during shutdown of last, open a new call */
                the_app->pending_comm_action = CommActionCall;
            }
#if 0			
            else if ( the_app->audio_streaming_state!=StreamingInactive && (last_comm_action==*comm_action || last_comm_action==CommActionNone) )
            {    /* Audio streaming detected during shutdown of call, open an audio connection */
                the_app->pending_comm_action = CommActionStream;
            }
#endif			
            command_to_process = checkPendingCommAction(comm_action);
            break;
        }
        case AppStateStreaming:
        {
            /* Not in a call.  Ignore request and check for any queued action */
            command_to_process = checkPendingCommAction(comm_action);
            break;
        }
        case AppStateInCall:
        {
            /* Close the current call */
            the_app->comm_action = *comm_action;
            appCallCloseReq();
            break;
        }
        default:
        {
            /* Command issued in wrong state, queue it */
            the_app->pending_comm_action = *comm_action;
            break;
        }
    }
    
    return command_to_process;
}


/****************************************************************************
NAME
    commActionStream

DESCRIPTION
    Handles create stream comm action.
    
*/
static bool commActionStream(mvdCommAction *comm_action)
{
    bool command_to_process = FALSE;
    
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        {
            if ((aghfpSlcGetConnectedHF() != NULL) || a2dpSlcIsMediaOpen())
            {    /* Start streaming audio */
                the_app->comm_action = *comm_action;
                appStreamOpenReq();
            }
            /* TODO - can create a connection if one doesn't exist */
            else
            {    /* No last paired device, ignore this command and check for any queued action */
                command_to_process = checkPendingCommAction(comm_action);
            }
            break;        
        }
        case AppStateStreaming:
        {
            /* Move any A2DP devices that are in the open state to streaming state */
            a2dpStreamStartA2dp();
            /* In correct state, check for any queued action */
            command_to_process = checkPendingCommAction(comm_action);
            break;
        }
        case AppStateInCall:
        {
            /* End the current call to allow streaming to occur */
            the_app->comm_action = *comm_action;
            appCallCloseReq();
            break;
        }
        default:
        {
            /* Command issued in wrong state, queue it */
            the_app->pending_comm_action = *comm_action;
            break;
        }
    }
    
    return command_to_process;
}


/****************************************************************************
NAME
    commActionEndStream

DESCRIPTION
    Handles end stream comm action.
    
*/
static bool commActionEndStream(mvdCommAction *comm_action)
{
    bool command_to_process = FALSE;
    mvdCommAction last_comm_action = the_app->comm_action;
    
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        {
            if ( the_app->voip_call_active && (last_comm_action==*comm_action || last_comm_action==CommActionNone) )
            {    /* Voip call detected during close of audio connection, open a call */
                the_app->pending_comm_action = CommActionCall;
            }
            else if ( the_app->audio_streaming_state!=StreamingInactive && (last_comm_action==*comm_action || last_comm_action==CommActionNone) )
            {    /* Audio streaming detected while closing last audio connection, open another audio connection */
                the_app->pending_comm_action = CommActionStream;
            }
            command_to_process = checkPendingCommAction(comm_action);
            break;
        }
        case AppStateStreaming:
        {
            /* Stop streaming */
            the_app->comm_action = *comm_action;
            appStreamCloseReq();
            break;
        }
        case AppStateInCall:
        {
            /* Not streaming.  Ignore request and check for any queued action */
            command_to_process = checkPendingCommAction(comm_action);
            break;
        }
        default:
        {
            /* Command issued in wrong state, queue it */
            the_app->pending_comm_action = *comm_action;
            break;
        }
    }
    
    return command_to_process;
}


/****************************************************************************
NAME
    commActionConnect

DESCRIPTION
    Handles connect comm action. Connects to device with Bluetooth address of
    the_app->connect_bdaddr.
    
*/
static bool commActionConnect(mvdCommAction *comm_action)
{
    bool command_to_process = FALSE;
    
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        {
            if (profileSlcAreAnyRemoteDevicesConnecting())
            {            
                break;
            }   
            
            /* Attempt to connect to either current or last known device */
            if (!BdaddrIsZero(&the_app->connect_bdaddr))
            {         
                the_app->comm_action = *comm_action;
                the_app->connect_attempts = 0;
                profileSlcConnectReq();                                                              
            }
            
            /* No last paired device, ignore this command and check for any queued action */
            command_to_process = checkPendingCommAction(comm_action);
            break;
        }    
        case AppStateConnecting:
        {
            /* Already connecting - ignore */
            break;
        }        
        case AppStateStreaming:
        case AppStateInCall:
        {
            /* In correct state, check for any queued action */
            command_to_process = checkPendingCommAction(comm_action);
            break;
        }
        default:
        {
            /* Command issued in wrong state, queue it */
            the_app->pending_comm_action = *comm_action;
            break;
        }
    }    
    return command_to_process;
}


/****************************************************************************
NAME
    commActionDisconnect

DESCRIPTION
    Handles disconnect comm action. Disconnects all active connections.
    
*/
static bool commActionDisconnect(mvdCommAction *comm_action)
{
    bool command_to_process = FALSE;
    
    switch ( the_app->app_state )
    {            
        case AppStateIdle:
        {
            if (profileSlcAreAllDevicesNotConnected())
            {
                command_to_process = checkPendingCommAction(comm_action);
            }
            else
            {
                /* Close the SLC connection */
                the_app->comm_action = *comm_action;
                appDisconnectReq();
            }
            break;        
        }
        case AppStateStreaming:
        {
            /* Suspend A2DP streaming before disconnecting profiles */
            the_app->comm_action = *comm_action;
            appStreamCloseReq();
            break;
        }
        case AppStateInCall:
        {
            /* End the current call before closing the SLC connection */
            the_app->comm_action = *comm_action;
            appCallCloseReq();
            break;
        }
        default:
        {
            /* Command issued in wrong state, queue it */
            the_app->pending_comm_action = *comm_action;
            break;
        }
    }
        
    return command_to_process;
}


/****************************************************************************
NAME
    commActionInquire

DESCRIPTION
    Handles inquire comm action.
    
*/
static bool commActionInquire(mvdCommAction *comm_action)
{
    bool command_to_process = FALSE;
    
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        case AppStateConnecting:
        {
            if ( scanHaveInquireResults() )
            {    /* Still have some inquire results to process */
               the_app->comm_action = *comm_action;
               scanProcessNextInquireResult();
            }
            else
            {    /* Start searching for a suitable device to connect to */
               the_app->comm_action = *comm_action;
               scanInquiryScanReq();
            }
            break;
            /* fall through if connected */
        }
        case AppStateStreaming:
        case AppStateInCall:
        {
            /* Already connected.  Ignore and check for a pending command */
            command_to_process = checkPendingCommAction(comm_action);
            break;
        }
        default:
        {
            /* Command issued in wrong state, queue it */
            the_app->pending_comm_action = *comm_action;
            break;
        }
    }
    return command_to_process;
}


/****************************************************************************
NAME
    commActionDiscover

DESCRIPTION
    Handles discover comm action. Will disconnect before starting a discovery.
    
*/
static bool commActionDiscover(mvdCommAction *comm_action)
{
    bool command_to_process = FALSE;
    
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        case AppStateConnecting:
        {
            if (profileSlcAreAllDevicesNotConnected())
            {
                   /* Ensure inquire is started afresh */
                scanFlushInquireResults();
                /* Start searching for a suitable device to connect to by issuing a CommActionInquire*/
                the_app->pending_comm_action = CommActionInquire;
                command_to_process = checkPendingCommAction(comm_action);
            }
            else
            {            
                /* Close SLC connection before starting search for a suitable device to connect to */
                the_app->comm_action = *comm_action;
                appDisconnectReq();
            }
            break;
        }
        case AppStateStreaming:
        {
            /* Suspend A2DP streaming before disconnecting profiles */
            the_app->comm_action = *comm_action;
            appStreamCloseReq();
            break;
        }
        case AppStateInCall:
        {
            /* End call before disconnecting profiles */
            the_app->comm_action = *comm_action;
            appCallCloseReq();
            break;
        }
        default:
        {
            /* Command issued in wrong state, queue it */
            the_app->pending_comm_action = *comm_action;
            break;
        }
    }
        
    return command_to_process;
}    


/****************************************************************************
NAME
    commActionDiscoverWhileConnected

DESCRIPTION
    Handles discover comm action. Will perform discovery while still connected.
    
*/
static bool commActionDiscoverWhileConnected(mvdCommAction *comm_action)
{        
    bool command_to_process = FALSE;
    
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        {
            /* Ensure inquire is started afresh */
            scanFlushInquireResults();
            /* Start searching for a suitable device to connect to */
            the_app->comm_action = CommActionInquire;
            scanInquiryScanReq();
            break;
        }
        case AppStateStreaming:
        {
            /* Suspend A2DP streaming before disconnecting profiles */
            the_app->comm_action = *comm_action;
            appStreamCloseReq();
            break;
        }
        case AppStateInCall:
        {
            /* End call before disconnecting profiles */
            the_app->comm_action = *comm_action;
            appCallCloseReq();
            break;
        }
        default:
        {
            /* Command issued in wrong state, queue it */
            the_app->pending_comm_action = *comm_action;
            break;
        }
    }
        
    return command_to_process;
}


/****************************************************************************
NAME
    unexpectedAppMessage

DESCRIPTION
    For debug purposes to track unhandled application messages.
    
*/
static void unexpectedAppMessage(MessageId id, mvdAppState state)
{
    DEBUG_EVENT(("Unexpected application specific message 0x%X in state %u\n", (uint16)id, (uint16)state));
}


/****************************************************************************
NAME
    handleAppInit

DESCRIPTION
    Handles the application initialise message.
    
*/
static void handleAppInit(void)
{
    mvdAppState app_state = the_app->app_state;
    
    switch ( app_state )
    {
        case AppStateUninitialised:
        {
            setAppState(AppStateInitialising);
            initApp();
            break;
        }    
        default:
        {
            unexpectedAppMessage(APP_INIT, app_state);
        }
    }
}    


/****************************************************************************
NAME
    handleAppInit

DESCRIPTION
    Handles the application initialised message.
*/
static void handleAppInitCfm(void)
{
    mvdAppState app_state = the_app->app_state;
    
    switch( app_state )
    {
        case AppStateInitialising:
        {
/*            ConnectionWriteClassOfDevice(AV_MAJOR_DEVICE_CLASS | AV_MINOR_HIFI | AV_COD_CAPTURE);*/
            ConnectionWriteClassOfDevice(0x40020C);           /* handset */
             /*  now allow buttons */
            pioInit(&the_app->pio_states, &the_app->task ) ;
            
            the_app->audioAdaptorPoweredOn = TRUE;  

/*			initSync();*/
			
			setAppState(AppStateIdle);
			UartPrintf("\r\nReady\r\n");
             break;
        }            
        default:
        {
            unexpectedAppMessage(APP_INIT_CFM, app_state);
        }
    }    
}


/****************************************************************************
NAME
    handleAppPowerOnReq

DESCRIPTION
    Handles the application power on request message.
    
*/
static void handleAppPowerOnReq(void)
{
    if(!(the_app->audioAdaptorPoweredOn) )
    {
        DEBUG_EVENT(("Powering on...\n"));
        /* Cancel the event message if there was one so it doesn't power off */
        MessageCancelAll ( &the_app->task, APP_LIMBO_TIMEOUT ) ;
        the_app->audioAdaptorPoweredOn = TRUE;
        the_app->PowerOffIsEnabled     = TRUE;
        
        /* Power on the Analogue Dongle when APP_POWERON_REQ Message is received */
/*        profileSlcStartConnectionProcess();*/
    }
}


/****************************************************************************
NAME
    handleAppResetPdlReq

DESCRIPTION
    Handles the application reset paired device list message.
    
*/
static void handleAppResetPdlReq(void)
{        
    the_app->clearing_pdl = FALSE;
}        


/****************************************************************************
NAME
    handleAppDeviceConnectReq

DESCRIPTION
    Handles the application connect message.
    
*/
static void handleAppDeviceConnectReq(bool disconnect_current)
{
    switch (the_app->app_state)
    {
        case AppStateIdle:
        {
            if (profileSlcAreAllDevicesDisconnected())
            {
                {
                    the_app->auto_connect_in_progress = TRUE;
                    kickCommAction(CommActionConnect);
                }
                break; 
            }
            /* fall through if connected */     
        }
        case AppStateStreaming:
        case AppStateInCall:
        {          
            {
                /* Disconnect from current device and connect to next device in list if one exists */
                the_app->auto_connect_in_progress = TRUE;
                if (disconnect_current)
                {
                    kickCommAction(CommActionDisconnect);
                }
                else
                {
                    if (the_app->app_state == AppStateStreaming)
                    {
                        /* Suspend A2DP streaming before connecting */
                        the_app->comm_action = CommActionConnect;
                        appStreamCloseReq();
                    }
                    else if (the_app->app_state == AppStateInCall)
                    {
                        /* End the current call before connecting */
                        the_app->comm_action = CommActionConnect;
                        appCallCloseReq();
                    }
                    else
                    {
                        /* Connect immediately */
                        kickCommAction(CommActionConnect);
                    }
                }
            }
            break;            
        }    
        case AppStateInquiring:
        case AppStateSearching:
        {
            {
                scanCancelInquiryScan();
                the_app->auto_connect_in_progress = TRUE;
                kickCommAction(CommActionConnect);
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
    handleAppDeviceDiscoverReq

DESCRIPTION
    Handles the application discover message.
    
*/
static void handleAppDeviceDiscoverReq(bool disconnect_current)
{
    switch (the_app->app_state)
    {
        case AppStateIdle:
        {
            if (profileSlcAreAllDevicesDisconnected())
            {
                scanMakeDiscoverable();
            }
            /* Fall through */                        
        }
        case AppStateStreaming:
        case AppStateInCall:
        case AppStateConnecting:
        {
            the_app->auto_connect_in_progress = FALSE;
            if (disconnect_current)
            {
                kickCommAction(CommActionDiscover);
            }
            else
            {
                kickCommAction(CommActionDiscoverWhileConnected);    
            }
            break;
        }
        case AppStateInquiring:
        case AppStateSearching:
        {
            /* Do nothing here, already attempting to find a new device */
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
    handleAppEnterDfuMode

DESCRIPTION
    Handles the application enter DFU mode message.
    
*/
static void handleAppEnterDfuMode(void)
{        
    if ( the_app->app_state == AppStateEnteringDfu )
    {
        BootSetMode(0);
    }
}


/****************************************************************************
NAME
    handleAppVoipCallIncoming

DESCRIPTION
    Handles the application incoming VOIP call message.
    
*/
static void handleAppVoipCallIncoming(void)
{        
    switch ( the_app->app_state )
    {
        case AppStateIdle:        
        case AppStateStreaming:
        {
            the_app->call_type = aghfp_call_type_incoming;
            kickCommAction(CommActionCall);
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
    handleAppVoipCallOutgoing

DESCRIPTION
    Handles the application outgoing VOIP call message.
    
*/
static void handleAppVoipCallOutgoing(void)
{        
    switch ( the_app->app_state )
    {
        case AppStateConnected:        
        case AppStateStreaming:
        {
            the_app->call_type = aghfp_call_type_outgoing;
            kickCommAction(CommActionCall);
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
    handleAppVoipCallCancel

DESCRIPTION
    Handles the application cancel VOIP call message.
    
*/
static void handleAppVoipCallCancel(void)
{        
    switch ( the_app->app_state )
    {
        case AppStateInCall:
        {
            /* Cancel an incoming/outgoing call setup */
            aghfpCallCancel(the_app->call_type);
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
    handleAppVoipCallActive
    
DESCRIPTION
    Handles the application VOIP call active message.
    
*/
static void handleAppVoipCallActive(void)
{            
    the_app->voip_call_active = TRUE;
                
    switch ( the_app->app_state )
    {
        case AppStateIdle:        
        case AppStateStreaming:
        {
            the_app->call_type = aghfp_call_type_transfer;
            kickCommAction(CommActionCall);
            break;
        }
        case AppStateInCall:
        {
            /* Answer an incoming/outgoing call */
            aghfpCallAnswer(the_app->call_type);
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
    handleAppVoipCallInactive
    
DESCRIPTION
    Handles the application VOIP call inactive message.
    
*/
static void handleAppVoipCallInactive(void)
{                    
    if (a2dpSlcIsMediaOpen() && the_app->audio_streaming_state!=StreamingActive)
    {    /* Force an A2dp media channel to be (re)opened after call ends */
        a2dpStreamSetAudioStreamingState(StreamingPending);
    }
        
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        {
            if (aghfpSlcGetConnectedHF() == NULL)
            {
                the_app->voip_call_active = FALSE;
                break;
            }
            /* Fall through as state reached if user ends call from headset */
        }
        case AppStateInCall:     /* This state reached if an active call is in progress or an incoming/outgoing call is being setup */
        {
            if ( the_app->voip_call_active )
            {
                the_app->voip_call_active = FALSE;
                        
                /* Presence of audio when AppStateConnected state is reached will trigger a CommActionStream */
                kickCommAction(CommActionEndCall);
            }
            else
            {   /* Being in InCall state without voip_call_active asserted imples an incoming/outgoing call is being setup */                    
                /* Cancel an incoming/outgoing call setup */
                aghfpCallCancel(the_app->call_type);
            }
            break;
        }
        default:
        {
            the_app->voip_call_active = FALSE;
            break;
        }
    }
}            


/****************************************************************************
NAME
    handleAppAudioStreamingActive
    
DESCRIPTION
    Handles the application audio streaming active message.
    
*/
static void handleAppAudioStreamingActive(void)
{            
    a2dpStreamSetAudioStreamingState(StreamingActive);
    
    if ( !profileSlcAreAnyRemoteDevicesConnecting() && !the_app->voip_call_active )
    {    
        switch ( the_app->app_state )
        {
            case AppStateIdle:        
            {
                kickCommAction(CommActionStream);
                break;
            }
            default:
            {
                break;
            }
        }
    }
}


/****************************************************************************
NAME
    handleAppAudioStreamingInactive
    
DESCRIPTION
    Handles the application audio streaming inactive message.
    
*/
static void handleAppAudioStreamingInactive(void)
{            
#if 0
    if (a2dpSlcIsMediaOpen())
    {    /* Leave an A2DP media channel open until timer expires */
        a2dpStreamSetAudioStreamingState(StreamingPending);
    }
    else
#endif		
    {
        a2dpStreamSetAudioStreamingState(StreamingInactive);
                    
        switch ( the_app->app_state )
        {
            case AppStateStreaming:
            {
                kickCommAction(CommActionEndStream);
                break;
            }
            default:
            {
                break;
            }
        }
    }
}            


/****************************************************************************
NAME
    handleAppAudioStreamingTimer
    
DESCRIPTION
    Handles the application audio streaming timer message.
    
*/
static void handleAppAudioStreamingTimer(void)
{            
    a2dpStreamSetAudioStreamingState(StreamingInactive);
                    
    switch ( the_app->app_state )
    {
        case AppStateStreaming:
        {
            kickCommAction(CommActionEndStream);
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
    handleAppA2dpMediaStreamHoldoff
    
DESCRIPTION
    Handles the application A2DP streaming holdoff message.
    
*/
static void handleAppA2dpMediaStreamHoldoff(void)
{        
    the_app->a2dp_media_stream_holdoff = FALSE;
    if ( the_app->a2dp_media_stream_requested )
    {
        the_app->a2dp_media_stream_requested = FALSE;
        
        a2dpStreamStartA2dp();
    }
}        


/****************************************************************************
NAME
    handleAppRefreshEncryptionReq
    
DESCRIPTION
    Handles the application refresh encryption key message.
    
*/
static void handleAppRefreshEncryptionReq(devInstanceTaskData *inst)
{        
    switch (the_app->app_state)
    {
        case AppStateIdle:
        {
            /* If we have a device active but not in call / streaming */
            ConnectionSmEncryptionKeyRefresh(&inst->bd_addr);
            break;
        }
        default:
        {
            break;
        }
    }           
    MessageSendLater(&inst->task, APP_REFRESH_ENCRYPTION_REQ, 0, D_MIN(EPR_TIMEOUT));
}            


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    kickCommAction

DESCRIPTION
    Used to kick off a comm action eg. connect/disconnect.
    
*/
void kickCommAction (mvdCommAction comm_action)
{
    bool command_to_process;
    
    do
    {
        DEBUG_EVENT(("kickCommAction(%s) last=%s\n",s_comm_action_names[comm_action],s_comm_action_names[the_app->comm_action]));

        DEBUG_EVENT(("AppState(%s)\n",s_app_states[the_app->app_state]));
        
        command_to_process = FALSE;
        
        switch (comm_action)
        {
            case CommActionNone:
            {
                commActionNone();
                break;
            }
            case CommActionCall:
            {
                command_to_process = commActionCall(&comm_action);
                break;
            }
            case CommActionEndCall:
            {
                command_to_process = commActionEndCall(&comm_action);
                break;
            }
            case CommActionStream:
            {
                command_to_process = commActionStream(&comm_action);
                break;
            }
            case CommActionEndStream:
            {
                command_to_process = commActionEndStream(&comm_action);            
                break;
            }
            case CommActionConnect:
            {
                command_to_process = commActionConnect(&comm_action);            
                break;
            }
            case CommActionDisconnect:
            {
                command_to_process = commActionDisconnect(&comm_action);           
                break;
            }
            case CommActionInquire:
            {
                command_to_process = commActionInquire(&comm_action);            
                break;
            }
            case CommActionDiscover:
            {
                command_to_process = commActionDiscover(&comm_action);            
                break;
            }
            case CommActionDiscoverWhileConnected:
            {
                command_to_process = commActionDiscoverWhileConnected(&comm_action);            
                break;
            }
            default:
            {
                /* Should never get here */
                break;
            }
        }
    }
    while (command_to_process);
}


/****************************************************************************
NAME
    eventHandleAppMessage
    
DESCRIPTION
    Handles the application messages.
    
*/
void eventHandleAppMessage(MessageId id, Message message)
{    
    switch(id)
    {
        case APP_INIT:
        {
            DEBUG_EVENT(("APP_INIT\n"));
            handleAppInit();        
            break;
        }
        case APP_INIT_CFM:
        {
            DEBUG_EVENT(("APP_INIT_CFM\n"));
            handleAppInitCfm();        
            break;
        }
        case APP_POWERON_REQ:
        {
            DEBUG_EVENT(("APP_POWERON_REQ\n"));
            handleAppPowerOnReq();        
            break;
        }
        case APP_POWEROFF_REQ:
        {
            DEBUG_EVENT(("APP_POWEROFF_REQ\n"));
            break;    
        }
        case APP_LIMBO_TIMEOUT:
        {
            DEBUG_EVENT(("APP_LIMBO_TIMEOUT\n"));
            break;
        }
        case APP_RESET_PDL_REQ:
        {
            DEBUG_EVENT(("APP_RESET_PDL_REQ\n"));
            handleAppResetPdlReq();        
            break;
        }
        case APP_CANCEL_LED_INDICATION:
        {
            DEBUG_EVENT(("APP_CANCEL_LED_INDICATION\n"));
            break;
        }
        case APP_KICK_COMM_ACTION_REQ:
        {
            DEBUG_EVENT(("APP_KICK_COMM_ACTION_REQ\n"));
            kickCommAction(((APP_KICK_COMM_ACTION_REQ_T*)message)->comm_action);
            break;
        }
        case APP_DEVICE_CONNECT_REQ:
        {
            DEBUG_EVENT(("APP_DEVICE_CONNECT_REQ\n"));
            handleAppDeviceConnectReq(((APP_DEVICE_CONNECT_REQ_T *)message)->disconnect_current);        
            break;
        }
        case APP_DEVICE_DISCOVER_REQ:
        {
            DEBUG_EVENT(("APP_DEVICE_DISCOVER_REQ\n"));
            handleAppDeviceDiscoverReq(((APP_DEVICE_DISCOVER_REQ_T *)message)->disconnect_current);                
            break;
        }
        case APP_ENTER_DFU_MODE:
        {
            DEBUG_EVENT(("APP_ENTER_DFU_MODE\n"));
            handleAppEnterDfuMode();        
            break;
        }
        case APP_VOIP_CALL_INCOMING:        
        {
/*            if ( !the_app->voip_call_active )*/
            {
                DEBUG_EVENT(("APP_VOIP_CALL_INCOMING\n"));
                handleAppVoipCallIncoming();            
            }
            break;
        }
        case APP_VOIP_CALL_OUTGOING:
        {
            if ( !the_app->voip_call_active )
            {
                DEBUG_EVENT(("APP_VOIP_CALL_OUTGOING\n"));
                handleAppVoipCallOutgoing();        
				MessageSendLater(&the_app->task,APP_VOIP_CALL_ACTIVE,0,500);
            }
            break;
        }
        case APP_VOIP_CALL_CANCEL:
        {
            if ( !the_app->voip_call_active )
            {
                DEBUG_EVENT(("APP_VOIP_CALL_CANCEL\n"));
                handleAppVoipCallCancel();        
            }
            break;
        }
        case APP_VOIP_CALL_ACTIVE:
        {
            if ( !the_app->voip_call_active )
            {
                DEBUG_EVENT(("APP_VOIP_CALL_ACTIVE\n"));
                handleAppVoipCallActive();        
            }
            break;
        }
        case APP_VOIP_CALL_INACTIVE:
        {
            if ( the_app->voip_call_active || (the_app->app_state == AppStateInCall) )
            {
                DEBUG_EVENT(("APP_VOIP_CALL_INACTIVE\n"));
                handleAppVoipCallInactive();                
            }
            break;
        }
        case APP_AUDIO_STREAMING_ACTIVE:
        {
/*            if ( the_app->audio_streaming_state != StreamingActive )*/
            {
                DEBUG_EVENT(("APP_AUDIO_STREAMING_ACTIVE\n"));
                handleAppAudioStreamingActive();        
            }
            break;
        }
        case APP_AUDIO_STREAMING_INACTIVE:
        {
            if (the_app->audio_streaming_state == StreamingActive)
            {
                DEBUG_EVENT(("APP_AUDIO_STREAMING_INACTIVE\n"));
                handleAppAudioStreamingInactive();            
            }
            break;
        }
        case APP_AUDIO_STREAMING_TIMER:
        {
            if (the_app->audio_streaming_state == StreamingPending )
            {
                DEBUG_EVENT(("APP_AUDIO_STREAMING_TIMER\n"));
                handleAppAudioStreamingTimer();
            }
            break;
        }
        case APP_A2DP_MEDIA_STREAM_HOLDOFF:
        {
            DEBUG_EVENT(("APP_A2DP_MEDIA_STREAM_HOLDOFF\n"));
            handleAppA2dpMediaStreamHoldoff();
            break;
        }
        case APP_INQUIRY_TIMER:
        {
            DEBUG_EVENT(("APP_INQUIRY_TIMER\n"));
            /* Delay time-out until we fail to connect to current remote device */
            the_app->app_inquiry_timer_expired = TRUE;
            break;                    
        }
        case APP_RECONNECT_TIMER:
        {
            DEBUG_EVENT(("APP_RECONNECT_TIMER\n"));
            the_app->app_reconnect_timer_expired = TRUE;
            break;
        }
        case APP_HID_SEQUENCE:
        {
            DEBUG_EVENT(("APP_HID_SEQUENCE\n"));
            break;
        }
        case APP_STREAM_CLOSE_COMPLETE:
        {
            DEBUG_EVENT(("APP_STREAM_CLOSE_COMPLETE\n"));
            streamManagerCloseComplete();       
            break;
        }
        case APP_CONNECT_A2DP_AUDIO:
        {
            DEBUG_EVENT(("APP_CONNECT_A2DP_AUDIO\n"));
            a2dpStreamConnectA2dpAudio(((APP_CONNECT_A2DP_AUDIO_T *)message)->media_sink);
            break;
        }
        case APP_PROCESS_INQUIRE_RESULT:
        {
            DEBUG_EVENT(("APP_PROCESS_INQUIRE_RESULT\n"));
            scanProcessNextInquireResult();
            break;
        }
		case APP_MSG_TIMEOUT:
			the_app->waiting_msg = FALSE;
			MessageCancelAll(&the_app->task,APP_MSG_TIMEOUT);
			break;
        default:
        {
            DEBUG_EVENT(("Unhandled APP message 0x%X\n", (uint16)id));
            break;
        }
    }
}


/****************************************************************************
NAME
    eventHandleInstanceMessage
    
DESCRIPTION
    Handles the application messages associated with a device instance.
    
*/
void eventHandleInstanceMessage(devInstanceTaskData *inst, MessageId id, Message message)
{
    switch (id)
    {
        case APP_REFRESH_ENCRYPTION_REQ:
        {
            DEBUG_EVENT(("APP_REFRESH_ENCRYPTION_REQ inst:[0x%x]\n", (uint16)inst));
            handleAppRefreshEncryptionReq(inst);           
            break;            
        }
        case APP_PROFILE_CONNECT_TIMER:
        {
            DEBUG_EVENT(("APP_PROFILE_CONNECT_TIMER inst:[0x%x]\n", (uint16)inst));
            /* The device has become unresponsive. Finish the connection attempt. */
            profileSlcCancelConnectionAttempt(inst);
            break;
        }
        case APP_INTERNAL_DESTROY_REQ:
        {
            DEBUG_EVENT(("APP_INTERNAL_DESTROY_REQ inst:[0x%x]\n", (uint16)inst));
            devInstanceDestroy(inst);
            return;
        }
        case APP_INTERNAL_CONNECT_MEDIA_CHANNEL_REQ:
        {
            DEBUG_EVENT(("APP_INTERNAL_CONNECT_MEDIA_CHANNEL_REQ inst:[0x%x]\n", (uint16)inst));
            a2dpSlcConnect(inst);
            return;
        }   
        case APP_MEDIA_CHANNEL_REOPEN_REQ:
        {
            DEBUG_EVENT(("APP_MEDIA_CHANNEL_REOPEN_REQ inst:[0x%x]\n", (uint16)inst));
            a2dpSlcReOpen(inst);
            return;
        }
        case APP_CONNECT_TIMER:
        {
            DEBUG_EVENT(("APP_CONNECT_TIMER\n"));
            inst->connect_timer_expired = TRUE;
/*            the_app->connect_bdaddr = inst->bd_addr;
            kickCommAction(CommActionConnect);*/
            return;    
        }
        case APP_CONNECT_CFM:
        {
            DEBUG_EVENT(("APP_CONNECT_CFM\n"));
            profileSlcConnectComplete(inst, ((APP_CONNECT_CFM_T *)message)->success);
            break;
        }
        case APP_LATE_CONNECT_TIMER:
        {
            DEBUG_EVENT(("APP_LATE_CONNECT_TIMER\n"));
            profileSlcConnectReq();
        }
        default:
        {
            break;
        }
    }        
}


/****************************************************************************
NAME
    eventHandleCancelCommAction

DESCRIPTION
    Cancels all comm actions.
    
*/
void eventHandleCancelCommAction (void)
{
    /* Cancel any current and queued action */
    DEBUG_EVENT(("eventHandleCancelCommAction\n"));
    the_app->comm_action = CommActionNone;
    the_app->pending_comm_action = CommActionNone;
}


/****************************************************************************
NAME
    eventHandleSendKickCommActionMessage

DESCRIPTION
    Creates a APP_KICK_COMM_ACTION_REQ message with the supplied mvdCommAction for immediate delivery.
    
*/
void eventHandleSendKickCommActionMessage (mvdCommAction comm_action)
{
    MAKE_APP_MESSAGE(APP_KICK_COMM_ACTION_REQ);
    message->comm_action = comm_action;
    MessageSend(&the_app->task, APP_KICK_COMM_ACTION_REQ, message);
}
