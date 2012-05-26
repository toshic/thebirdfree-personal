/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    avrcp_init.c        

DESCRIPTION
    This file contains the initialisation code for the AVRCP profile library.

NOTES

*/
/****************************************************************************
    Header files
*/
#include "avrcp_init.h"
#include "avrcp_profile_handler.h"

/*****************************************************************************/
const profile_task_recipe avrcp_recipe = {
    sizeof(AVRCP),
    AVRCP_INTERNAL_TASK_INIT_REQ,
    {avrcpProfileHandler},
    1,
    (const profile_task_recipe *) 0
};


/*****************************************************************************/
static void avrcpInit(Task                  theAppTask, 
                    const avrcp_init_params *config, 
                    uint16                  lazy, 
                    Task                    connectionTask)
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
            avrcpInitTaskData(avrcp, theAppTask, avrcpInitialising, 
                              config->device_type, 
                              config->supported_controller_features, 
                              config->supported_target_features, 
                              config->profile_extensions, lazy);
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
void avrcpInitTaskData(AVRCP             *avrcp, 
                       Task              client, 
                       avrcpState        state, 
                       avrcp_device_type dev,
                       uint8             controller_features, 
                       uint8             target_features, 
                       uint8             extensions, 
                       uint16            lazy)
{
    /* Set the handler function */
    avrcp->task.handler = avrcpProfileHandler;

    /* Store the app task so we know where to return responses */
    avrcp->clientTask = client;    

    /* Init the remaining profile state */
    avrcpSetState(avrcp, state);
    avrcp->pending = avrcp_none;
    avrcp->block_received_data = 0;
    avrcp->srcUsed = 0;
    avrcp->sink = 0;
    avrcp->cmd_transaction_label = 0;
    avrcp->l2cap_mtu = 672; /* default l2cap mtu */
    avrcp->identifier = 0;
    avrcp->device_type = dev;
    avrcp->lazy = lazy;
    avrcp->registered_events = 0;
    avrcp->continuation_pdu = 0;
    avrcp->sdp_search_mode = avrcp_sdp_search_none;
    avrcp->av_msg = NULL;    
    avrcp->av_msg_len = 0;

    /* Mask all bits except bit 6 to disable Browsing */ 
    /* Only first 6 bits are currently valid */
    avrcp->local_target_features = (target_features & 0xBF);

     /* Only category device bits for controller */
    avrcp->local_controller_features = (controller_features & 0xf);

    /* If no target features supplied then enable category 2 as default. */
    /* Make sure bit 0 is set if metadata bits are also set. */
    if (target_features & (AVRCP_PLAYER_APPLICATION_SETTINGS | 
                           AVRCP_GROUP_NAVIGATION))
        avrcp->local_target_features |= 0x1;

    avrcp->local_extensions = extensions;

    (avrcp->dataFreeTask.cleanUpTask).handler = avrcpDataCleanUp;
    avrcp->dataFreeTask.sent_data = 0;
}


/*****************************************************************************/
void AvrcpInit(Task theAppTask, const avrcp_init_params *config)
{
    avrcpInit(theAppTask, config, 0, theAppTask);
}


/*****************************************************************************/
void AvrcpInitLazy(Task                     clientTask, 
                   Task                     connectionTask, 
                   const avrcp_init_params *config)
{
    avrcpInit(clientTask, config, 1, connectionTask);
}


/*****************************************************************************/
void avrcpHandleInternalInitReq(AVRCP *avrcp, 
                                const AVRCP_INTERNAL_INIT_REQ_T *req)
{
    /* 
        If recipe is null connection lib runs in non-lazy mode so one 
        function call can handle both modes 
    */
    if (avrcp->lazy)
        ConnectionL2capRegisterRequestLazy(&avrcp->task, 
                                           req->connectionTask, 
                                           AVCTP_PSM, 
                                           req->recipe);
    else
        ConnectionL2capRegisterRequest(&avrcp->task, AVCTP_PSM);
}


/*****************************************************************************/
void avrcpSendInitCfmToClient(Task               clientTask,
                              AVRCP              *avrcp, 
                              avrcp_status_code  status)
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
