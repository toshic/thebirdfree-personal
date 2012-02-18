/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	init.c        

DESCRIPTION
	This file contains the initialisation code for the avrcp profile library.

NOTES

*/
/*lint -e655 */


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_common.h"
#include "init.h"
#include "profile_handler.h"

#include <panic.h>


/*****************************************************************************/
const profile_task_recipe avrcp_recipe = {
    sizeof(AVRCP),
    AVRCP_INTERNAL_TASK_INIT_REQ,
    {avrcpProfileHandler},
    1,
    0
};


/*****************************************************************************/
static void avrcpInit(Task theAppTask, const avrcp_init_params *config, uint16 lazy, Task connectionTask)
{
    if (!config)
    {
        /* Must pass in valid configuration parameters. */
        avrcpSendInitCfmToClient(theAppTask, 0, avrcp_fail);
    }
    else
    {
	    AVRCP *avrcp = PanicUnlessNew(AVRCP);

        /* Store the device type to configure this instance to */
        if ((config->device_type == avrcp_target) ||
            (config->device_type == avrcp_controller) ||
            (config->device_type == avrcp_target_and_controller))    
            avrcpInitTaskData(avrcp, theAppTask, avrcpInitialising, config->device_type, lazy);
        else
        {
            avrcpSendInitCfmToClient(theAppTask, 0, avrcp_unsupported);
            return;
        }

    	/* Send an internal init message to kick off initialisation */
        {
            MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_INIT_REQ);
            message->connectionTask = connectionTask;

            if (lazy)
                message->recipe = &avrcp_recipe;
            else
                message->recipe = 0;

	        MessageSend(&avrcp->task, AVRCP_INTERNAL_INIT_REQ, message);
        }
    }
}


/*****************************************************************************/
void avrcpInitTaskData(AVRCP *avrcp, Task client, avrcpState state, avrcp_device_type dev, uint16 lazy)
{
    /* Set the handler function */
	avrcp->task.handler = avrcpProfileHandler;

    /* Store the app task so we know where to return responses */
	avrcp->clientTask = client;    

    /* Init the remaining profile state */
    avrcpSetState(avrcp, state);
    avrcp->pending = avrcp_none;
    avrcp->block_received_data = 0;
    avrcp->watchdog = 0;
    avrcp->sink = 0;
	avrcp->transaction_label = 0;
    avrcp->fragmented = avrcp_frag_none;
	avrcp->l2cap_mtu = 672; /* default l2cap mtu */
    avrcp->identifier = 0;
    avrcp->device_type = dev;
    avrcp->lazy = lazy;
}

/*****************************************************************************/
void AvrcpInit(Task theAppTask, const avrcp_init_params *config)
{
    avrcpInit(theAppTask, config, 0, theAppTask);
}


/*****************************************************************************/
void AvrcpInitLazy(Task clientTask, Task connectionTask, const avrcp_init_params *config)
{
    avrcpInit(clientTask, config, 1, connectionTask);
}


/*****************************************************************************/
void avrcpHandleInternalInitReq(AVRCP *avrcp, const AVRCP_INTERNAL_INIT_REQ_T *req)
{
    /* 
        If recipe is null connection lib runs in non-lazy mode so one 
        function call can handle both modes 
    */
    if (avrcp->lazy)
        ConnectionL2capRegisterRequestLazy(&avrcp->task, req->connectionTask, AVCTP_PSM, req->recipe);
    else
        ConnectionL2capRegisterRequest(&avrcp->task, AVCTP_PSM);
}


/*****************************************************************************/
void avrcpSendInitCfmToClient(Task clientTask, AVRCP *avrcp, avrcp_status_code status)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INIT_CFM);
	message->status = status;

    if ((status == avrcp_success) && !avrcp->lazy)
    {
        message->avrcp = avrcp;
        avrcpSetState(avrcp, avrcpReady);
    }
    else
    {
        message->avrcp = 0;
        free(avrcp);
    }

	MessageSend(clientTask, AVRCP_INIT_CFM, message);
}
