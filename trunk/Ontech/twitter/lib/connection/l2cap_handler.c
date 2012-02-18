/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    l2cap_handler.c        

DESCRIPTION
	This file contains the guts of the L2CAP connection manager		

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "common.h"
#include "connection_private.h"
#include "connection_manager.h"
#include "dm_link_policy_handler.h"
#include "l2cap_handler.h"

#include <panic.h>
#include <print.h>
#include <vm.h>
#ifdef CONNECTION_DEBUG_LIB
#include <bdaddr.h>
#endif

/* Extract the Qos options */
static uint32 getNextQosOpt(const uint8 *options)
{
	return (((uint32) options[3]<<24) | ((uint32) options[2]<<16) | ((uint32) options[1]<<8) | options[0]);
}


/* Init QoS struct to default values */
static void setQosToDefault(qos_flow *qos)
{
	qos->service_type = DEFAULT_SERVICE_TYPE;
	qos->token_rate = DEFAULT_TOKEN_RATE;
	qos->token_bucket = DEFAULT_TOKEN_BUCKET;
	qos->peak_bw = DEFAULT_PEAK_BW;
	qos->latency = DEFAULT_LATENCY;
	qos->delay_var = DEFAULT_DELAY_VAR;
}


/* Unfortunately we need to do this to work around a compiler bug */
#define same(a,b) (((a)^(b))==0)

/* Return true if all the values in the QoS struct are set to defaults, false otherwise */
static bool isQosDefault(const qos_flow *qos)
{
	return (qos->service_type == DEFAULT_SERVICE_TYPE &&
		same(qos->token_rate, DEFAULT_TOKEN_RATE) &&
		same(qos->token_bucket, DEFAULT_TOKEN_BUCKET) &&
		same(qos->peak_bw, DEFAULT_PEAK_BW) &&
		same(qos->latency, DEFAULT_LATENCY) &&
		same(qos->delay_var, DEFAULT_DELAY_VAR));
}


static void convertToQosFlow_t(const qos_flow *in, QOS_FLOW_T *out)
{
	out->service_type = in->service_type;
	out->token_rate = in->token_rate;
	out->token_bucket = in->token_bucket;
	out->peak_bw = in->peak_bw;
	out->latency = in->latency;
	out->delay_var = in->delay_var;
}


/* Retrieve a task id from the task map */
static Task getTask(uint16 psm)
{
	map_id id;
	id.psm = psm;

	return connectionGetTaskFromMap(conn_l2cap, id);    
}


/* Delete an entry from the task map */
static bool deleteTaskMap(uint16 psm)
{
	map_id id;
	id.psm = psm;
	return connectionDeleteTaskMap(conn_l2cap, id);
}


/* Create a connection state and add it to the linked list */
static void addConnection(uint16 psm_local, cid_t cid, bdaddr addr, const l2cap_config_params *config_params, identifier_t id, Task app_task)
{
	CREATE_L2CAP_CONN_INSTANCE(psm_local, cid);

	if (config_params)
	{
		conn->config.l2cap.config = *config_params;
	}
	else
	{
		conn->config.l2cap.config.mtu_local = MINIMUM_MTU;
		conn->config.l2cap.config.mtu_remote_min = MINIMUM_MTU;
		conn->config.l2cap.config.flush_timeout = DEFAULT_FLUSH_TO;
		conn->config.l2cap.config.accept_non_default_flush = 1;
		setQosToDefault(&conn->config.l2cap.config.qos);			
		conn->config.l2cap.config.accept_qos_settings = 1;
	}

	conn->appTask = app_task;
	conn->addr = addr;
	
	conn->config.l2cap.config_half_done = 0;
	conn->config.l2cap.id = id;
	conn->config.l2cap.app_task = app_task;
	conn->config.l2cap.connect_status = l2cap_connect_failed;
	conn->next = 0;	
		
	(void) connectionAddInstance(conn);
}		


/* Delete connection state instance from the linked list */
static void deleteConnection(uint16 psm, cid_t cid, const bdaddr *bd_addr)
{
	CREATE_L2CAP_CONN_ID(psm, cid);
	(void) connectionDeleteInstance(conn_l2cap, new_id, bd_addr);
}


/* Retrieve a connection state for the linked list */
static conn_instance *getConnection(uint16 psm, cid_t cid, const bdaddr *bd_addr)
{
	CREATE_L2CAP_CONN_ID(psm, cid);	
	return connectionGetInstance(conn_l2cap, new_id, bd_addr);
}


/* Create a register confirm message and send it to the supplied task */
static void sendL2capRegisterCfmToTask(Task appTask, connection_lib_status status, uint16 psm)
{
	MAKE_CL_MESSAGE(CL_L2CAP_REGISTER_CFM);
	message->status = status;
	message->psm = psm;
	MessageSend(appTask, CL_L2CAP_REGISTER_CFM, message);

	PRINT(("CL_L2CAP_REGISTER_CFM status 0x%x psm 0x%x\n", status, psm));
}


