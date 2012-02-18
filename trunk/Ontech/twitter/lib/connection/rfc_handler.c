/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    rfc_handler.c        

DESCRIPTION
	This file implements all the RFCOMM Connection Entity handler functions
    for upstream RFC messages from Bluestack and all downstream messages.

    The Connection Library manages multiple RFCOMM connections limited by
    available resources.  Requests for incoming and outgoing connections are
    handled within this file.  It's possible to handle multiple connection
    requests concurrently.  Each request for an RFCOMM connection is handled
    individually with connection state dynamically allocated on a per 
    connection basis.  This state only has a limited life.  A connection 
    attempt can end in one of two ways, either an RFCOMM is successfully 
    established or alternatively the attempt can fail for a number of different
    reasons.  In either case, once the connection is established or the attempt
    fails, the connection control block (state) is deleted.

NOTES
    
    Multiplexer connection start up & close down.

		Local Device               Remote Device
	Initiator:
		--------------RFC_START_REQ------------>
		<-------------RFC_START_CFM(Pending)----
		<-------------RFC_START_CFM-------------
	Responder:
		<-------------RFC_START_IND-------------
		--------------RFC_START_RES------------>
		<-------------RFC_START_IND-------------
		--------------RFC_START_RES------------>
		<-------------RFC_STARTCMP_IND----------

    Closing down:
		--------------RFC_CLOSE_REQ------------>
	Or:
		<-------------RFC_CLOSE_IND------------

    

    DLC establishment & release.

		Local Device               Remote Device
	Initiator:
		 -------------RFC_PARNEG_REQ------------>
		<-------------RFC_PARNEG_CFM------------ 
		 ------------RFC_ESTABLISH_REQ---------->
		<------------RFC_ESTABLISH_CFM---------- 
	Responder:
		<-------------RFC_PARNEG_IND------------ 
		 -------------RFC_PARNEG_RES------------>
		<------------RFC_ESTABLISH_IND---------- 
		 ------------RFC_ESTABLISH_RES---------->

    Closing down:
		 ------------RFC_RELEASE_REQ------------>
		<------------RFC_RELEASE_IND------------
	Or:
		<------------RFC_RELEASE_IND------------

    Muxliplexer connections have to be shut down
    by sending an RFC_CLOSE_REQ.  This does not 
    happen automatically even if all server channels
    have been released
*/


/****************************************************************************
	Header files
*/
#include    "connection.h"
#include 	"common.h"
#include    "connection_private.h"
#include    "connection_manager.h"
#include	"common.h"
#include 	"dm_link_policy_handler.h"
#include    "rfc_handler.h"

#include    <print.h>
#include    <string.h>
#include    <vm.h>


#define OPTIMAL_START_MTU   (890)


/*****************************************************************************/
static conn_instance* addRfcommConnection(const bdaddr* bd_addr, mux_id_t mux_id, uint8 chan)
{
    CREATE_RFCOMM_CONN_INSTANCE(bd_addr, mux_id, chan);
    return connectionAddInstance(conn);
}


/*****************************************************************************/
static conn_instance* getRfcommConnection(const bdaddr* bd_addr, mux_id_t mux_id, uint8 chan)
{
    CREATE_RFCOMM_CONN_ID(bd_addr, chan, mux_id);
    return connectionGetInstance(conn_rfcomm, new_id, NULL);
}


/*****************************************************************************/
static bool deleteRfcommConnection(const bdaddr* bd_addr, mux_id_t mux_id, uint8 chan)
{
    CREATE_RFCOMM_CONN_ID(bd_addr, chan, mux_id);
    return connectionDeleteInstance(conn_rfcomm, new_id, NULL);
}


/*****************************************************************************/
static bool deleteTaskMap(server_chan_t server_chan)
{
    /* Delete an entry from the task map */
	map_id id;
	id.channel = server_chan;
	return connectionDeleteTaskMap(conn_rfcomm, id);
}


/*****************************************************************************/
static void sendRfcommConnectInd(Task dest, const bdaddr *bd_addr, uint8 channel, uint16 frame_size)
{
    MAKE_CL_MESSAGE(CL_RFCOMM_CONNECT_IND);
    message->bd_addr = *bd_addr;
    message->server_channel = channel;
    message->frame_size = frame_size;
    MessageSend(dest, CL_RFCOMM_CONNECT_IND, message);
}


/*****************************************************************************/
static bool sendRfcommConnectionInd(const bdaddr *bd_addr, uint8 channel, uint16 frame_size, conn_instance *conn)
{
    conn_task_map *map;
    map_id id;
    id.channel = channel;

    /* Get the task map based on the id as this has the task recipe */
    map = connectionFindTaskMap(conn_rfcomm, id);

    if (map)
    {
        Task app_task = 0;

        if (map->task_recipe)
        {
            /*  
                This code handles the task creation on an incoming rfcomm 
                connection however the library handling the rfcomm connection 
                may have as its client another library rather than an 
                application. If this is the case the connection library needs
                to create both tasks. It is assumed that the parent profile 
                (i.e. the task recipe where parent_profile is set to null) 
                is the task instance actually handling the rfcomm messages.
                Currently only two level library nesting is supported but 
                this can be generalised to support more levels. 
            */
            Task child_task = createTaskInstance(map->task_recipe, map->appTask);
            app_task = child_task;

            if (!app_task)
                return 0;

            if ((map->task_recipe)->parent_profile)
            {
                /* Create the parent task */
                app_task = createTaskInstance((map->task_recipe)->parent_profile, child_task);

                if (!app_task)
                    return 0;
            }                
        }
        else
        {
            /* The destination app task is the task we have stored */
            app_task = map->appTask;
        }

        /* Record the client app task */
        conn->config.rfcomm.app_task = app_task;

        /* Send connect indication to profile instance task */
        sendRfcommConnectInd(app_task, bd_addr, channel, frame_size);
    }
    else
        /* Should already be checking for this so panic if we ever get here. */
        Panic();

    return 1;
}


