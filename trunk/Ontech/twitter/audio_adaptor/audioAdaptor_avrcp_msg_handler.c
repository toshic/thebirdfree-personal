/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2009
Part of Audio-Adaptor-SDK 2009.R1

DESCRIPTION
    Handles messages received from the AVRCP library.
*/

#include "audioAdaptor_private.h"
#include "audioAdaptor_init.h"
#include "audioAdaptor_profile_slc.h"
#include "audioAdaptor_avrcp_msg_handler.h"
#include "audioAdaptor_statemanager.h"
#include "audioAdaptor_dev_instance.h"

#include <avrcp.h>
#include <string.h>
#include <panic.h>
#include <stdlib.h>


/****************************************************************************
  LOCAL FUNCTIONS
*/

/****************************************************************************
NAME
    avrcpUnhandledMessage

DESCRIPTION
    For debug purposes, so any unhandled AVRCP messages are discovered.     

*/
static void avrcpUnhandledMessage(devInstanceTaskData *theInst, uint16 id)
{
    DEBUG_AVRCP(("UNHANDLED AVRCP MESSAGE: inst:[0x%x] id:[0x%x] avrcp_state:[%d]\n",(uint16)theInst, id, theInst->avrcp_state));
}


/****************************************************************************
NAME
    acceptAvrcpConnectInd

DESCRIPTION
    Decide if the incoming AVRCP connection can be connected.
    Returns the device instance if successful, otherwise returns NULL.

*/
static devInstanceTaskData *acceptAvrcpConnectInd(bdaddr *bd_addr)
{
    devInstanceTaskData *inst;
    uint16 i;
    uint16 active_avrcp_connections = 0;
    
    /* 1 AVRCP connection allowed for non-DualStream, or 2 for DualStream */
    for (i = 0; i < MAX_NUM_DEV_CONNECTIONS; i++)
    {
        inst = the_app->dev_inst[i];
        if (inst != NULL)
        {            
            switch (getAvrcpState(inst))
            {
                case AvrcpStateDisconnected:               
                case AvrcpStateDisconnecting:
                {
                    /* These states are okay, there's no active connection */
                    break;
                }
                case AvrcpStatePaging:
                {
                    /* An incoming connection is not currently allowed if there is an outgoing connection */
                    return NULL;   
                }        
                default:
                {
                    /* Mark that there is already an ongoing connection here */                    
                    active_avrcp_connections++;
                }
            }
        }
    }

    if (active_avrcp_connections < MAX_NUM_DEV_CONNECTIONS)
    {                            
        /* Check there isn't already an instance for this device */
        inst = devInstanceFindFromBddr(bd_addr, TRUE);    
    
        if (inst != NULL)
        {            
            return inst;
        }
    }
    
    return NULL;
}


/****************************************************************************
NAME
    handleAvrcpSubUnitInfoInd

DESCRIPTION
    Handles the AVRCP library SUBUNIT INFO indication message.

*/
static void handleAvrcpSubUnitInfoInd (devInstanceTaskData *inst, AVRCP_SUBUNITINFO_IND_T *ind)
{
    bool accept = FALSE;
    uint8 page_data[PAGE_DATA_LENGTH];
    memset(page_data, 0, PAGE_DATA_LENGTH);

    /* We only have content for page zero. */
    if (!ind->page)
    {
        /* Fill in the single subunit with address 0 and pad with 0xff's */
        page_data[0] = subunit_panel<<3;
        page_data[1] = 0xff;
        page_data[2] = 0xff;
        page_data[3] = 0xff;
    }
    
    /* Send response back to the lib */
    AvrcpSubUnitInfoResponse(inst->avrcp, accept, page_data);
}