/* Create a connect cfm message and send it to the supplied task */
static void sendL2capConnectCfmToTask(Task appTask, l2cap_connect_status status, uint16 psm, Sink sink, uint16 mtu, uint16 timeout, const qos_flow *qos, cid_t cid)
{
	/* Cancel the timeout message */
	(void) MessageCancelFirst(connectionGetCmTask(), CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND);

	PRINT(("CL_L2CAP_CONNECT_CFM status 0x%x psm 0x%x\n", status, psm));

	{
		MAKE_CL_MESSAGE(CL_L2CAP_CONNECT_CFM);
		message->status = status;
		message->psm_local = psm;
		message->sink = sink;
        message->connection_id = cid;
		message->mtu_remote = mtu;
		message->flush_timeout_remote = timeout;

		if (qos)
			message->qos_remote = *qos;
		else
			setQosToDefault(&message->qos_remote);

		MessageSend(appTask, CL_L2CAP_CONNECT_CFM, message);
	}
}


/* Create a disconnect ind message and send it to the supplied task */
static void sendL2capDisconnectIndToTask(Task appTask, l2cap_disconnect_status status, Sink sink)
{
	/* Sink return_sink; */
	MAKE_CL_MESSAGE(CL_L2CAP_DISCONNECT_IND);
	message->sink = sink;
	message->status = status;
	MessageSend(appTask, CL_L2CAP_DISCONNECT_IND, message);

	PRINT(("CL_L2CAP_DISCONNECT_IND status 0x%x\n", status));
}


/* Send an L2CAP connect response message */
static void sendL2capConnectRes(identifier_t id, cid_t cid, response_t resp)
{
	MAKE_PRIM_T(L2CA_CONNECT_RSP);
	prim->identifier = id;
    prim->cid = cid;
    prim->response = resp;
	VmSendL2capPrim(prim);
}


/* Send an L2CAP config request message */
static void sendL2capConfigReq(cid_t cid, const conn_instance *this_conn)
{	
	options_t opts;	
	
	MAKE_PRIM_T(L2CA_CONFIG_REQ);
	prim->cid = cid;
	
	/* Init the options specifier */
	opts = 0;
	
	/* Indicate we're specifying an MTU if its not the default */
	if (this_conn->config.l2cap.config.mtu_local != DEFAULT_MTU)
		opts |= SPECIFY_MTU;
	
	prim->in_mtu = this_conn->config.l2cap.config.mtu_local;
	
	/* Indicate we're specifying the flush timeout if not set to default */
	if (this_conn->config.l2cap.config.flush_timeout != DEFAULT_FLUSH_TO)
		opts |= SPECIFY_FLUSH;
	
	prim->outflush_to = this_conn->config.l2cap.config.flush_timeout;
	
	/* Indicate we're specifying the QoS if not set to default */
	if(!isQosDefault(&this_conn->config.l2cap.config.qos))
		opts |= SPECIFY_QOS;		
	
	convertToQosFlow_t(&this_conn->config.l2cap.config.qos, &prim->outflow);
	
	prim->options = opts;
	prim->more_flag = 0;
	VmSendL2capPrim(prim);
	
	PRINT(("CONFIG REQ cid %d\n", cid));
}


/* Send an L2CAP config response message */
static void sendL2capConfigRes(cid_t cid, identifier_t id, uint8 *opts, uint16 size, response_t resp)
{
	MAKE_PRIM_T(L2CA_CONFIG_RSP);
	prim->cid = cid;
	prim->identifier = id;
	prim->more_flag = 0;
	prim->size_options = size;

	if (size)
		prim->options = VmGetHandleFromPointer(opts);
	else
		prim->options = 0;

    prim->response = resp;
	VmSendL2capPrim(prim);

	PRINT(("CONFIG RSP cid %d\n", cid));
}


/* Send an L2CAP disconnect request message */
static void sendL2capDisconnectReq(Sink sink)
{	
	MAKE_PRIM_T(L2CA_EX_DISCONNECT_REQ);
	prim->sink = sink;
	VmSendL2capPrim(prim);
}

static void sendInterlockL2capDisconnectRsp(identifier_t id, cid_t cid)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP);
    message->id = id;
    message->cid = cid;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP, message);
}


/* Check if we have conpleted the channel configuration and if we have send a cfm to the client */
static void configDanceCompleted(uint16 psm, cid_t cid)
{	
	conn_instance *this_conn = getConnection(psm, cid, NULL);

	/* Get connection state */
	if (this_conn)
	{
		if (!this_conn->config.l2cap.config_half_done)
		{
			/* Config not complete - only half done */
			this_conn->config.l2cap.config_half_done = 1;
		}
		else
		{
			Task app_task;
			bdaddr addr;
			Sink sink = StreamL2capSink(cid);
            
            if (sink == 0)
                Panic();

			SinkGetBdAddr(sink, &addr);
#ifdef CONNECTION_DEBUG_LIB
            PanicFalse(BdaddrIsSame(&this_conn->addr, &addr));
#endif
			app_task = this_conn->config.l2cap.app_task;

			/* Send connect cfm to client */
			sendL2capConnectCfmToTask(app_task, 
				l2cap_connect_success, 
				psm, sink, 
				this_conn->config.l2cap.config.mtu_remote_min, 
				this_conn->config.l2cap.config.flush_timeout, 
				&this_conn->config.l2cap.config.qos,
                cid);

			/* Associate the task with its sink */
			(void) MessageSinkTask(sink, app_task);

			/* Delete the connection state */
			deleteConnection(psm, cid, NULL);

			/* See if this completed connection should be added to the multiple channel connection list */
			connectionStoreCompletedConnection(app_task, addr, psm, cid);
		}
	}
	/* Connection state not found - we could get here if the config was a retransmission */
}