/*****************************************************************************/
static void sendRfcommDisconnectInd(Task task, reason_code_t reason, Sink sink)
{
    if (task)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_DISCONNECT_IND);
        message->status = connectionConvertRfcommDisconnectStatus(reason);
        message->sink = sink;
        MessageSend(task, CL_RFCOMM_DISCONNECT_IND, message);
    }
}


/*****************************************************************************/
static void sendRfcommConnectionCfm(Task task, rfcomm_connect_status status, uint8 channel, uint16 frame_size, Sink sink)
{
    if (task)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_CONNECT_CFM);
        message->status = status;
        message->server_channel = channel;
        message->frame_size = frame_size;
        message->sink = sink;
        MessageSend(task, CL_RFCOMM_CONNECT_CFM, message);
    }
}


/*****************************************************************************/
static void sendRfcommControlInd(Task task, Sink sink, uint16 break_signal, uint16 modem_status)
{
    if (task)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_CONTROL_IND);
        message->sink = sink;
        message->break_signal = break_signal;
        message->modem_status = modem_status;
        MessageSend(task, CL_RFCOMM_CONTROL_IND, message);
    }
}
   

/*****************************************************************************/
static void sendRfcommRegisterCfm(Task appTask, connection_lib_status status, uint8 chan)
{
    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_REGISTER_CFM);
        message->status = status;
        message->server_channel = chan;
        MessageSend(appTask, CL_RFCOMM_REGISTER_CFM, message);
    }
}


/*****************************************************************************/
static void startParnegPhase(const conn_instance* conn)
{
    MAKE_PRIM_T(RFC_PARNEG_REQ);
    prim->mux_id = conn->id.rfcomm_id.mux_id;
    prim->loc_server_chan = conn->id.rfcomm_id.server_channel;
    prim->rem_server_chan = conn->config.rfcomm.remote_server_channel;
    /*
        Most other implementations always allow 2 bytes for length 
        in the RFCOMM field.  To avoid an extra round of negotiation, 
        we deliberately suggest an RFC mtu one less that what we 
        know we can get away with.
    */
    prim->dlc_pars.max_frame_size = conn->config.rfcomm.params.max_frame_size - 1; 
    prim->dlc_pars.credit_flow_ctrl = 1;
    /* This must be zero to avoid confusing the on-chip credit proxy */
    prim->dlc_pars.initial_credits = 0; 

    VmSendRfcommPrim(prim);
}


/*****************************************************************************/
static void startEstablishPhase(conn_instance *conn)
{
    MAKE_PRIM_T(RFC_ESTABLISH_REQ);
    prim->mux_id = conn->id.rfcomm_id.mux_id;
    prim->loc_server_chan = conn->id.rfcomm_id.server_channel;
    prim->rem_server_chan = conn->config.rfcomm.remote_server_channel;
    VmSendRfcommPrim(prim);

    conn->config.rfcomm.state = establish_phase;
}


/*****************************************************************************/
static void startControlPhase(const conn_instance* conn)
{
    MAKE_PRIM_T(RFC_CONTROL_REQ);
    prim->mux_id = conn->id.rfcomm_id.mux_id;
    prim->server_chan = conn->id.rfcomm_id.server_channel;
    prim->control_pars.modem_signal = (CONTROL_MODEM_RTC_MASK | CONTROL_MODEM_RTR_MASK);
    prim->control_pars.break_signal = 0x0;  
    VmSendRfcommPrim(prim);
}

/*****************************************************************************/
static void sendControlSignal(Sink sink, uint16 break_signal, uint16 modem_signal)
{
	MAKE_PRIM_T(RFC_EX_CONTROL_REQ);
    prim->sink = sink;
    prim->dummy = 0;
    prim->control_pars.modem_signal = modem_signal;
    prim->control_pars.break_signal = break_signal;  
    VmSendRfcommPrim(prim);
}


/*****************************************************************************/
static void closeMuxSession(mux_id_t mux_id)
{
    MAKE_PRIM_T(RFC_CLOSE_REQ);
    prim->mux_id = mux_id;
    VmSendRfcommPrim(prim);
}


/*****************************************************************************/
static void releaseServerChannel(Sink sink)
{
    MAKE_PRIM_T(RFC_EX_RELEASE_REQ);
    prim->sink = sink;
    prim->dummy = 0;
    VmSendRfcommPrim(prim);
}


/*****************************************************************************/
static void releaseServerChannelNoSink(mux_id_t mux, server_chan_t chan)
{
    MAKE_PRIM_T(RFC_RELEASE_REQ);
    prim->mux_id = mux;
    prim->server_chan = chan;
    VmSendRfcommPrim(prim);
}


