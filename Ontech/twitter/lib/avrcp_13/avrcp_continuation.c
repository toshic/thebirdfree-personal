/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_continuation.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_common.h"
#include "avrcp_continuation_handler.h"
#include "avrcp_metadata_command_req.h"


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void AvrcpRequestContinuing(AVRCP *avrcp, uint16 pdu_id)
{
	uint8 params[] = {0};
	avrcp_status_code status;

	params[0] = pdu_id & 0xFF;

	status = avrcpMetadataStatusCommand(avrcp, AVRCP_REQUEST_CONTINUING_RESPONSE_PDU_ID, avrcp_request_continuation, 1, params, 0, 0, 0);

	if (status != avrcp_success)
	{
		MAKE_AVRCP_MESSAGE(AVRCP_REQUEST_CONTINUING_RESPONSE_CFM);

		message->avrcp = avrcp;
		message->transaction = 0;
		message->status = status;
		message->pdu_id = params[0];

		MessageSend(avrcp->clientTask, AVRCP_REQUEST_CONTINUING_RESPONSE_CFM, message);
	}
}


/*****************************************************************************/
void AvrcpAbortContinuing(AVRCP *avrcp, uint16 pdu_id)
{
	uint8 params[] = {0};
	avrcp_status_code status;

	params[0] = pdu_id & 0xFF;

	status = avrcpMetadataStatusCommand(avrcp, AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID, avrcp_abort_continuation, 1, params, 0, 0, 0);

	if (status != avrcp_success)
	{
		avrcpSendCommonMetadataCfm(avrcp, status, 0, AVRCP_ABORT_CONTINUING_RESPONSE_CFM, 0, 0);
	}
}
#endif
