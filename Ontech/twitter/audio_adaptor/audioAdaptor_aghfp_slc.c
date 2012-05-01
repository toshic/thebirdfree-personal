/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Deals with connecting and disconnecting AGHFP connections, and provides helper functions
    for determining active connections.
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_aghfp_slc.h"
#include "audioAdaptor_statemanager.h"

#include <string.h>
#include <panic.h>
#include <stdlib.h>
 
/****************************************************************************
NAME 
    aghfpSlcInitHf

DESCRIPTION
    Initialises the AGHFP library with HFP v1.5 profile.
 
*/
void aghfpSlcInitHf (void)
{
    uint16 supported_feature = aghfp_incoming_call_reject;
    if ( (the_app->aghfp == NULL))
    {
        if(the_app->support_inbandring)
            supported_feature |= aghfp_inband_ring;
            AghfpInit(&the_app->task, aghfp_handsfree_15_profile, supported_feature); 
    }
}


/****************************************************************************
NAME 
    aghfpSlcConnect

DESCRIPTION
    Makes an AGHFP connection to the remote device, if no connections are currently pending.
 
*/
bool aghfpSlcConnect (devInstanceTaskData *inst)
{
    uint16 i;
    bool aghfp_disconnected = TRUE;
    
    /* Make sure HFP is disconnected */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        if (the_app->dev_inst[i] != NULL)
        {
            if (getAghfpState(the_app->dev_inst[i]) != AghfpStateDisconnected)
                aghfp_disconnected = FALSE;
        }
    }
    
    if ( (inst != NULL))
    {
        if (aghfp_disconnected)
        {        
            DEBUG_AGHFP(("AghfpSlcConnect 0x%X 0x%X 0x%lX\n", inst->bd_addr.nap, inst->bd_addr.uap, inst->bd_addr.lap));
        
             /* Connect using HFP */
            inst->aghfp = the_app->aghfp;
     
            setAghfpState(inst, AghfpStatePaging);
            the_app->s_connect_attempts = 0;
            AghfpSlcConnect(inst->aghfp, &inst->bd_addr);
            
            return TRUE;
        }
        else
        {
            /* the profile is already connected, mark it as responding for this instance also */
            inst->responding_profiles |= (ProfileAghfp | ProfileAghsp);
        }
    }
    return FALSE;
}


/****************************************************************************
NAME 
    aghfpSlcDisconnectAll

DESCRIPTION
    Disconnect all AGHFP connections.
 
*/
void aghfpSlcDisconnectAll (void)
{    
    uint16 i;
    
    /* Disconnect all AGHFP connections */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        if (the_app->dev_inst[i] != NULL)
        {
            switch (getAghfpState(the_app->dev_inst[i]))
            {
                case AghfpStateConnected:
                case AghfpStateAudioOpening:
                case AghfpStateAudioStreaming:
                case AghfpStateAudioClosing:
                case AghfpStateCallSetup:
                case AghfpStateCallActive:
                case AghfpStateCallShutdown:
                {
                    setAghfpState(the_app->dev_inst[i], AghfpStateDisconnecting);
                    AghfpSlcDisconnect(the_app->dev_inst[i]->aghfp);
                    DEBUG_AGHFP(("AghfpDisconnect\n"));
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
}


/****************************************************************************
NAME 
    aghfpSlcAudioOpen

DESCRIPTION
    Open an AGHFP audio connection.
 
*/
void aghfpSlcAudioOpen (devInstanceTaskData *inst)
{
    if ( (inst->aghfp != NULL))
    {
        if (getAghfpState(inst) == AghfpStateConnected)
        {
            setAghfpState(inst, AghfpStateAudioOpening);
            AghfpAudioConnect(inst->aghfp, AUDIO_PACKET_TYPES, NULL);
        }
    }
}


/****************************************************************************
NAME 
    aghfpSlcAudioClose

DESCRIPTION
    Closes an AGHFP audio connection.
 
*/
void aghfpSlcAudioClose (void)
{
    devInstanceTaskData *inst = aghfpSlcGetConnectedHF();
    
    if ( (inst != NULL) && (inst->aghfp != NULL))
    {
        if (getAghfpState(inst) == AghfpStateAudioStreaming)
        {
            setAghfpState(inst, AghfpStateAudioClosing);
            AghfpAudioDisconnect(inst->aghfp);
        }
    }
}


/****************************************************************************
NAME 
    aghfpSlcGetConnectedHF

DESCRIPTION
    Returns any device instance that has a completed AGHFP connection.
 
*/
devInstanceTaskData *aghfpSlcGetConnectedHF(void)
{
    uint16 i;
  
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        if (the_app->dev_inst[i] != NULL)
        {
            switch (the_app->dev_inst[i]->aghfp_state)
            {
                case AghfpStateConnected:
                case AghfpStateAudioOpening:
                case AghfpStateAudioStreaming:
                case AghfpStateAudioClosing:
                case AghfpStateCallSetup:
                case AghfpStateCallActive:
                case AghfpStateCallShutdown:
                {
                    return the_app->dev_inst[i];
                }
                default:
                {
                    break;
                }
            }
        }
    }
    return NULL;
}