/*****************************************************************************/
static void sendEstablishRes(mux_id_t mux, server_chan_t chan, bool response)
{
    MAKE_PRIM_T(RFC_ESTABLISH_RES);
    prim->mux_id = mux;
    prim->server_chan = chan;
    prim->accept = response;
    VmSendRfcommPrim(prim);
}


/*****************************************************************************/
static void endConnection(const conn_instance *conn)
{
    PRINT(("endConnection\n"));

    switch(conn->config.rfcomm.state)
    {
        case mux_phase:
            /* Close the Mux session */
            PRINT(("Issue RFC_CLOSE_REQ - Mux 0x%x\n", conn->id.rfcomm_id.mux_id));
            closeMuxSession(conn->id.rfcomm_id.mux_id);
            break;

        case disconnected:
        case parneg_phase:
        case establish_phase:
        case modem_status_phase:
            {
                Sink sink = StreamRfcommSink(conn->id.rfcomm_id.mux_id, conn->id.rfcomm_id.server_channel);
                PRINT(("Issue RFC_RELEASE_REQ - Mux0x%x chan 0x%x\n", conn->id.rfcomm_id.mux_id, conn->id.rfcomm_id.server_channel));

                /* Close the server channel */
                if (sink)
                    releaseServerChannel(sink);
                else
                    releaseServerChannelNoSink(conn->id.rfcomm_id.mux_id, conn->id.rfcomm_id.server_channel);
            }
            break;
        case connected:
            break;
    }
}


/*****************************************************************************/
void connectionHandleRfcommRegisterReq(connectionRfcommState *rfcommState, const CL_INTERNAL_RFCOMM_REGISTER_REQ_T *req)
{
    if(!rfcommState->registerLock)
    {
        /* 
            Create an entry in the connection map to enable incoming primitives on this
            server channel to be mapped to the correct task 
        */
        Task connectTask;
        const profile_task_recipe *recipe;
        map_id id;

        id.channel = INVALID_SERVER_CHANNEL;

        /* If we're using lazy tasks store the info otherwise store nulls */
        if (req->theConnectTask && req->task_recipe)
        {
            connectTask = req->theConnectTask;
            recipe = req->task_recipe;
        }
        else
        {
            connectTask = req->theAppTask;
            recipe = 0;
        }

        if(connectionAddTaskMap(conn_rfcomm, id, connectTask, recipe))
        {
            /* Lock RFC registration */
            rfcommState->registerLock = req->theAppTask;

            {
                /* Register with Bluestack so it allocates an RFCOMM channel */
                MAKE_PRIM_T(RFC_REGISTER_REQ);
                prim->phandle = 0;
	            VmSendRfcommPrim(prim);
            }
            
            {
                /* Set up watchdog timeout */
                MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND);
                message->theAppTask = req->theAppTask;
                MessageSendLater(connectionGetCmTask(), CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND, message, RFCOMM_REGISTER_TIMEOUT);
            }
        }
        else
            sendRfcommRegisterCfm(req->theAppTask, fail, INVALID_SERVER_CHANNEL);
    }
    else
    {
        /* 
            RFC registration in progress, post a conditional internal 
            message to queue request for processing later 
        */
        MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_REGISTER_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_RFCOMM_REGISTER_REQ, message, &rfcommState->registerLock);
    }
}


/*****************************************************************************/
void connectionHandleRfcommRegisterCfm(connectionRfcommState *rfcommState, const RFC_REGISTER_CFM_T *cfm)
{
	connection_lib_status status = fail;
	
    /* Cancel the message checking we got a register cfm from BlueStack */
	(void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND);

    if(rfcommState->registerLock)
    {
        if(cfm->accept)
        {
            map_id id, new_id;

            id.channel = INVALID_SERVER_CHANNEL;
            new_id.channel = cfm->server_chan;

            /* Update the task map with the registered server channel. */
            connectionUpdateTaskMap(conn_rfcomm, id, new_id);

            status = success;
        }
        else
        {
            /* Delete task map entry the rfcomm register failed */
            deleteTaskMap(INVALID_SERVER_CHANNEL);
        }

        /* Send register cfm message to Client application */
        sendRfcommRegisterCfm(rfcommState->registerLock, status, status==success?cfm->server_chan:0x00);
				
        /* Reset lock */
        rfcommState->registerLock = 0;
    }
}


/*****************************************************************************/
void connectionHandleRfcommRegisterTimeout(const CL_INTERNAL_RFCOMM_REGISTER_TIMEOUT_IND_T *ind)
{
    /* Delete the entry from the map */
    deleteTaskMap(INVALID_SERVER_CHANNEL);

	/* Send a cfm to the client task */
	sendRfcommRegisterCfm(ind->theAppTask, fail, INVALID_SERVER_CHANNEL);
}