/* The more to come flag in config is set, so handle that case here */
static void configMoreFlagSet(uint16 psm, cid_t cid)
{
	conn_instance *this_conn = getConnection(psm, cid, NULL);

	/* 
		Even in L2CAP 1.2 all options (43 octets) fit into the min MTU size
		(48 octets) so we should not see this (see p48 of L2CAP spec v1.2)
				
		TODO B-5017 should implement this at some point though!
	*/		

	/* 
		Config attempt failed - config had more bit set.
		Update connect status - tell client when connection has disconnected.
	*/
	this_conn->config.l2cap.connect_status = l2cap_connect_failed_more_flag_set;
	
	/* Disconnect the connection - no point proceeding if we can't handle this */
	sendL2capDisconnectReq(StreamL2capSink(cid));
}


/* Parse the config options sent by the remote device */
static uint16 extractConfigOptions(uint8 *options, uint16 size_opts, uint16 *mtu, uint16 *timeout, qos_flow *qos)
{	
	uint16 failed = 0;

	/* Init params to defaults since if they are not specified this is what we'll use */
	*mtu = DEFAULT_MTU;
	*timeout = DEFAULT_FLUSH_TO;
	setQosToDefault(qos);

	if ((size_opts == 4) && (options[0]==1))
	{
		/* Optimised route for remote device which just sends MTU. */
		*mtu = ((options[3]<<8) | options[2]);
	}
	else
	{
		/* Need to parse the lot */
		uint8 *failed_options = options;
				
		while(size_opts)
		{
			uint8 type = options[0];
			uint8 entry_size = options[1];
			
			options += 2;
			size_opts -= 2;
			
			/* Check if option must be recognised or is a hint */
			if (!(type & 0x80))
			{
				/* Option must be recognised */
				if (type > 0x04)
				{
					/* 
						Option type not currently defined in L2CAP spec. Its ok to
						modify the options since each entry in IN is larger than
						in OUT so we're only trashing read data.
					*/
					failed_options[failed] = type;
					failed_options[failed+1] = 0;
					
					failed+=2;
				}
				else
				{
					/* Option type defined in L2CAP spec - work out what it is */
					if ((type & 0x7f) == MTU_OPTION)
					{
						/* Extract the MTU */
						*mtu = (options[1]<<8 | options[0]);
					}
					else if ((type & 0x7f) == FLUSH_OPTION)
					{
						/* Extract the flush timeout */
						*timeout = (options[1]<<8 | options[0]);
					}
					else if ((type & 0x7f) == QOS_OPTION)
					{
						/* Extract the QoS settings */
						qos->service_type = options[0];
						qos->token_rate = getNextQosOpt(options);
						qos->token_bucket = getNextQosOpt((options+4));
						qos->peak_bw = getNextQosOpt((options+8));
						qos->latency = getNextQosOpt((options+12));
						qos->delay_var = getNextQosOpt((options+16));
					}
					else if ((type & 0x7f) == 0x04)
					{
						/* 
							TODO  B-5018
							Extract the retransmission and flow control option.
						*/
					}
					else
					{
						/* Something has gone wrong, add this to tha failed options */ 
						failed_options[failed] = type;
						failed_options[failed+1] = 0;
						
						failed+=2;
					}
				}
			}
			/* else - Option is a hint - skip it and continue */
			
			/* Move to the next option */
			options += entry_size;
			size_opts -= entry_size;
		}				
	}
		
	/* Return size of failed options */
	return failed;
}


static void handleDisconnect(cid_t cid, uint16 disc_result, bool link_loss)
{
	conn_instance *this_conn = getConnection(L2CA_PSM_INVALID, cid, NULL);

	if (this_conn)
	{
		/* 
			Connection has not been fully established otherwise we would not
			have found this entry. 
		*/

		/* Tell the app the connect attempt failed */
		sendL2capConnectCfmToTask(this_conn->config.l2cap.app_task, 
			this_conn->config.l2cap.connect_status, this_conn->id.l2cap_id.psm, 0, 0, 0, 0, cid);

		/* Remove the connection state entry. */
		deleteConnection(this_conn->id.l2cap_id.psm, cid, NULL);
	}
	else
	{
		/* 
			We've just disconnected an active L2CAP connection. Once a connection 
			is established, its connection state entry is removed so we don't
			expect to find an entry for it so this is OK.

            If both devices tried to close the channel at the same time, we
            may be called twice; once for the L2CA_DISCONNECT_IND and once for
            the L2CA_DISCONNECT_CFM.
            In this case, we must be careful only to post the message the first
            message i.e. if we still think the stream is valid.
		*/
		Sink sink = StreamL2capSink(cid);

		/* Send a disconnect ind to the app */
        if (sink)
		{
			l2cap_disconnect_status status;

			/* Convert the BlueStack status into the connection lib return type */
			switch(disc_result)
			{
				case L2CA_DISCONNECT_SUCCESSFUL:
					status = link_loss ? l2cap_disconnect_link_loss : l2cap_disconnect_successful;
					break;

				case L2CA_DISCONNECT_TIMEOUT:
					status = l2cap_disconnect_timed_out;
					break;

				default:	
					status = l2cap_disconnect_error;
			}

			/* Send a response to the client */
		    sendL2capDisconnectIndToTask(MessageSinkGetTask(sink), status, sink);

			connectionRemoveCompletedConnection(cid);
		}
	}	
}


