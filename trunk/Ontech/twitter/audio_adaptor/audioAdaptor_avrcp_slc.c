/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles messages received from the AVRCP library
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_avrcp_slc.h"
#include "audioAdaptor_avrcp_msg_handler.h"
#include "audioAdaptor_statemanager.h"

#include <avrcp.h>
#include <string.h>
#include <panic.h>
#include <stdlib.h>


/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME 
    avrcpSlcConnect

DESCRIPTION
    Connects the AVRCP profile from the local device;
 
*/
bool avrcpSlcConnect(devInstanceTaskData *inst)
{
    switch (getAvrcpState(inst))
    {
        case AvrcpStateDisconnected:
        {
            avrcp_init_params avrcp_config;
    
            avrcp_config.device_type = avrcp_target;
            
            DEBUG_AVRCP(("AvrcpConnect 0x%X 0x%X 0x%lX\n", inst->bd_addr.nap, inst->bd_addr.uap, inst->bd_addr.lap));
            
            the_app->s_connect_attempts = 0;
            setAvrcpState(inst, AvrcpStatePaging);  
            AvrcpConnectLazy(&inst->task, &inst->bd_addr, &avrcp_config);                                  
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
    avrcpSlcDisconnectAll

DESCRIPTION
    Disconnects all AVRCP connections.
 
*/
void avrcpSlcDisconnectAll(void)
{
    devInstanceTaskData *inst;
    uint16 i = 0;
    
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {          
            switch (getAvrcpState(inst))
            {
                case AvrcpStateConnected:
                {
                    DEBUG_AVRCP(("AvrcpDisconnect\n"));
                    setAvrcpState(inst, AvrcpStateDisconnecting);  
                    AvrcpDisconnect(inst->avrcp);
                }
                default:
                {
                    break;
                }
            }
        }
    }
}