/*****************************************************************************/
void connectionHandleRfcommConnectReq(connectionRfcommState* rfcommState, const CL_INTERNAL_RFCOMM_CONNECT_REQ_T* req)
{
    if(!rfcommState->lock)
    {
        map_id id;

        /* If the client has passed in a server channel use that otherwise look it up */
        if (!req->local_server_channel)
            id = connectionGetIdFromTaskMap(conn_rfcomm, req->theAppTask);
        else
            id.channel = req->local_server_channel;
        
        /* Before connecting check that the client task registered the local server channel */
        if (id.channel == INVALID_SERVER_CHANNEL)
	    {
            /* 
                The local server channel has not been registered, end
                the connection attempt and let the client know 
            */
            sendRfcommConnectionCfm(req->theAppTask, rfcomm_server_channel_not_registered, INVALID_SERVER_CHANNEL, 0, 0);
        }
        else
        {
            /* Instantiate a Connection instance for this request */
            conn_instance *conn = addRfcommConnection(&req->bd_addr, INVALID_MUX_ID, id.channel);
            conn->config.rfcomm.params = req->config;
            conn->config.rfcomm.state = mux_phase;
            conn->config.rfcomm.app_task = req->theAppTask;
            conn->config.rfcomm.remote_server_channel = req->remote_server_channel;

            /* Lock */
            rfcommState->lock = req->theAppTask;

            /* 
                Start connection timer, if this timer fires before a successful connection
                is made to the peer device then the connection attempt will be aborted 
            */
            {
		        MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND);
                message->theAppTask = req->theAppTask;
		        message->bd_addr = req->bd_addr;
                message->server_channel = id.channel;
		        MessageSendLater(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND, message, req->config.timeout);
		    }

            /* 
                This is a request to establish an outgoing connection to a peer device,
                the first task is to establish a Multiplexer (MUX) session 
            */
            {
                MAKE_PRIM_T(RFC_START_REQ);
	            connectionConvertBdaddr_t(&prim->bd_addr, &conn->id.rfcomm_id.bd_addr);
                prim->psm_remote = RFCOMM_PSM;
                prim->respond_phandle = 0x0;

                if (conn->config.rfcomm.params.max_frame_size > OPTIMAL_START_MTU)
                    prim->sys_pars.max_frame_size = conn->config.rfcomm.params.max_frame_size; 
                else
                    prim->sys_pars.max_frame_size = OPTIMAL_START_MTU; 
             
                prim->sys_pars.port_speed = PORT_SPEED_UNUSED;
                VmSendRfcommPrim(prim);
            }
        }
    }
    else
    {
        /* 
            Serialise the connection request by reposting the message conditionally
            on the currently active DLC to this device being completed 
        */
        MAKE_CL_MESSAGE(CL_INTERNAL_RFCOMM_CONNECT_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_REQ, message, &rfcommState->lock);
    }
}


/*****************************************************************************/
void connectionHandleRfcommConnectTimeout(connectionRfcommState *rfcommState, const CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND_T *ind)
{
    /* 
        It's possible to have multiple connections being setup to different 
        devices, therefore first get the connection instance that this 
        message refers.  The mux id will always be invalid at this point
        as it was not known when the message was first posted, therefore we
        search for the connection instance keyed on Bluetooth Device Address
        and server channel 
    */
    conn_instance *conn = getRfcommConnection(&ind->bd_addr, INVALID_MUX_ID, ind->server_channel);

    if(conn)
    {
        /* Depending upon the current state, abort the connection attempt */
        endConnection(conn);

        /* Inform the client application the connection attempt has timed out */
        sendRfcommConnectionCfm(ind->theAppTask, rfcomm_connect_timeout, INVALID_SERVER_CHANNEL, 0, 0);

        /* Remove the connection data entry as its no longer needed */
        (void) deleteRfcommConnection(&ind->bd_addr, INVALID_MUX_ID, ind->server_channel);

        /* Unlock */
        rfcommState->lock = 0;
    }
    else
    {
        /* 
			This should never happen (if we delete the conn data the timeout should 
			be cancelled) so panic.
		*/
        Panic();
    } 
}


/*****************************************************************************/
void connectionHandleRfcommConnectRes(const CL_INTERNAL_RFCOMM_CONNECT_RES_T* res)
{
    map_id id;
    id.channel = res->server_channel;

    if (!connectionFindTaskMap(conn_rfcomm, id))
    {
        /* 
            The local server channel has not been registered, end
            the connection attempt and let the client know 
        */
        sendRfcommConnectionCfm(res->theAppTask, rfcomm_server_channel_not_registered, INVALID_SERVER_CHANNEL, 0, 0);
    }
    else
    {
        /* Determine the connection instance keyed by Mux Id */
        conn_instance *conn = getRfcommConnection(&res->bd_addr, INVALID_MUX_ID, res->server_channel);

        if(conn)
        {
            /* Update connection instance */
            conn->config.rfcomm.params = res->config;

            /* 
                Decide whether the proposed params are acceptable and if so
                accept the connection otherwise reject it, setting the dlc
                params to ones you want 
            */
            if(res->response)
            {
                if (conn->config.rfcomm.state == establish_phase)
                {
                    sendEstablishRes(conn->id.rfcomm_id.mux_id, conn->id.rfcomm_id.server_channel, 1);

                    /* Move to the modem status phase */
                    conn->config.rfcomm.state = modem_status_phase;

                    /* 
                        Send modem status signal, data cannot be exchanged until both
                        sides have exchanged modem status signals 
                    */
                    startControlPhase(conn);
                }   
                else
                {
                    MAKE_PRIM_T(RFC_PARNEG_RES);
                    prim->server_chan = conn->id.rfcomm_id.server_channel;
                    prim->mux_id = conn->id.rfcomm_id.mux_id;

                    /* set the frame size */
                    if (res->config.max_frame_size <= conn->config.rfcomm.maxMtu)
                    {   
                        prim->dlc_pars.max_frame_size = res->config.max_frame_size;
                        conn->config.rfcomm.maxMtu = res->config.max_frame_size;
                    }
                    else
                    {
                        prim->dlc_pars.max_frame_size = conn->config.rfcomm.maxMtu;
                    }                

                    /* set the flow control type */
                    if (conn->config.rfcomm.credit_flow_ctrl)
                        prim->dlc_pars.credit_flow_ctrl = 1;
                    else
                        prim->dlc_pars.credit_flow_ctrl = 0;

                    prim->dlc_pars.initial_credits = 0;

                    VmSendRfcommPrim(prim); 
                }
            }
            else
            {
                if (conn->config.rfcomm.state == establish_phase)
                {
                    sendEstablishRes(conn->id.rfcomm_id.mux_id, conn->id.rfcomm_id.server_channel, 0);
                }
                else
                {
                    conn->config.rfcomm.state = disconnected;

                    /* Client application has rejected connection request */
                    endConnection(conn);
                }
            }
        }
        else
        {
            /* This should never happen */
            Panic();
        }
    }
}


