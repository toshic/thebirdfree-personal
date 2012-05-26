/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

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


static void sendInitCfmToClient(AVRCP *avrcp, avrcp_status_code status)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INIT_CFM);
	message->status = status;

	if (status == avrcp_success)
        message->avrcp = avrcp;
    else
        message->avrcp = 0;

	MessageSend(avrcp->clientTask, AVRCP_INIT_CFM, message);

    /* If the initialisation failed, free the allocated task */
	if (status != avrcp_success)
		free(avrcp);
}


/*****************************************************************************/
void AvrcpInit(Task theAppTask, const avrcp_init_params *config)
{
	AVRCP *avrcp = PanicUnlessNew(AVRCP);
	
	/* Set the state to initialising */
	avrcpSetState(avrcp, avrcpInitialising);

	/* Set the handler function */
	avrcp->task.handler = avrcpProfileHandler;

	/* Init the remaining profile state */
	avrcp->sink = 0;
	avrcp->transaction_label = 0;
	avrcp->watchdog = 0;
	avrcp->pending = avrcp_none;
    avrcp->block_received_data = 0;
	avrcp->fragmented = avrcp_frag_none;
	avrcp->l2cap_mtu = 672; /* default l2cap mtu */
	
	/* Store the app task so we know where to return responses */
	avrcp->clientTask = theAppTask;

    /* Store the device type to configure this instance to */
    if ((config->device_type == avrcp_target) ||
        (config->device_type == avrcp_controller) ||
        (config->device_type == avrcp_target_and_controller))
    {
        avrcp->device_type = config->device_type;
    }
    else
    {
        sendInitCfmToClient(avrcp, avrcp_unsupported);
        return;
    }

	/* Reset the identifier field */
	avrcp->identifier = 0;

	/* Send an internal init message to kick off initialisation */
	MessageSend(&avrcp->task, AVRCP_INTERNAL_INIT_REQ, 0);
}


/****************************************************************************
NAME	
	avrcpHandleInternalInitReq

DESCRIPTION
	Init request messages are sent internally withing the profile instance
	until its initialisation has completed. These messages are handled in
	this function.
*/
void avrcpHandleInternalInitReq(AVRCP *avrcp)
{
	/* Register AVCTP PSM with the Connection library */
	ConnectionL2capRegisterRequest(&avrcp->task, AVCTP_PSM);
}


/****************************************************************************
NAME	
	avrcpSendInternalInitCfm

DESCRIPTION
	Send an internal init cfm message.
*/
void avrcpSendInternalInitCfm(Task task, avrcp_status_code status)
{
	MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_INIT_CFM);
	message->status = status;
	MessageSend(task, AVRCP_INTERNAL_INIT_CFM, message);
}


/****************************************************************************
NAME	
	avrcpHandleInternalInitCfm

DESCRIPTION
	This message is sent once various parts of the library initialisation 
	process have completed.
*/
void avrcpHandleInternalInitCfm(AVRCP *avrcp, const AVRCP_INTERNAL_INIT_CFM_T *cfm)
{
    sendInitCfmToClient(avrcp, cfm->status);

	/* Set the state to Ready */
	avrcpSetState(avrcp, avrcpReady);
}