/****************************************************************************
NAME	
	connectionHandleL2capRegisterReq

DESCRIPTION
	Used by client tasks to register a PSM so remote devices can connect to 
	it. 

RETURNS
	void
*/
void connectionHandleL2capRegisterReq(connectionL2capState *l2capState, const CL_INTERNAL_L2CAP_REGISTER_REQ_T *req)
{
    if (!l2capState->l2capRegisterLock)
    {
        const profile_task_recipe *recipe;
        map_id id;
        id.psm = req->app_psm;
        
        if (req->l2cap_recipe)
            recipe = req->l2cap_recipe;
        else
            recipe = 0;
        
        if (connectionAddTaskMap(conn_l2cap, id, req->connectionTask, recipe))
        {
            /* PSM added successfully sne the L2CAP register req */
            MAKE_PRIM_T(L2CA_REGISTER_REQ);
            prim->phandle = 0;
            prim->psm = req->app_psm;
            VmSendL2capPrim(prim);

            l2capState->l2capRegisterLock = req->clientTask;		
        }
        else
        {
            /* Send an error message since an entry already exists for this PSM */
            sendL2capRegisterCfmToTask(req->clientTask, fail, req->app_psm);
        }
    }
    else
    {
        /* 
            L2cap registration in progress, post a conditional internal 
            message to queue request for processing later 
        */
        MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_REGISTER_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_L2CAP_REGISTER_REQ, message, &l2capState->l2capRegisterLock);
    }
}


/****************************************************************************
NAME	
	connectionHandleL2capRegisterCfm

DESCRIPTION
	Confirmation that a PSM has been registered with the L2CAP layer of
	BlueStack.

RETURNS
	void
*/
void connectionHandleL2capRegisterCfm(connectionL2capState *l2capState, const L2CA_REGISTER_CFM_T *cfm)
{
    if (l2capState->l2capRegisterLock)
    {
        /* Send a cfm to that task that the registration has completed */
        sendL2capRegisterCfmToTask(l2capState->l2capRegisterLock, success, cfm->psm);

        /* Reset the lock so waiting messages are delivered. */
        l2capState->l2capRegisterLock = 0;
    }
}


/****************************************************************************
NAME	
	connectionHandleL2capUnregisterReq

DESCRIPTION
	Request to unregister a particular psm.

RETURNS
	void
*/
void connectionHandleL2capUnregisterReq(const CL_INTERNAL_L2CAP_UNREGISTER_REQ_T *req)
{
	MAKE_CL_MESSAGE(CL_L2CAP_UNREGISTER_CFM);

	if (deleteTaskMap(req->app_psm))
		message->status = success;
	else
		message->status = fail;
				 
	message->psm = req->app_psm;
	MessageSend(req->theAppTask, CL_L2CAP_UNREGISTER_CFM, message);
}


/****************************************************************************
NAME	
	connectionHandleL2capConnectReq

DESCRIPTION
	Request to initiate an L2CAP connection.

RETURNS
	void
*/
void connectionHandleL2capConnectReq(const CL_INTERNAL_L2CAP_CONNECT_REQ_T *req)
{
	conn_task_map *map;
    map_id id;
	uint16 no_instances, total_instances;

    id.psm = req->psm_local;

    /* Get the task map based on the id as this has the task recipe */
    map = connectionFindTaskMap(conn_l2cap, id);

	/* Find if a connection instance exists with this PSM and Task */
	total_instances = connectionGetPsmTaskMatch(req->psm_local, req->theAppTask);

	/* See if any completed connections exist with this PSM and Task */
	no_instances = connectionCompletedConnectionsGetPsmTaskMatch(req->psm_local, req->theAppTask);

	total_instances += no_instances;

	if (map && map->task_recipe)
	{		
		Task task_a = connectionGetPsmBdaddrMatch(req->psm_local, req->bd_addr, &no_instances);
		Task task_b = connectionCompletedConnectionsGetPsmBdaddrMatch(req->psm_local, req->bd_addr, &no_instances);

		/* Check for max channels reached for this Task. Also it won't allow a connection with the same baddr and PSM, but different Tasks. */
		if ((total_instances == map->task_recipe->max_channels) ||
				(task_a && (task_a != req->theAppTask)) ||
				(task_b && (task_b != req->theAppTask)))
		{
			/* Don't allow connection as this task can't handle any more channels */
			sendL2capConnectCfmToTask(req->theAppTask, 
										l2cap_connect_instance_not_found, req->psm_local, 0, 0, 0, 0, 0);
			return;
		}
	}
	
	{
		/* Attempt to connect */
		MAKE_PRIM_T(L2CA_CONNECT_REQ);
		prim->psm_local = req->psm_local;
		connectionConvertBdaddr_t(&prim->bd_addr, &req->bd_addr);
		prim->psm = req->psm_remote;
		prim->packet_type = 0;
		VmSendL2capPrim(prim);
	}

	/* Store info about this connect attempt */
	addConnection(req->psm_local, L2CA_CID_INVALID, req->bd_addr, (l2cap_config_params *) &req->config_params, 0, req->theAppTask);
}


