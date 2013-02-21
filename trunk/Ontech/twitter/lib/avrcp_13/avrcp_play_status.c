/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_element_attributes.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_metadata_command_req.h"
#include "avrcp_play_status_handler.h"
#include "avrcp_private.h"


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void AvrcpGetPlayStatus(AVRCP *avrcp)
{
	avrcp_status_code status = avrcpMetadataStatusCommand(avrcp, AVRCP_GET_PLAY_STATUS_PDU_ID, avrcp_get_play_status, 0, 0, 0, 0, 0);

	if (status != avrcp_success)
	{
		avrcpSendGetPlayStatusCfm(avrcp, status, 0, 0, 0, 0, 0, 0);
	}
}
#endif

/*****************************************************************************/
void AvrcpGetPlayStatusResponse(AVRCP *avrcp, avrcp_response_type response, uint32 song_length, uint32 song_elapsed, avrcp_play_status play_status)
{
	/* Only send a response if the command was received. */
	if (avrcp->block_received_data == avrcp_get_play_status)
    {
		sendPlayStatusResponse(avrcp, response, song_length, song_elapsed, play_status);
	}
	else
		PRINT(("AvrcpGetPlayStatusResponse: CT not waiting for response\n"));
}


