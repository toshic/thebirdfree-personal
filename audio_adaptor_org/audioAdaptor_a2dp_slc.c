/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Deals with connecting and disconnecting A2DP, and provides helper functions
    for determining active connections.
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_init.h"
#include "audioAdaptor_statemanager.h"
#include "audioAdaptor_a2dp_slc.h"
#include "audioAdaptor_dev_instance.h"

#include <panic.h>
#include <stdlib.h>


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME 
    a2dpSlcConnect

DESCRIPTION
    Connects the A2DP profile from the local device.
 
*/
bool a2dpSlcConnect(devInstanceTaskData *inst)
{
    uint8 seids[NUM_SEPS];
    uint16 size_seids = 1;
    
    switch (getA2dpState(inst))
    {
        case A2dpStateDisconnected:
        case A2dpStateConnected:
        {
            /* Open media channel. Call the correct API depending on whether signalling connected or not. */
            size_seids = initSeidConnectPriority(seids);
                    
            setA2dpState(inst, A2dpStateOpening);
            
            DEBUG_A2DP(("A2dpConnect 0x%X 0x%X 0x%lX\n", inst->bd_addr.nap, inst->bd_addr.uap, inst->bd_addr.lap));

            if (inst->a2dp && inst->a2dp_sig_sink)
            {
                DEBUG_A2DP(("A2dpOpenMediaChannelOnly, size_seids:0x%X seids[0]:0x%X\n", size_seids, seids[0]));
                A2dpOpen(inst->a2dp, size_seids, seids);
            }
            else
            {
                DEBUG_A2DP(("A2dpOpenSignallingAndMediaChannels, size_seids:0x%X seids[0]:0x%X\n", size_seids, seids[0]));
                A2dpConnectOpen(&inst->task, &inst->bd_addr, size_seids, seids, the_app->a2dp_data.sep_entries);
            }
    
            inst->a2dp_reopen = FALSE;

            return TRUE;
        }
        default:
        {
            break;
        }
    }
   
    return FALSE;
}


/****************************************************************************
NAME 
    a2dpSlcReOpen

DESCRIPTION
    Reopens A2DP with a default configuration.
 
*/
bool a2dpSlcReOpen(devInstanceTaskData *inst)
{
    uint8 seids[NUM_SEPS];
    uint16 size_seids;
     
    /* Always try a reopen with SBC, as this must be supported */
#ifdef DUAL_STREAM    
    size_seids = 2;
    seids[0] = SBC_SEID;
    seids[1] = SBC_DS_SEID;
#else
    size_seids = 1;
    seids[0] = SBC_SEID;
#endif /* DUAL_STREAM */ 
     
#ifdef DEMO_MODE
    /* In demo mode the codec to use on a media channel reopen can be chosen by a button press */
    if (inst->a2dp_reopen_codec)
    {
        size_seids = 1;
        seids[0] = inst->a2dp_reopen_codec;
    }
#endif /* DEMO_MODE */    
    
    switch (getA2dpState(inst))
    {
        case A2dpStateDisconnected:
        case A2dpStateConnected:
        {
            /* Open media channel. Call the correct API depending on whether signalling connected or not. */            
                    
            setA2dpState(inst, A2dpStateOpening);
            
            DEBUG_A2DP(("A2dpConnect 0x%X 0x%X 0x%lX\n", inst->bd_addr.nap, inst->bd_addr.uap, inst->bd_addr.lap));

            if (inst->a2dp && inst->a2dp_sig_sink)
            {
                DEBUG_A2DP(("A2dpOpenMediaChannelOnly, size_seids:0x%X seids[0]:0x%X\n", size_seids, seids[0]));
                A2dpOpen(inst->a2dp, size_seids, seids);
            }
            else
            {
                DEBUG_A2DP(("A2dpOpenSignallingAndMediaChannels, size_seids:0x%X seids[0]:0x%X\n", size_seids, seids[0]));
                A2dpConnectOpen(&inst->task, &inst->bd_addr, size_seids, seids, the_app->a2dp_data.sep_entries);
            }
            
            inst->a2dp_reopen = TRUE;

            return TRUE;
        }
        default:
        {
            break;
        }
    }
   
    return FALSE;
}