/****************************************************************************
NAME
    handleAvrcpOperation

DESCRIPTION
    Processes the incoming PASSTHROUGH AVRCP command.

*/
static void handleAvrcpOperation (avc_operation_id opid, bool state)
{
    /* Convert the AVRCP event to HID */
    if (!state)
    {    
        /* Action command on button up event */
        if (opid == opid_pause)
        {    /* Some headsets use Pause to toggle between playing and paused states */
			SendEvent(EVT_AVRCP_CMD_PAUSE,state);
            if ( the_app->audio_streaming_state==StreamingActive )
            {
				MessageSend(&the_app->task,APP_AUDIO_STREAMING_INACTIVE,0);
            }
            else
            {
				MessageSend(&the_app->task,APP_AUDIO_STREAMING_ACTIVE,0);
            }
        }
        else if (opid == opid_play)
        {
			SendEvent(EVT_AVRCP_CMD_PLAY,state);
            if ( the_app->audio_streaming_state==StreamingActive )
            {
				MessageSend(&the_app->task,APP_AUDIO_STREAMING_INACTIVE,0);
            }
            else
            {
				MessageSend(&the_app->task,APP_AUDIO_STREAMING_ACTIVE,0);
            }
        }
        else if (opid == opid_forward)
        {
			SendEvent(EVT_AVRCP_CMD_SKIP_FWD,state);
        }
        else if (opid == opid_fast_forward)
        {
			SendEvent(EVT_AVRCP_CMD_FWD,state);
        }
        else if (opid == opid_backward)
        {
			SendEvent(EVT_AVRCP_CMD_SKIP_RWD,state);
        }
        else if (opid == opid_rewind)
        {
			SendEvent(EVT_AVRCP_CMD_RWD,state);
        }		
        else if (opid == opid_stop)
        {
			SendEvent(EVT_AVRCP_CMD_STOP,state);
			MessageSend(&the_app->task,APP_AUDIO_STREAMING_TIMER,0);
        }
    }
}


/****************************************************************************
NAME
    handleAvrcpInitCfm

DESCRIPTION
    Handles the AVRCP library initialisation result.

*/
static void handleAvrcpInitCfm(const AVRCP_INIT_CFM_T *msg)
{
    if ( msg->status == avrcp_success )
    {
        initProfileCfm(ProfileAvrcp, TRUE);
    }
    else
    {
        initProfileCfm(ProfileAvrcp, FALSE);
    }
}


/****************************************************************************
NAME
    handleAvrcpConnectInd

DESCRIPTION
    Handles the AVRCP library incoming connection message.

*/
static void handleAvrcpConnectInd(AVRCP_CONNECT_IND_T *ind)
{
    devInstanceTaskData *dev_inst;

    /* Check that this incoming connection can be accepted */
    dev_inst = acceptAvrcpConnectInd(&ind->bd_addr);      
    
    if (dev_inst != NULL)
    {
        static const avrcp_init_params init_params = {avrcp_target};
        
        /* Update state to indicate remote device is connecting */
        setAvrcpState(dev_inst, AvrcpStatePaged); 
        /* update the remote profiles supported */
        dev_inst->remote_profiles |= ProfileAvrcp;
        
        profileSlcAcceptConnectInd(dev_inst);
        
        DEBUG_AVRCP(("   Accepted AVRCP connection\n"));
        AvrcpConnectResponseLazy(ind->avrcp, ind->connection_id, TRUE, &init_params); 
 
        /* Finished so return */
        return;
    }
    
    /* Reject incoming connection, either there is an existing instance, or we failed to create a new instance */
    DEBUG_AVRCP(("   Rejected AVRCP connection\n"));
    AvrcpConnectResponseLazy(ind->avrcp, ind->connection_id, FALSE, NULL);    
}


/****************************************************************************
NAME
    handleAvrcpConnectCfm

DESCRIPTION
    Handles the AVRCP library connection confirmation message.

*/
static void handleAvrcpConnectCfm(devInstanceTaskData *inst, const AVRCP_CONNECT_CFM_T *msg)
{ 
    mvdAvrcpState current_state = getAvrcpState(inst);
    
    switch(current_state)
    {
        case AvrcpStatePaging:
        case AvrcpStatePaged:
        {                     
            if (msg->status == avrcp_success)
            {
                inst->avrcp = msg->avrcp;
                the_app->s_connect_attempts  = 0;                               
                setAvrcpState(inst, AvrcpStateConnected);
                profileSlcConnectCfm(inst, ProfileAvrcp, current_state == AvrcpStatePaged ? TRUE : FALSE);                              
            }
            else
            {        
                avrcp_init_params avrcp_config;

                avrcp_config.device_type = avrcp_target;

/* don't retry to connect */
#if 0
                the_app->s_connect_attempts += 1;

                if ( (current_state == AvrcpStatePaging) )
                {    
                    /* Try connection attempt again */
                    setAvrcpState(inst, AvrcpStatePaging);  
                    AvrcpConnectLazy(&inst->task, &inst->bd_addr, &avrcp_config);   
                }
                else 
				if( msg->status == avrcp_key_missing )
                {
                    /* Delete link key from our side and try again */
                    ConnectionSmDeleteAuthDevice(&inst->bd_addr);
                    setAvrcpState(inst, AvrcpStatePaging);  
                    AvrcpConnectLazy(&inst->task, &inst->bd_addr, &avrcp_config);   
                }
                else
#endif					
                {                
                    setAvrcpState(inst, AvrcpStateDisconnected);
                    profileSlcConnectCfm(inst, ProfileAvrcp, FALSE);
                    
                    /* send a message to try and delete this device instance */             
                    MessageSend(&inst->task, APP_INTERNAL_DESTROY_REQ, 0);
                }
            }
            return;
        }
        default:
        {
            avrcpUnhandledMessage(inst, AVRCP_CONNECT_CFM);
            return;
        }
    }
}