/****************************************************************************
NAME	
	connectionHandleL2capConnectCfm

DESCRIPTION
	Response to an L2CAP connect request

RETURNS
	void
*/
void connectionHandleL2capConnectCfm(const L2CA_CONNECT_CFM_T *cfm)
{
	conn_instance *this_conn;
    bdaddr addr;
    
	connectionConvertBdaddr(&addr, &cfm->bd_addr);

	if (cfm->response == L2CA_CONNECTION_PENDING)
		this_conn = getConnection(cfm->psm, L2CA_CID_INVALID, &addr);
	else
	{
		/* Look for the cid in the connect_cfm */
		this_conn = getConnection(cfm->psm, cfm->cid, &addr);

		/* 
			If cid not found, try to find an empty entry, we'll get to 
			here if the remote end does not send a auth_pending connect 
			response but just accepts directly. 
		*/
		if (!this_conn)
			this_conn = getConnection(cfm->psm, L2CA_CID_INVALID, &addr);
	}

	/* Check we have the connection state stored otherwise we're in trouble */
	if (!this_conn)
	{
		Sink sink = StreamL2capSink(cfm->cid);

		/* Send a connect cfm to the task to indicate that we failed to connect */
		/* NOTE: We can't send to task that created connection, so we have to send to task that registered the PSM for this connection */
		sendL2capConnectCfmToTask(getTask(cfm->psm), l2cap_connect_failed_internal_error, cfm->psm, 0, 0, 0, 0, cfm->cid);
		
		/* Only attempt disconnect if we have a valid sink */
		if (sink)
			/* Something has gone wrong connection state not found - disconnect */
			sendL2capDisconnectReq(sink);
	}
	else
	{
		/* Update the cid in the connection state */
		this_conn->id.l2cap_id.cid = cfm->cid;

		/* Check the response indicates success */
		if (cfm->response == L2CA_CONNECTION_SUCCESSFUL)
		{
			/* Config the connection */
			sendL2capConfigReq(cfm->cid, this_conn);
			
			/* Set a timer - if this expires then disconnect as something has got stuck */
			{	
				MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND);
				message->theAppTask = this_conn->config.l2cap.app_task;
				message->psm = cfm->psm;
				message->cid = cfm->cid;
				MessageSendLater(connectionGetCmTask(), CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND, message, D_SEC(this_conn->config.l2cap.config.timeout));
			}
		}	
		else if (cfm->response == L2CA_CONNECTION_PENDING)
		{
			/* Update the cid in the connection state */
			this_conn->id.l2cap_id.cid = cfm->cid;
		}
		else 
		{
			/* Connect request failed */
			l2cap_connect_status error;
			
			if (cfm->response == L2CA_CONNECTION_FAILED)
				error = l2cap_connect_failed_remote_disc;
			else if(cfm->response == L2CA_CONNECTION_KEY_MISSING)
				error = l2cap_connect_failed_key_missing;
			else
				error = l2cap_connect_failed_remote_reject;
			
			/* Tell the client task the connect attempt failed */
			sendL2capConnectCfmToTask(this_conn->config.l2cap.app_task, error, cfm->psm, 0, 0, 0, 0, cfm->cid);
            
			/* Remove from the list of connection states */		
			deleteConnection(cfm->psm, cfm->cid, &addr);			
		}
	}
}


/****************************************************************************
NAME	
	connectionHandleL2capConfigCfm

DESCRIPTION
	Have received a confirm in response to a config request we sent out

RETURNS
	void
*/
void connectionHandleL2capConfigCfm(const L2CA_CONFIG_CFM_T *cfm)
{
	conn_instance *this_conn = getConnection(L2CA_PSM_INVALID, cfm->cid, NULL);

	PRINT(("CONFIG CFM response 0x%x cid 0x%x\n", cfm->response, cfm->cid));

	/* Check we have the connection state stored otherwise we're in trouble */
	/* We check the connect_status, if it is the default (l2cap_connect_failed) then we haven't
	   aborted the connection attempt */
	if ((this_conn) && (this_conn->config.l2cap.connect_status == l2cap_connect_failed))
	{
		if (cfm->response == CONFIG_SUCCESS)
		{
			/* If connection config completed then we can send a connect cfm to the client task. */
			if (!cfm->more_flag)
			{
				/* If config has completed then we've got a usable L2CAP connection */
				configDanceCompleted(this_conn->id.l2cap_id.psm, cfm->cid);
			}
			else
			{
				/* More flag set - can't cope! */
				configMoreFlagSet(this_conn->id.l2cap_id.psm, cfm->cid);
			}
		}
		else
		{
			/* 
				Config attempt failed - remote device rejected our params.		
				Update connect status - tell client when connection has disconnected.
			*/
			this_conn->config.l2cap.connect_status = l2cap_connect_failed_config_rejected;

			/* Send an L2CAP disconnect */
			sendL2capDisconnectReq(StreamL2capSink(cfm->cid));
		}
	}
	/* Probably a retransmit so ignore */
}


