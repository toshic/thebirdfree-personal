/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_character.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_character_handler.h"
#include "avrcp_common.h"
#include "avrcp_metadata_control_command_req.h"


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void AvrcpInformDisplayableCharacterSet(AVRCP *avrcp, uint16 size_attributes, Source attributes)
{
	avrcp_status_code status;

	status = avrcpMetadataControlCommand(avrcp, AVRCP_INFORM_CHARACTER_SET_PDU_ID, avrcp_character_set, size_attributes, attributes);

	if (status != avrcp_success)
	{
		avrcpSendCommonMetadataCfm(avrcp, status, 0, AVRCP_INFORM_CHARACTER_SET_CFM, 0, 0);
	}
}
#endif

void AvrcpInformDisplayableCharacterSetResponse(AVRCP *avrcp, avrcp_response_type response)
{
	/* Only allow a response to be sent if the corresponding command arrived. */
	if (avrcp->block_received_data == avrcp_character_set)
    {
		avrcpSendInformCharSetResponse(avrcp, response);     
    }
	else
		PRINT(("AvrcpInformDisplayableCharacterSetResponse: CT not waiting for response\n"));
}