/*****************************************************************************/
void connectionHandleRfcommDisconnectReq(const CL_INTERNAL_RFCOMM_DISCONNECT_REQ_T* req)
{
    if (req->sink)
	    releaseServerChannel(req->sink);
    else
    {
        /* 
            The client may attempt to disconnect the rfcomm before it has been fully 
            established. At this point it doesn't have a valid sink but we still have 
            the connection data so attempt to pull out the right info from there. 
        */                
        conn_instance *conn = connectionGetInstanceByState(establish_phase);
        
        if (conn)
            releaseServerChannelNoSink(conn->id.rfcomm_id.mux_id, conn->id.rfcomm_id.server_channel);
        else
            releaseServerChannel(req->sink);
    }
}


/*****************************************************************************/
void connectionHandleRfcommControlReq(const CL_INTERNAL_RFCOMM_CONTROL_REQ_T* req)
{
    sendControlSignal(req->sink, req->break_signal, req->modem_signal);
}



/***************************************************************************/
/* Bluestack RFC message handlers                                          */
/***************************************************************************/

/****************************************************************************
    This function handler is called in response to an RFC_START_CFM message
    being sent by Bluestack.  This message is sent to indicate the status of 
    the requested RFCOMM MUX session.
*/
void connectionHandleRfcommStartCfm(connectionRfcommState* rfcommState, const RFC_START_CFM_T* cfm)
{
	conn_instance*  conn;
	bdaddr	addr;
	connectionConvertBdaddr(&addr, &cfm->bd_addr);
	
    /* 
        It's possible to have multiple MUX sessions being setup to different 
        devices, therefore first get the connection instance that this 
        message refers.  At this point in the connection state machine, the
        only parameter that uniquely identifies this connection instance is
        the target Bluetooth Device Address 
    */
    conn = getRfcommConnection(&addr, INVALID_MUX_ID, INVALID_SERVER_CHANNEL);

    if(conn)
    {
        rfcomm_connect_status status = rfcomm_connect_failed;
        /* Determine the current status */
        switch(cfm->result_code)
        {
            case RFC_CONNECTION_PENDING:
			{
                /* 
                    Wait for second RFC_START_CFM, if we never get this message
                    then the RFCOMM connection timeout will expire to cancel the
                    connection request 
                */                
                conn->id.rfcomm_id.mux_id = cfm->mux_id;
                break;
			}

            case MUX_ALREADY_OPEN:
            case RFC_SUCCESS:
                {
                    /* 
                        A MUX session has successfully been established or was already
                        open, move to the parameter negotiation phase 
                    */
                    conn->config.rfcomm.state = parneg_phase;
                    
                    /* Add the mux_id */
                    conn->id.rfcomm_id.mux_id = cfm->mux_id;
                    
                    /* Initiate parameter negotiation */
                    startParnegPhase(conn);
                    
                    /* Unlock */
                    rfcommState->lock = 0;
                }
                break;

            case RFC_CONNECTION_REJ_KEY_MISSING:
                status = rfcomm_connect_key_missing;
                /* Fall through */
                
            default:
			{
                /* This indicates that Mux session establishment has failed */
                sendRfcommConnectionCfm(conn->config.rfcomm.app_task, status, INVALID_SERVER_CHANNEL, 0, 0);
            
                /* CAncel the watchdog timer and delete connection data */
                (void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND);
                (void) deleteRfcommConnection(&addr, INVALID_MUX_ID, INVALID_SERVER_CHANNEL);
	            
                /* Unlock */
                rfcommState->lock = 0;
                break;
			}
        }
    }
    else
    {
        /* 
            This indicates that a previous attempt to open a Mux has failed resulting 
            in a timeout.  We need to tear down this mux 
        */
        closeMuxSession(cfm->mux_id);
    }
}