/****************************************************************************
NAME	
	connectionHandleL2capConfigInd

DESCRIPTION
	Have received a config indication with channel params the remote end 
	wishes to use, need to respond either accepting or rejecting these.

RETURNS
	void
*/
void connectionHandleL2capConfigInd(L2CA_CONFIG_IND_T *ind)
{
	/* TODO 

		how do we differentiate between a retransmission and the remote end 
		reconfiguring the channel - which we don't want to support. We need to 
		keep track of the identifier but what if we've already removed the conn state
		looks like we need a timer and once this expires we need to reject all config 
		reqs
	*/
			
	/* Get the connection state */
	conn_instance *this_conn = getConnection(L2CA_PSM_INVALID, ind->cid, NULL);

	PRINT(("CONFIG IND cid 0x%x\n", ind->cid));
		
	/* Check we have the connection state stored otherwise we're in trouble */
	/* We check the connect_status, if it is the default (l2cap_connect_failed) then we haven't
	   aborted the connection attempt */
	if ((this_conn) && (this_conn->config.l2cap.connect_status == l2cap_connect_failed))
	{
		/* Check if the more flag is set */
		if (!ind->more_flag)
		{
			uint8 *remote_options;
			uint16 size_options;
			response_t resp;
			uint16 remote_mtu;
			uint16 remote_timeout;
			qos_flow *remote_qos = PanicUnlessNew(qos_flow);
			
			/* Get a ptr to the options */
			remote_options = VmGetPointerFromHandle(ind->options);
			
			/* Parse the options */
			size_options = extractConfigOptions(remote_options, ind->size_options, &remote_mtu, &remote_timeout, remote_qos);
			
			/* Decide whether to accept/ reject the config depending on the opts */
			if (size_options)
			{
				/* We have options we don't understand so reject the config */
				resp = CONFIG_UNKNOWN;
			}
			else
			{
				/* Free the options array here because we're going to reallocate it */
				if (ind->size_options)
					free(remote_options);
				
				/* Check we like the connection options sent */			
				size_options = 0;
				remote_options = 0;
				resp = CONFIG_SUCCESS;			
								
				/* Check remote MTU is greater than the min MTU specified by the client task */
				if (remote_mtu < this_conn->config.l2cap.config.mtu_remote_min)
				{
					/* Total length of the mtu options we are sending back */
					uint16 mtu_opts_size = (SIZEOF_OPTION_HEADER + SIZEOF_MTU_OPTION);

					/* Send back an acceptable MTU value */
					remote_options = (uint8 *) malloc(mtu_opts_size);

                    if (remote_options)
                    {
					    /* Set the reject code to unacceptable params */
					    resp = CONFIG_UNACCEPTABLE;					

					    /* Supply the MTU oopts we're willing to accept */
					    remote_options[size_options] = MTU_OPTION;
					    remote_options[size_options+1] = SIZEOF_MTU_OPTION;
					    remote_options[size_options+2] = (this_conn->config.l2cap.config.mtu_remote_min & 0xff);
					    remote_options[size_options+3] = (this_conn->config.l2cap.config.mtu_remote_min >> 8);

					    size_options += mtu_opts_size;
                    }
                    else
                        /* Could not alloc array so just reject without reason. */
                        resp = CONFIG_REJECTED;
				}
				else 
				{
					/* MTU acceptable so store it */
					this_conn->config.l2cap.config.mtu_remote_min = remote_mtu;
				}
				
				/* Check the flush timeout is acceptable */
				if (!this_conn->config.l2cap.config.accept_non_default_flush && remote_timeout != DEFAULT_FLUSH_TO)
				{
                    uint16 fto_opts_size = (SIZEOF_OPTION_HEADER + SIZEOF_FLUSH_OPTION);

                    if (remote_options)                    
                        remote_options = (uint8 *) realloc(remote_options, size_options+fto_opts_size);
                    else
                        remote_options = (uint8 *) malloc(fto_opts_size);

                    
					if (remote_options)
                    {
                        resp = CONFIG_UNACCEPTABLE;

					    /* Rejecting param specify value we are willing to accept  */
                        remote_options[size_options] = FLUSH_OPTION;
					    remote_options[size_options+1] = SIZEOF_FLUSH_OPTION;
					    remote_options[size_options+2] = ((DEFAULT_FLUSH_TO) & 0xff);
					    remote_options[size_options+3] = ((DEFAULT_FLUSH_TO) >> 8);

                        size_options += fto_opts_size;
                    }
                    else
                    {
                        /* Could not alloc array so just reject without reason. */
                        resp = CONFIG_REJECTED;
                        free(remote_options);
                    }
				}
				else
				{
					/* Flush timeout acceptable so store it */
					this_conn->config.l2cap.config.flush_timeout = remote_timeout;
				}
				
				/* Check the quality of service settings are acceptable */
				if (!this_conn->config.l2cap.config.accept_qos_settings)
				{
					/* Accept only best effort QoS */
					if (remote_qos->service_type != DEFAULT_SERVICE_TYPE)
					{
						resp = CONFIG_REJECTED;
					}
					else
					{
						/* Service type best_effort - store the QoS we've just agreed to */
						this_conn->config.l2cap.config.qos = *remote_qos;
					}
				}
				else
				{
					/* We'll accept any old settings as long as they don't have wildcards in them */
					if ((remote_qos->token_rate == 0xFFFFFFFF) || (remote_qos->token_bucket == 0xFFFFFFFF))
					{	
						/* Just reject this config and don't bother specifying why */
						resp = CONFIG_REJECTED;
					}
					else
					{
						/* Store the QoS we've just agreed to */
						this_conn->config.l2cap.config.qos = *remote_qos;
					}
				}
				
				if (resp == CONFIG_SUCCESS)
				{
					/* If the config has been completed we've got a usable L2CAP connection */
					configDanceCompleted(this_conn->id.l2cap_id.psm, ind->cid);
				}
				
				/* Else connection state no longer exists */
			}
			
			/* Send the config response to BlueStack. */
			sendL2capConfigRes(ind->cid, ind->identifier, remote_options, size_options, resp);
			
			/* Free the qos data we allocated above */
			free(remote_qos);
		}
		else
		{
			/* Send a response rejecting the config */
			sendL2capConfigRes(ind->cid, ind->identifier, 0, 0, CONFIG_REJECTED);
		
			/* More flag set - can't cope! */
			configMoreFlagSet(this_conn->id.l2cap_id.psm, ind->cid);
		
			/* Free the options array */
			if (ind->size_options)
			{
				ind->options = VmGetPointerFromHandle(ind->options);
				free(ind->options);
			}
		}
	}
	/* Probably a retransmit so ignore */
}