/****************************************************************************
NAME
    handleAvrcpDisconnectInd

DESCRIPTION
    Handles the AVRCP library disconnection message.

*/
static void handleAvrcpDisconnectInd(devInstanceTaskData *inst, const AVRCP_DISCONNECT_IND_T *msg )
{
    switch(getAvrcpState(inst))
    {
        case AvrcpStatePaging:
        {        
            /* Connection process aborted - indicate this by sending a connect fail confirmation */
            inst->avrcp = NULL;
            setAvrcpState(inst, AvrcpStateDisconnected);
            profileSlcConnectCfm(inst, ProfileAvrcp, FALSE);
            break;
        }
        case AvrcpStateConnected:
        case AvrcpStateDisconnecting:
        {        
            inst->avrcp = NULL;
            setAvrcpState(inst, AvrcpStateDisconnected);
            profileSlcDisconnectInd(inst, ProfileAvrcp);
            break;
        }
        default:
        {
            avrcpUnhandledMessage(inst, AVRCP_DISCONNECT_IND);
            return;
        }
    }
 
    inst->responding_profiles &= ~ProfileAvrcp;
    /* send a message to try and delete this device instance */
    MessageSend(&inst->task, APP_INTERNAL_DESTROY_REQ, 0);
}


/****************************************************************************
NAME
    handleAvrcpPassthroughInd

DESCRIPTION
    Handles the AVRCP library PASSTHROUGH message.

*/
static void handleAvrcpPassthroughInd(devInstanceTaskData *inst, const AVRCP_PASSTHROUGH_IND_T *msg)
{
    switch(getAvrcpState(inst))
    {
        case AvrcpStateConnected:
        {
            /* Acknowledge the request and pass via USB */
            AvrcpPassthroughResponse(msg->avrcp, avctp_response_accepted);
            handleAvrcpOperation(msg->opid, msg->state);
            return;
        }
        default:        
        {
            /* Reject the request */
            AvrcpPassthroughResponse(msg->avrcp, avctp_response_rejected);
            return;
        }
    }
}



/****************************************************************************
  MAIN FUNCTIONS
*/

/****************************************************************************
NAME
    avcrpMsgHandleLibMessage

DESCRIPTION
    Handles initial AVRCP library messages before an instance of the remote device has been created at the application level.
    Once a device instance has been created the AVRCP library messages are handled by the avrcpMsgHandleInstanceMessage handler.
    
*/
void avcrpMsgHandleLibMessage(MessageId id, Message message)
{
    switch(id)
    {
        case AVRCP_INIT_CFM:
        {
            DEBUG_AVRCP(("AVRCP_INIT_CFM status = %u [0x%X]\n", ((AVRCP_INIT_CFM_T *)message)->status,(uint16)((AVRCP_INIT_CFM_T *)message)->avrcp));
            handleAvrcpInitCfm((AVRCP_INIT_CFM_T *)message);
            break;
        }
        case AVRCP_CONNECT_IND:
        {
			SendEvent(EVT_AVRCP_SIGNAL_CONNECT_IND,0);
            DEBUG_AVRCP(("AVRCP_CONNECT_IND from: 0x%X 0x%X 0x%lX\n", ((AVRCP_CONNECT_IND_T *)message)->bd_addr.nap, ((AVRCP_CONNECT_IND_T *)message)->bd_addr.uap, ((AVRCP_CONNECT_IND_T *)message)->bd_addr.lap));
            handleAvrcpConnectInd((AVRCP_CONNECT_IND_T *)message);
            break;
        }
         default:
        {
            DEBUG_AVRCP(("Unhandled AVRCP message 0x%X\n", (uint16)id));
            break;
        }
    }
}


