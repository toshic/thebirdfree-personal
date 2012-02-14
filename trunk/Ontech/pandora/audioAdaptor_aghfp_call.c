/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Call functionality is handled in this file.
*/


#include "audioAdaptor_private.h"
#include "audioAdaptor_aghfp_call.h"
#include "audioAdaptor_aghfp_slc.h"
#include "audioAdaptor_statemanager.h"
#include "audioAdaptor_events.h"
#include "audioAdaptor_event_handler.h"

#include <string.h>
#include <panic.h>
#include <stdlib.h>


/****************************************************************************
  MAIN FUNCTIONS
*/


/****************************************************************************
NAME 
    aghfpCallCreate

DESCRIPTION
    Creates a new call, and an audio connection for the call if one is not present.
 
*/
void aghfpCallCreate (devInstanceTaskData *inst, aghfp_call_type call_type)
{    
    switch (getAghfpState(inst))
    {
        case AghfpStateConnected:
        case AghfpStateAudioStreaming:        
        {
            setAghfpState(inst, AghfpStateCallSetup);
			if(call_type != aghfp_call_type_incoming && inst->audio_sink==0)
            {    /* Need to set up new audio connection with call */
                AghfpCallCreateAudio(inst->aghfp, call_type, TRUE, AUDIO_PACKET_TYPES, NULL);
            }
            else
            {   /* Use existing audio connection for call */
                AghfpCallCreate(inst->aghfp, call_type, TRUE);
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
    aghfpCallAnswer

DESCRIPTION
    Handles answering of incoming or outgoing call.
 
*/
void aghfpCallAnswer (aghfp_call_type call_type)
{
    devInstanceTaskData *inst = aghfpSlcGetConnectedHF();
    
    if ( (inst != NULL))
    {
        switch (getAghfpState(inst))
        {
            case AghfpStateCallSetup:
            {
                switch(call_type)
                {
                    case aghfp_call_type_incoming:
                    {
                        AghfpCallAnswer(inst->aghfp);
                        break;                
                    }    
                    case aghfp_call_type_outgoing:
                    {
                        AghfpCallRemoteAnswered(inst->aghfp);
                        break;                
                    }
                    case aghfp_call_type_transfer:
                    {
                        /* Ignore - already in a call */
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
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
    aghfpCallCancel

DESCRIPTION
    Handles the cancelling of incoming or outgoing call.
 
*/
void aghfpCallCancel (aghfp_call_type call_type)
{
    devInstanceTaskData *inst = aghfpSlcGetConnectedHF();
    
    if ( (inst != NULL))
    {
        switch (getAghfpState(inst))
        {
            case AghfpStateCallSetup:
            {
                switch(call_type)
                {
                    case aghfp_call_type_incoming:
                    case aghfp_call_type_outgoing:
                    {
                        AghfpCallTerminate(inst->aghfp, FALSE);
                        break;                
                    }
                    case aghfp_call_type_transfer:
                    {
                        /* Ignore - already in a call */
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
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
    aghfpCallEnd

DESCRIPTION
    Ends an active call.
 
*/
void aghfpCallEnd (void)
{
    devInstanceTaskData *inst = aghfpSlcGetConnectedHF();
    
    if ( (inst != NULL))
    {
        switch (getAghfpState(inst))
        {
            case AghfpStateCallActive:            
            {
                setAghfpState(inst, AghfpStateCallShutdown);
                AghfpCallTerminate(inst->aghfp, FALSE);
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
      aghfpCallOpenComplete

DESCRIPTION
     A call opening attempt has been completed.
 
*/
void aghfpCallOpenComplete (bool status)
{
    DEBUG_CALL(("aghfpCallOpenComplete status=%u app_state=%x\n", !status, the_app->app_state));
    switch ( the_app->app_state )
    {
        case AppStateIdle:
        case AppStateInCall:
        {
            if ( status )
            {    /* Call setup was successful */
                setAppState(AppStateInCall);
                kickCommAction(the_app->comm_action);
            }
            else
            {    /* Call setup NOT successful */
                setAppState(AppStateIdle);
                eventHandleCancelCommAction();  /* Get around last_comm_action filtering in kickCommAction() */
                kickCommAction(CommActionEndCall);    /* Will force entry to Streaming mode is usb audio is present */
            }
            break;
        }    
        default:
        {
            DEBUG_CALL((" - IGNORED\n"));
            break;
        }
    }
}


/****************************************************************************
NAME 
    aghfpCallCloseComplete

DESCRIPTION
    A call closure has been completed.
 
*/
void aghfpCallCloseComplete (void)
{
    DEBUG_CALL(("aghfpCallCloseComplete app_state=%x\n", the_app->app_state));
    switch ( the_app->app_state )
    {
        case AppStateInCall:
        {
            setAppState(AppStateIdle);
            if ( the_app->comm_action != CommActionCall )
                kickCommAction(the_app->comm_action);
            break;
        }
        default:
        {
            DEBUG_CALL((" - IGNORED\n"));
            break;
        }
    }
}