/****************************************************************************
NAME	
	connectionHandleL2capConnectInd

DESCRIPTION
	Indication that the remove device is trying to connect to this device.

RETURNS
	void
*/
void connectionHandleL2capConnectInd(const L2CA_CONNECT_IND_T *ind)
{
    conn_instance *this_conn;
    bdaddr addr;
    
	connectionConvertBdaddr(&addr, &ind->bd_addr);

	/* Check if we already have connection state for this connection */
    this_conn = getConnection(ind->psm, ind->cid, &addr);
    
    PRINT(("CONNECT IND psm 0x%x cid 0x%x\n", ind->psm, ind->cid));
    
    if (!this_conn)
    {
        conn_task_map *map;
        map_id id;


        id.psm = ind->psm;

        /* Get the task map based on the id as this has the task recipe */
        map = connectionFindTaskMap(conn_l2cap, id);

        if (map)
        {
            Task app_task = 0;

            if (map->task_recipe)
            {
                Task child_task;

				if (map->task_recipe->max_channels <= 1)
				{
					child_task = createTaskInstance(map->task_recipe, map->appTask);
				}
				else
				{
					uint16 total_instances, no_instances;
					Task temp_task;
					
					child_task = connectionGetPsmBdaddrMatch(ind->psm, addr, &total_instances);
					temp_task = connectionCompletedConnectionsGetPsmBdaddrMatch(ind->psm, addr, &no_instances);
					
					total_instances += no_instances;
					
					if (total_instances && (child_task || temp_task))
					{
						if (!child_task)
							child_task = temp_task;

						if (total_instances == map->task_recipe->max_channels)
						{
							/* Reached the max channels that this profile can handle, so reject connection. */
							sendL2capConnectRes(ind->identifier, ind->cid, L2CA_CONNECTION_REJ_RESOURCES);
							return;
						}
					}
					else
					{
						child_task = createTaskInstance(map->task_recipe, map->appTask);
					}
				}

                app_task = child_task;

                if (!app_task)
                {
                    /* Could not alloc task to handle connection so reject it. */
                    sendL2capConnectRes(ind->identifier, ind->cid, L2CA_CONNECTION_REJ_RESOURCES);
                    return;
                }

                if ((map->task_recipe)->parent_profile)
                {
                    /* Create the parent task */
                    app_task = createTaskInstance((map->task_recipe)->parent_profile, child_task);

                    if (!app_task)
                    {
                        /* Could not alloc task to handle connection so reject it. */
                        sendL2capConnectRes(ind->identifier, ind->cid, L2CA_CONNECTION_REJ_RESOURCES);
                        return;
                    }
                }
            }
            else
            {
                /* The destination app task is the task we have stored */
                app_task = map->appTask;
            }

            {
                /* 
                    Check with the client that registered the PSM whether it wants 
                    to accept the connect request 
                */
                MAKE_CL_MESSAGE(CL_L2CAP_CONNECT_IND);  				
                message->bd_addr = addr;
                message->psm = ind->psm;
                message->connection_id = ind->cid;
                MessageSend(app_task, CL_L2CAP_CONNECT_IND, message);
            			
				/* Add a connection state instance since we need to store the identifier */
				addConnection(ind->psm, ind->cid, addr, 0, ind->identifier, app_task);			
            }
        }
        else
        {
            /* PSM not registered by any task, reject outright. */
            sendL2capConnectRes(ind->identifier, ind->cid, L2CA_CONNECTION_REJ_PSM);
        }
    }
    else
    {
        /* Must be a retransmit so just accept again */
        sendL2capConnectRes(ind->identifier, ind->cid, L2CA_CONNECTION_SUCCESSFUL);
    }
}


/****************************************************************************
NAME	
	connectionHandleL2capConnectRes

DESCRIPTION
	Handle a response from the client task telling us whether to proceed
	with establishing the L2CAP connection. 

RETURNS
	void
*/
void connectionHandleL2capConnectRes(const CL_INTERNAL_L2CAP_CONNECT_RES_T *res)
{
	/* Get the connection state */
	conn_instance *this_conn = getConnection(res->psm_local, res->cid, NULL);

	/* Check the connection state exists */
	if (this_conn)
	{
		if (res->response)
		{
			/* Update the connection state with the config params sent by the task */
			this_conn->config.l2cap.config = res->config_params;

			/* Update task that handles this connection */
			this_conn->config.l2cap.app_task = res->theAppTask;

			/* Task has accepted the connection so proceed */
			sendL2capConnectRes(this_conn->config.l2cap.id, this_conn->id.l2cap_id.cid, L2CA_CONNECTION_SUCCESSFUL);
			
			/* Send config request as soon as we send the connect response */
			sendL2capConfigReq(this_conn->id.l2cap_id.cid, this_conn);		
			
			
			/* Set a timer - if this expires then disconnect as something has got stuck */
			{
				MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND);
				message->theAppTask = res->theAppTask;
				message->psm = res->psm_local;
				message->cid  = this_conn->id.l2cap_id.cid;
				MessageSendLater(connectionGetCmTask(), CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND, message, D_SEC(this_conn->config.l2cap.config.timeout));
			}
		}
		else
		{
			/* Client task has rejected the connection so reject connect request */
			sendL2capConnectRes(this_conn->config.l2cap.id, this_conn->id.l2cap_id.cid, L2CA_CONNECTION_REJ_RESOURCES);
						
			/* Delete the connection state */
			deleteConnection(res->psm_local, this_conn->id.l2cap_id.cid, NULL);
		}
	}
}