/****************************************************************************
    This function handler is called in response to an RFC_START_IND message
    being sent by Bluestack.  This message is sent to indicate that a peer
    device is attempting to establish a MUX session.  
    Send back an RFC_START_RES
*/
void connectionHandleRfcommStartInd(const RFC_START_IND_T* ind)
{
	conn_instance*  conn;
	bdaddr	addr;
	connectionConvertBdaddr(&addr, &ind->bd_addr);
	
    /* 
        It's possible to have multiple connections being setup to different 
        devices, therefore first get the connection instance 
    */
    conn = getRfcommConnection(&addr, ind->mux_id, INVALID_SERVER_CHANNEL);

    /* 
        We see two RFC_START_IND for each Mux session, only add a connection
        instance once 
    */
    if(conn == NULL)
    {
        /* Instantiate a Connection instance */
        conn_instance* new_conn = addRfcommConnection(&addr, ind->mux_id, INVALID_SERVER_CHANNEL);

        new_conn->config.rfcomm.state = mux_phase;
    
        /* 
            At this point we don't know the server channel that the remote device
            is attempting to connect too, therefore we can't find out the registered
            client application task.  This will be filled in later. 
        */
        new_conn->config.rfcomm.app_task = 0;

        /* Keep a record of the MTU */
        new_conn->config.rfcomm.maxMtu = ind->sys_pars.max_frame_size;

        /* Add a dummy channel entry to assist parneg ind test */
        new_conn->id.rfcomm_id.server_channel = 0xff;
    }

    /* Send an RFC_START_RES message primitive to Bluestack*/
    {
        MAKE_PRIM_T(RFC_START_RES);
        prim->mux_id = ind->mux_id;
        prim->accept = TRUE;
        prim->sys_pars.port_speed = ind->sys_pars.port_speed;        
        prim->sys_pars.max_frame_size = ind->sys_pars.max_frame_size;
        prim->respond_phandle = INVALID_PHANDLE;
        VmSendRfcommPrim(prim);
    }
}


/****************************************************************************
    This function handler is called in response to an RFC_STARTCMP_IND message
    being sent by Bluestack.  This message is sent to indicate that the
    underlying L2CAP connection has been established.  Providing the result
    code is Success then we need to store the L2CAP MTU size
*/
void connectionHandleRfcommStartCmpInd(const RFC_STARTCMP_IND_T* ind)
{
    /* 
        It's possible to have multiple connections being setup to different 
        devices, therefore first get the connection instance identified by Mux id 
    */
    conn_instance *conn = getRfcommConnection(NULL, ind->mux_id, INVALID_SERVER_CHANNEL);

    if(conn)
    {
        if (ind->result_code == RFC_SUCCESS)
        {
            /* Record frame size */
            conn->config.rfcomm.maxMtu = ind->sys_pars.max_frame_size;

            /* Move to the Parameter Negotiation state */
            conn->config.rfcomm.state = parneg_phase;
        }
        else
        {
            /* The incoming connection has failed, silently abort connection */
            (void) deleteRfcommConnection(NULL, ind->mux_id, INVALID_SERVER_CHANNEL);
        }
    }
    else
    {
        /* This should never happen */
        Panic();
    }
}


/****************************************************************************
    This function handler is called in response to an RFC_PARNEG_IND message
    being sent by Bluestack.	
*/
void connectionHandleRfcommParnegInd(const RFC_EX_PARNEG_IND_T* ind)
{
    map_id id;
    id.channel = ind->server_chan;

    if (!connectionCheckIdInTaskMap(conn_rfcomm, id))
	{
        /* 
			The local server channel was not registered by the client task therefore
			we cannot inform the Client application as we do not have their task id.
			Just drop the connection.
		*/        
        Sink sink = StreamRfcommSink(ind->mux_id, ind->server_chan);

        if (sink)
            releaseServerChannel(sink);
        else
            releaseServerChannelNoSink(ind->mux_id, ind->server_chan);
    }
    else
    {
        /* Get connection instance */
        conn_instance *conn = getRfcommConnection(NULL, ind->mux_id, 0xff);

        if(conn)
        {
			conn_instance *conn_duplicate = getRfcommConnection(NULL, ind->mux_id, ind->server_chan);

			/* Connection entry exists this must be a multiple PN round */
			if (conn_duplicate)
			{
				/* Set the frame size */
				if (ind->dlc_pars.max_frame_size <= conn->config.rfcomm.maxMtu)
					conn_duplicate->config.rfcomm.maxMtu = ind->dlc_pars.max_frame_size;

				{
					/* Send back a response */
					MAKE_PRIM_T(RFC_PARNEG_RES);
					prim->server_chan = conn_duplicate->id.rfcomm_id.server_channel;
					prim->mux_id = conn_duplicate->id.rfcomm_id.mux_id;
					prim->dlc_pars.max_frame_size = conn_duplicate->config.rfcomm.maxMtu;
					prim->dlc_pars.credit_flow_ctrl = conn_duplicate->config.rfcomm.credit_flow_ctrl;            
					VmSendRfcommPrim(prim);
				}
			}
			else
			{
                if (sendRfcommConnectionInd(&conn->id.rfcomm_id.bd_addr, ind->server_chan, (ind->dlc_pars.max_frame_size <= conn->config.rfcomm.maxMtu) ? ind->dlc_pars.max_frame_size : conn->config.rfcomm.maxMtu, conn))
                {
				    /* Update instance with the Server Channel */
				    conn->id.rfcomm_id.server_channel = ind->server_chan;
				
				    /* Set the frame size */
				    if (ind->dlc_pars.max_frame_size <= conn->config.rfcomm.maxMtu)
					    conn->config.rfcomm.maxMtu = ind->dlc_pars.max_frame_size;
				
				    /* Update instance with flow control type */
				    conn->config.rfcomm.credit_flow_ctrl = ind->dlc_pars.credit_flow_ctrl;
                }
                else
                    releaseServerChannelNoSink(ind->mux_id, ind->server_chan);
			}
        }
        else
        {
            /* 
                This indicates that there is already a Multiplexer session open.
                This is the first indication that another server channel is being opened 
            */
			conn_instance *new_conn;
			bdaddr	addr;
			connectionConvertBdaddr(&addr, &ind->bd_addr);

            /* Instantiate a Connection instance */
            new_conn = addRfcommConnection(&addr, ind->mux_id, ind->server_chan);

            /* Keep a record of the MTU */
            new_conn->config.rfcomm.maxMtu = ind->dlc_pars.max_frame_size;

            /* Update instance with flow control type */
            new_conn->config.rfcomm.credit_flow_ctrl = ind->dlc_pars.credit_flow_ctrl;

            /* Inform client application of an incoming connection request */
            if (!sendRfcommConnectionInd(&new_conn->id.rfcomm_id.bd_addr, new_conn->id.rfcomm_id.server_channel, new_conn->config.rfcomm.maxMtu, new_conn))
            {
                /* Delete the entry and reject the connection if client task could not be created. */
                deleteRfcommConnection(&addr, ind->mux_id, ind->server_chan);
                releaseServerChannelNoSink(ind->mux_id, ind->server_chan);
            }
        }
    }
}


