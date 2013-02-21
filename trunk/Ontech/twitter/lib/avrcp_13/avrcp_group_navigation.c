/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_group_navigation.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_common.h"
#include "avrcp_group_navigation_handler.h"
#include "avrcp_metadata_command_req.h"
#include "avrcp_private.h"

#include <stdlib.h>


#ifdef AVRCP_CT_SUPPORT
static void sendGroupRequest(AVRCP *avrcp, uint16 vendor_id)
{
	Source src;	
	const uint16 length = 5;
	uint8 *data = (uint8 *) malloc(length);

	/* Send a group request as part of the Passthrough command. */
	data[0] = (AVRCP_BT_COMPANY_ID >> 16) & 0xff;
    data[1] = (AVRCP_BT_COMPANY_ID >> 8) & 0xff;
    data[2] = AVRCP_BT_COMPANY_ID & 0xff;
	data[3] = 0x00;
	data[4] = vendor_id;

	src = avrcpSourceFromData(avrcp, data, length);

	if (vendor_id)
		avrcp->pending = avrcp_previous_group;
	else
		avrcp->pending = avrcp_next_group;

	{
		MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_PASSTHROUGH_REQ);
		message->subunit_type = subunit_panel;
		message->subunit_id = 0;
		message->state = 0;
		message->opid = opid_vendor_unique;
		message->operation_data = src;
		message->operation_data_length = length;
		MessageSend(&avrcp->task, AVRCP_INTERNAL_PASSTHROUGH_REQ, message);
	}
}


/*****************************************************************************/
void AvrcpNextGroup(AVRCP *avrcp)
{
	if (!avrcpMetadataEnabled(avrcp))
	{
		/* Must have metadata supported at this end to be able to send this command.  
		   Maybe should look at remote supported features also. 
		*/
		avrcpSendCommonMetadataCfm(avrcp, avrcp_unsupported, 0, AVRCP_NEXT_GROUP_CFM, 0, 0);
		return;
	}

	if (!avrcp->dataFreeTask.sent_data && !avrcp->block_received_data && !avrcp->pending)
		/* Send the data if none is already being sent. */
		sendGroupRequest(avrcp, 0);
    else
        /* Already sending data */
		avrcpSendCommonMetadataCfm(avrcp, avrcp_busy, 0, AVRCP_NEXT_GROUP_CFM, 0, 0);
}


/*****************************************************************************/
void AvrcpPreviousGroup(AVRCP *avrcp)
{
	if (!avrcpMetadataEnabled(avrcp))
	{
		/* Must have metadata supported at this end to be able to send this command.  
		   Maybe should look at remote supported features also. 
		*/
		avrcpSendCommonMetadataCfm(avrcp, avrcp_unsupported, 0, AVRCP_PREVIOUS_GROUP_CFM, 0, 0);
		return;
	}

	if (!avrcp->dataFreeTask.sent_data && !avrcp->block_received_data && !avrcp->pending)
		/* Send the data if none is already being sent. */
		sendGroupRequest(avrcp, 1);
    else
        /* Already sending data */
		avrcpSendCommonMetadataCfm(avrcp, avrcp_busy, 0, AVRCP_PREVIOUS_GROUP_CFM, 0, 0);
}
#endif

/*****************************************************************************/
void AvrcpNextGroupResponse(AVRCP *avrcp, avrcp_response_type response)
{
	/* Send the response only if the command arrived to start with. */
	if (avrcp->block_received_data == avrcp_next_group)
    {
		sendNextGroupResponse(avrcp, response);
	}
	else
		PRINT(("AvrcpNextGroupResponse: CT not waiting for response\n"));
}


/*****************************************************************************/
void AvrcpPreviousGroupResponse(AVRCP *avrcp, avrcp_response_type response)
{
	/* Send the response only if the command arrived to start with. */
	if (avrcp->block_received_data == avrcp_previous_group)
    {
		sendPreviousGroupResponse(avrcp, response);
	}
	else
		PRINT(("AvrcpPreviousGroupResponse: CT not waiting for response\n"));
}