/****************************************************************************
NAME
    avrcpMsgHandleInstanceMessage

DESCRIPTION
    Handles AVRCP library messages associated with a device instance.
    
*/
void avrcpMsgHandleInstanceMessage(devInstanceTaskData *theInst, MessageId id, Message message)
{  
    /* Handle A2DP library messages */
    switch (id)
    {
        case AVRCP_CONNECT_IND:
        {
			SendEvent(EVT_AVRCP_SIGNAL_CONNECT_IND,0);
            DEBUG_AVRCP(("AVRCP_CONNECT_IND from: 0x%X 0x%X 0x%lX inst:[0x%x]\n", ((AVRCP_CONNECT_IND_T *)message)->bd_addr.nap, ((AVRCP_CONNECT_IND_T *)message)->bd_addr.uap, ((AVRCP_CONNECT_IND_T *)message)->bd_addr.lap, (uint16)theInst));
            handleAvrcpConnectInd((AVRCP_CONNECT_IND_T *)message);
            break;
        }
        case AVRCP_CONNECT_CFM:
        {
			SendEvent(EVT_AVRCP_SIGNAL_CONNECT_CFM,((AVRCP_CONNECT_CFM_T *)message)->status);
			the_app->conn_status.avrcp_con = ((AVRCP_CONNECT_CFM_T *)message)->status ? 0:1;
			
            DEBUG_AVRCP(("AVRCP_CONNECT_CFM status = %u inst:[0x%x]\n", ((AVRCP_CONNECT_CFM_T *)message)->status, (uint16)theInst));
            handleAvrcpConnectCfm(theInst, (AVRCP_CONNECT_CFM_T *)message);
            return;
        }
        case AVRCP_DISCONNECT_IND:
        {
			SendEvent(EVT_AVRCP_SIGNAL_DISCONNECT_IND,((AVRCP_DISCONNECT_IND_T *)message)->status);
			the_app->conn_status.avrcp_con = 0;
			
            DEBUG_AVRCP(("AVRCP_DISCONNECT_IND status = %u inst:[0x%x]\n",((AVRCP_DISCONNECT_IND_T *)message)->status, (uint16)theInst));
            handleAvrcpDisconnectInd(theInst, (AVRCP_DISCONNECT_IND_T *)message);
            return;
        }
        case AVRCP_PASSTHROUGH_IND:
        {
            DEBUG_AVRCP(("AVRCP_PASSTHROUGH_IND  opid = 0x%X inst:[0x%x]\n",(uint16)((AVRCP_PASSTHROUGH_IND_T *)message)->opid, (uint16)theInst));
            handleAvrcpPassthroughInd(theInst, (AVRCP_PASSTHROUGH_IND_T *)message);
            return;
        }
        case AVRCP_UNITINFO_IND:
        {
            DEBUG_AVRCP(("AVRCP_UNITINFO_IND inst:[0x%x]\n", (uint16)theInst));
            /* Send a UnitInfo response. We don't have a company ID, so we return all f's */
            AvrcpUnitInfoResponse(theInst->avrcp, 1, subunit_panel, 0, 0xffffff);
            return;
        }
        case AVRCP_SUBUNITINFO_IND:
        {
            DEBUG_AVRCP(("AVRCP_SUBUNITINFO_IND inst:[0x%x]\n", (uint16)theInst));
            handleAvrcpSubUnitInfoInd(theInst, (AVRCP_SUBUNITINFO_IND_T *)message);
              return;
        }
        case AVRCP_VENDORDEPENDENT_IND:
        {
            DEBUG_AVRCP(("AVRCP_VENDORDEPENDENT_IND inst:[0x%x]\n", (uint16)theInst));
            /* Reject vendor requests */
            AvrcpVendorDependentResponse(theInst->avrcp, avctp_response_not_implemented);
            return;            
        }
        default:
        {
            avrcpUnhandledMessage(theInst, id);
            return;
        }
    }
}