/*****************************************************************************/
void connectionHandleRfcommParnegCfm(const RFC_PARNEG_CFM_T* cfm)
{
    /* Get the connection instance data keyed by server channel and mux id */
    conn_instance *conn = getRfcommConnection(NULL, cfm->mux_id, cfm->server_chan);

    if(conn)
    {
        /* Record frame size */
        conn->config.rfcomm.maxMtu = cfm->dlc_pars.max_frame_size;
    
        /* Establish a connection */
        startEstablishPhase(conn);
    }
    else
    {
        /* This should never happen */
        Panic();
    }
}


/*****************************************************************************/
void connectionHandleRfcommEstablishInd(const RFC_EX_ESTABLISH_IND_T* ind)
{
    /* Get the connection instance data keyed by server channel and mux id */
    conn_instance *conn = getRfcommConnection(NULL, ind->mux_id, ind->server_chan);

    if(conn)
    {
        sendEstablishRes(conn->id.rfcomm_id.mux_id, conn->id.rfcomm_id.server_channel, 1);
        
        /* Move to the modem status phase */
        conn->config.rfcomm.state = modem_status_phase;

        /* 
            Send modem status signal, data cannot be exchanged until both
            sides have exchanged modem status signals 
        */
        startControlPhase(conn);
    }
    else
    {
        /* Mux already open we don't have to go through parneg again */
        map_id id;
        id.channel = ind->server_chan;
        
        /* Check if this server channel has been registered */
        if (connectionCheckIdInTaskMap(conn_rfcomm, id))
        {
            conn_instance *new_conn;
            bdaddr	addr;

		    connectionConvertBdaddr(&addr, &ind->bd_addr);

            /* Instantiate a Connection instance */
            new_conn = addRfcommConnection(&addr, ind->mux_id, ind->server_chan);
      
            /* Record the client app task */
            new_conn->config.rfcomm.state = establish_phase;

            /* Inform client application of an incoming connection request */
            if (!sendRfcommConnectionInd(&addr, ind->server_chan, 0, new_conn))
            {
                /* Delete the entry and reject the connection if client task could not be created. */
                deleteRfcommConnection(&addr, ind->mux_id, ind->server_chan);
                sendEstablishRes(conn->id.rfcomm_id.mux_id, conn->id.rfcomm_id.server_channel, 0);
            }
        }
        else
        {
            /* This server channel has not been registered so reject the connection */ 
            sendEstablishRes(conn->id.rfcomm_id.mux_id, conn->id.rfcomm_id.server_channel, 0);
        }
    }
}


/*****************************************************************************/
void connectionHandleRfcommEstablishCfm(const RFC_ESTABLISH_CFM_T* cfm)
{
    /* Get the connection instance data keyed by server channel and mux id */
    conn_instance *conn = getRfcommConnection(NULL, cfm->mux_id, cfm->server_chan);

    if(conn)
    {
        if(cfm->result_code == RFC_SUCCESS)
        {
            /* Move to the modem status phase */
            conn->config.rfcomm.state = modem_status_phase;

            /* 
				Send modem status signal, data cannot be exchanged until both
				sides have exchanged modem status signals 
			*/
            startControlPhase(conn);
        }
        else if (cfm->result_code == DLC_ALREADY_EXISTS)
        {
            /* Send a cfm to the app indicating an error has ocurred*/
            sendRfcommConnectionCfm(conn->config.rfcomm.app_task, rfcomm_connect_channel_already_open, INVALID_SERVER_CHANNEL, 0, 0);

            /* Cancel the connect timeout and clean up the connection state data. */
            (void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND);
            (void) deleteRfcommConnection(NULL, cfm->mux_id, cfm->server_chan);
        }
        else if (cfm->result_code == REMOTE_REFUSAL)
        {
            /* Connection establishment failed, clean up */
            endConnection(conn);

			/* Inform the app of the failure */
            sendRfcommConnectionCfm(conn->config.rfcomm.app_task, rfcomm_connect_rejected, INVALID_SERVER_CHANNEL, 0, 0);

			/* Cancel the connect timeout and clean up the connection state data. */
            (void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND);
            (void) deleteRfcommConnection(NULL, cfm->mux_id, cfm->server_chan);
        }
		else if (cfm->result_code == RFC_CONNECTION_REJ_KEY_MISSING)
		{
			/* Connection establishment failed, clean up */
            endConnection(conn);

			/* Inform the app of the failure */
            sendRfcommConnectionCfm(conn->config.rfcomm.app_task, rfcomm_connect_key_missing, INVALID_SERVER_CHANNEL, 0, 0);

			/* Cancel the connect timeout and clean up the connection state data. */
            (void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND);
            (void) deleteRfcommConnection(NULL, cfm->mux_id, cfm->server_chan);
		}
        else
        {
			/* Connection establishment failed, clean up */
            endConnection(conn);

			/* Inform the app of the failure */
            sendRfcommConnectionCfm(conn->config.rfcomm.app_task, rfcomm_connect_failed, INVALID_SERVER_CHANNEL, 0, 0);

			/* Cancel the connect timeout and clean up the connection state data. */
            (void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND);
            (void) deleteRfcommConnection(NULL, cfm->mux_id, cfm->server_chan);
        }
    }
	/* 
		This could happen if the connect timeout has triggered and cleaned up the connection 
		state entry and then we get an RFC_ESTABLISH_CFM indicating the remote end failed to 
		respond. We should just ignore this cfm. 
	*/
}


