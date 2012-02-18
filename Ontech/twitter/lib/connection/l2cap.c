/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    l2cap.c        

DESCRIPTION
	File containing the l2cap API function implementations.	

NOTES

*/


/****************************************************************************
	Header files
*/
#include    "connection.h"
#include    "connection_private.h"


/******************************************************************************/
static void initConfigToDefaults(l2cap_config_params *config_params)
{
	config_params->mtu_local = 895;
    config_params->mtu_remote_min = MINIMUM_MTU;
    config_params->flush_timeout = DEFAULT_FLUSH_TO;
    config_params->accept_non_default_flush = 1;
	config_params->qos.service_type = DEFAULT_SERVICE_TYPE;
	config_params->qos.token_rate = DEFAULT_TOKEN_RATE;
	config_params->qos.token_bucket = DEFAULT_TOKEN_BUCKET;
	config_params->qos.peak_bw = DEFAULT_PEAK_BW;
	config_params->qos.latency = DEFAULT_LATENCY;
	config_params->qos.delay_var = DEFAULT_DELAY_VAR;
    config_params->accept_qos_settings = 1;
	config_params->timeout = DEFAULT_L2CAP_CONNECTION_TIMEOUT;
}


/*****************************************************************************/
static void sendInternalL2capRegister(Task client, Task connection, uint16 psm, const profile_task_recipe *recipe)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_REGISTER_REQ);
    message->clientTask = client;
	message->app_psm = psm;
    message->connectionTask = connection;
    message->l2cap_recipe = recipe;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_REGISTER_REQ, message);
}


/*****************************************************************************/
void ConnectionL2capRegisterRequest(Task clientTask, uint16 psm)
{
	/* Send an internal message */
    sendInternalL2capRegister(clientTask, clientTask, psm, 0);
}


/*****************************************************************************/
void ConnectionL2capRegisterRequestLazy(Task clientTask, Task connectionTask, uint16 psm, const profile_task_recipe *recipe)
{
    sendInternalL2capRegister(clientTask, connectionTask, psm, recipe);
}


/*****************************************************************************/
void ConnectionL2capConnectRequest(Task appTask, const bdaddr *addr, uint16 psm_local, uint16 psm_remote, const l2cap_config_params *config)
{
	/* Check a non null address ptr has been passed in */
#ifdef CONNECTION_DEBUG_LIB
	if (!addr)
		CL_DEBUG(("Null address ptr passed in.\n"));	
#endif

	{
		/* Send an internal message */
		MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_CONNECT_REQ);
		message->theAppTask = appTask;
		message->bd_addr = *addr;
		message->psm_local = psm_local;
		message->psm_remote = psm_remote;

		/* If client task has not supplied config params, initiate to defaults */
		if (!config)
			initConfigToDefaults(&message->config_params);
		else
			message->config_params = *config;

		MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_CONNECT_REQ, message);
	}
}


/*****************************************************************************/
void ConnectionL2capConnectResponse(Task appTask, bool response, uint16 psm, uint16 connection_id, const l2cap_config_params *config)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_CONNECT_RES);
	message->theAppTask = appTask;
	message->response = response;
	message->psm_local = psm;
	message->cid = connection_id;

	/* If client task has not supplied config params, initiate to defaults */
	if (!config)
		initConfigToDefaults(&message->config_params);
	else
		message->config_params = *config;

	MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_CONNECT_RES, message);
}


/*****************************************************************************/
void ConnectionL2capDisconnectRequest(Task appTask, Sink sink)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_DISCONNECT_REQ);
	message->theAppTask = appTask;	
	message->sink = sink;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_DISCONNECT_REQ, message);
}