/****************************************************************************
NAME 
    a2dpSlcDisconnectAll

DESCRIPTION
    Disconnects all A2DP connections.
 
*/
void a2dpSlcDisconnectAll(void)
{
    uint16 i = 0;
    
    DEBUG_A2DP(("A2dp Disconnect All\n"));
        
    /* Disconnect all A2DP connections */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        if (the_app->dev_inst[i] != NULL)
        {
            MessageCancelAll(&the_app->dev_inst[i]->task, APP_MEDIA_CHANNEL_REOPEN_REQ);
            switch (getA2dpState(the_app->dev_inst[i]))
            {
                case A2dpStateConnected:
                case A2dpStateOpen:
                case A2dpStateStarting:
                case A2dpStateStreaming:
                case A2dpStateSuspending:
                {
                    setA2dpState(the_app->dev_inst[i], A2dpStateDisconnecting);
                    A2dpDisconnectAll(the_app->dev_inst[i]->a2dp);
                    DEBUG_A2DP(("A2dpDisconnect\n"));
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
    a2dpSlcIsMediaOpen

DESCRIPTION
    Returns the number of open A2DP media channels.
 
*/
uint16 a2dpSlcIsMediaOpen(void)
{
    uint16 i;
    uint16 number_open_media = 0;

    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        if (the_app->dev_inst[i] != NULL)
        {           
            switch (the_app->dev_inst[i]->a2dp_state)
            {
                case A2dpStateOpen:
                case A2dpStateStarting:
                case A2dpStateStreaming:
                case A2dpStateSuspending:
                case A2dpStateClosing:
                {
                    number_open_media++;
                }
                default:
                {
                    break;
                }
            }
        }
    }
    return number_open_media;
}


/****************************************************************************
NAME 
    a2dpSlcIsDifferentMediaOpen

DESCRIPTION
    Returns if there are any open A2DP media channels,
    different from the connection to the device passed in.
 
*/
bool a2dpSlcIsDifferentMediaOpen(devInstanceTaskData *inst)
{
    uint16 i;

    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        if (the_app->dev_inst[i] != NULL)
        {
            if ((inst == NULL) || ((inst != NULL) && (the_app->dev_inst[i] != inst)))
            {
                switch (the_app->dev_inst[i]->a2dp_state)
                {
                    case A2dpStateOpen:
                    case A2dpStateStarting:
                    case A2dpStateStreaming:
                    case A2dpStateSuspending:
                    case A2dpStateClosing:
                    {
                        return TRUE;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }
    return FALSE;
}


/****************************************************************************
NAME 
    a2dpSlcIsMediaStreaming

DESCRIPTION
    Returns if there are any streaming A2DP media channels.
    
*/
bool a2dpSlcIsMediaStreaming(void)
{
    uint16 i;

    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        if (the_app->dev_inst[i] != NULL)
        {
            switch (the_app->dev_inst[i]->a2dp_state)
            {           
                case A2dpStateStreaming:
                case A2dpStateSuspending:
                {
                    return TRUE;
                }
                default:
                {
                    break;
                }
            }
        }
    }
    return FALSE;
}


/****************************************************************************
NAME 
    a2dpSlcIsDifferentMediaStreaming

DESCRIPTION
    Returns if there are any streaming A2DP media channels,
    different from the connection to the device passed in.
 
*/
bool a2dpSlcIsDifferentMediaStreaming(devInstanceTaskData *inst)
{
    uint16 i;

    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        if (the_app->dev_inst[i] != NULL)
        {
            if ((inst == NULL) || ((inst != NULL) && (the_app->dev_inst[i] != inst)))
            {
                switch (the_app->dev_inst[i]->a2dp_state)
                {
                    case A2dpStateStreaming:
                    case A2dpStateSuspending:
                    {
                        return TRUE;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }
    return FALSE;
}


/****************************************************************************
NAME 
    a2dpSlcIsMediaStarting

DESCRIPTION
    Returns if there are any A2DP channels about to start streaming.
 
*/
bool a2dpSlcIsMediaStarting(void)
{
    uint16 i;

    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        if (the_app->dev_inst[i] != NULL)
        {
            switch (the_app->dev_inst[i]->a2dp_state)
            {           
                case A2dpStateStarting:
                {
                    return TRUE;
                }
                default:
                {
                    break;
                }
            }
        }
    }
    return FALSE;
}