/*****************************************************************************/
void connectionHandleRfcommControlInd(const RFC_CONTROL_IND_T *ind)
{
    /* Get the connection instance data keyed by server channel and mux id */
    conn_instance *conn = getRfcommConnection(NULL, ind->mux_id, ind->server_chan);
	
    if(conn)
    {
		/* Get the connection sink */
		Sink sink = StreamRfcommSink(ind->mux_id, ind->server_chan);

        /* Cancel the connection timeout */
	    (void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND);		

        /* Connection establishment complete, let the client application know */     
        sendRfcommConnectionCfm(conn->config.rfcomm.app_task, rfcomm_connect_success, ind->server_chan, conn->config.rfcomm.maxMtu, sink);

		/* Associate the task with its sink */
		(void) MessageSinkTask(sink, conn->config.rfcomm.app_task);
		
        /* No longer require connection instance data */
        (void) deleteRfcommConnection(NULL, ind->mux_id, ind->server_chan);
    }
    else
    {
        /* 
            This indicates that the connection is already established and
            as we throw away connection state on successful connection 
            establishment, we wouldn't expect to find a connection instance 
        */
        Task task;
        Sink sink = StreamRfcommSink(ind->mux_id, ind->server_chan);
        task = MessageSinkGetTask(sink);

        /* Send modem status indication */
        if(task)
            sendRfcommControlInd(task, sink, ind->control_pars.break_signal, ind->control_pars.modem_signal);
    }
}


/*****************************************************************************/
void connectionHandleRfcommReleaseInd(const RFC_EX_RELEASE_IND_T *ind)
{
    /* Get the connection instance data keyed by server channel and mux id */
    conn_instance *conn = getRfcommConnection(NULL, ind->mux_id, ind->server_chan);

    if(conn)
    {
        /* 
            If the client application task id is valid then notify them of
            the connection status.  If the task is not valid then we never
            informed the client application task that an incoming connection
            was active therefore they do not need to know that the attempt
            has been terminated for some reason 
        */
        if(conn->config.rfcomm.app_task)
        {
            /* Cancel the connection timeout */
	        (void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_RFCOMM_CONNECT_TIMEOUT_IND);

            if (conn->config.rfcomm.state != disconnected)
            {
                /* Connection establishment not completed, connection failed */
                if (ind->reason_code == REMOTE_REFUSAL)
                    sendRfcommConnectionCfm(conn->config.rfcomm.app_task, rfcomm_connect_rejected, ind->server_chan, 0, 0);
                else
                    sendRfcommConnectionCfm(conn->config.rfcomm.app_task, rfcomm_connect_failed, ind->server_chan, 0, 0);

                if(ind->num_server_chans == 1)
                {
                    /* Close down mux if we opened it */
                    if((ind->reason_code == RFC_SUCCESS)||(ind->reason_code == REMOTE_REFUSAL))
                        closeMuxSession(ind->mux_id);
                }       
            }

            /* No longer require connection instance data */
            (void) deleteRfcommConnection(NULL, ind->mux_id, ind->server_chan);
        }
    }
    else
    {
        /* 
            This indicates that the connection was fully established and
            as we throw away connection state on successful connection 
            establishment, we wouldn't expect to find a connection instance 

            Check if this is the last DLC on this mux, RFC_EX_RELEASE_IND tells us 
            how many DLC's are still open
        */
        Sink sink;

        if(ind->num_server_chans == 1)
        {
            /* Close down mux if we opened it */
            if(ind->reason_code == RFC_SUCCESS)
            {
                closeMuxSession(ind->mux_id);
            }
        }

        /* 
            Send notification to the client application that the connection has
            been disconnected 
        */
        sink = StreamRfcommSink(ind->mux_id, ind->server_chan);        
        sendRfcommDisconnectInd(MessageSinkGetTask(sink), ind->reason_code, sink);
    }
}


/*****************************************************************************/
void connectionHandleRfcommPortNegInd(const RFC_PORTNEG_IND_T* ind)
{
	MAKE_PRIM_T(RFC_PORTNEG_RES);
    prim->mux_id = ind->mux_id;
    prim->server_chan = ind->server_chan;
	prim->port_pars = ind->port_pars;
    VmSendRfcommPrim(prim);
}