/****************************************************************************
NAME	
	connectionHandleL2capDisconnectReq

DESCRIPTION
	Request by the local device to disconnect the L2CAP connection

RETURNS
	void
*/
void connectionHandleL2capDisconnectReq(const CL_INTERNAL_L2CAP_DISCONNECT_REQ_T *req)
{
	if (req->sink)
	{
		/* Send a disconnect message to BlueStack */
		sendL2capDisconnectReq(req->sink);
	}
	else
		sendL2capDisconnectIndToTask(req->theAppTask, l2cap_disconnect_no_connection, req->sink);
}


/****************************************************************************
NAME	
	connectionHandleL2capDisconnectCfm

DESCRIPTION
	L2CAP connection has been disconnected by the local end.

RETURNS
	void
*/
void connectionHandleL2capDisconnectCfm(const L2CA_DISCONNECT_CFM_T *cfm)
{
	/* Tell the task we disconnected */
	handleDisconnect(cfm->cid, cfm->result, FALSE);
}


/****************************************************************************
NAME	
	connectionHandleL2capDisconnectInd

DESCRIPTION
	L2CAP connection has been disconnected by the remote end.

RETURNS
	void
*/
void connectionHandleL2capDisconnectInd(const L2CA_DISCONNECT_IND_T *ind)
{
	/* 
		If the other end disconnects we might not have an established L2CAP so might need
		to inform the client task. 
	*/
	handleDisconnect(ind->cid, L2CA_DISCONNECT_SUCCESSFUL, (ind->identifier == 0));

    /*
        To prevent race conditions, we must make sure the client knows about
        the disconnect before we inform the remote device the disconnect is
        complete.
        This is achieved by posting a message back to ourselves which is
        added after the CL_L2CAP_DISCONNECT_IND (sent above).
        This message will therefore not be delivered to us until the client
        message has been read and deleted from the global message queue.
    */
	sendInterlockL2capDisconnectRsp(ind->identifier, ind->cid);
}


/****************************************************************************
NAME	
	connectionHandleL2capConnectTimeout

DESCRIPTION
	Connect timer has expired and we still haven't got a connection (if we 
	had this timer would not have fired) so disconnect everything and tell
	the client task.

RETURNS
	void
*/
void connectionHandleL2capConnectTimeout(const CL_INTERNAL_L2CAP_CONNECT_TIMEOUT_IND_T *ind)
{
	/* Get the connection state */
	conn_instance *this_conn = getConnection(ind->psm, ind->cid, NULL);

	if (this_conn)
	{
		/* Update connect status - inform client once connection has disconnected */
		this_conn->config.l2cap.connect_status = l2cap_connect_timeout;

		/* Issue a disconnect */
		sendL2capDisconnectReq(StreamL2capSink(ind->cid));
	}
	else
	{
		/* TODO: B-4545 We should have a connection state entry something has gone wrong */
	}
}


/****************************************************************************
NAME	
	connectionHandleQosSetupCfm

DESCRIPTION
	Confirmation of QOS parameters.  There is an interoperability issue with
	some older baseband chipsets.  This chipset will set the latency to 14
	slots (8.75mS).  This is not appropriate for piconet/scatternet operation as
    this will starve links of bandwidth. Therefore if this condition is
	detected then the latency is set back to a preferred default value

RETURNS
	void
*/
void connectionHandleQosSetupCfm(const DM_HCI_QOS_SETUP_CFM_T* cfm)
{
#define	MINIMUM_LATENCY	((uint32)8750)
#define PREFERRED_LATENCY	((uint32)25000)
	
	if(cfm->latency < MINIMUM_LATENCY)
	{
		MAKE_PRIM_C(DM_HCI_QOS_SETUP_REQ);
		prim->bd_addr = cfm->bd_addr;
		prim->service_type = cfm->service_type;
		prim->token_rate = cfm->token_rate;
		prim->peak_bandwidth = cfm->peak_bandwidth;
		prim->latency = PREFERRED_LATENCY;
		prim->delay_variation = cfm->delay_variation;		
		VmSendDmPrim(prim);
	}
}

/****************************************************************************
NAME	
	connectionHandleL2capInterlockDisconnectRsp

DESCRIPTION
	The client has read the CL_L2CAP_DISCONNECT_IND message so it is
    now safe to inform the remote device.

RETURNS
	void
*/
void connectionHandleL2capInterlockDisconnectRsp(const CL_INTERNAL_L2CAP_INTERLOCK_DISCONNECT_RSP_T* msg)
{
	MAKE_PRIM_T(L2CA_DISCONNECT_RSP);
	prim->identifier = msg->id;
	prim->cid = msg->cid;
	VmSendL2capPrim(prim);
}
