/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_caps.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_caps_handler.h"
#include "avrcp_metadata_command_req.h"


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void AvrcpGetCapabilities(AVRCP *avrcp, avrcp_capability_id caps)
{
	uint8 caps_params[] = {0};
	avrcp_status_code status;

#ifdef AVRCP_DEBUG_LIB	
    if ((caps != avrcp_capability_company_id) && (caps != avrcp_capability_event_supported))
        AVRCP_DEBUG(("Invalid capability type requested 0x%x\n", caps));
#endif

	caps_params[0] = caps;

	status = avrcpMetadataStatusCommand(avrcp, AVRCP_GET_CAPS_PDU_ID, avrcp_get_caps, sizeof(caps_params), caps_params, 0, 0, 0);

	if (status != avrcp_success)
	{
		avrcpSendGetCapsCfm(avrcp, status, FALSE, 0, 1, 0, 0, 0, 0, 0, 0, 0);
	}
}
#endif


/*****************************************************************************/
void AvrcpGetCapabilitiesResponse(AVRCP *avrcp, avrcp_response_type response, avrcp_capability_id caps, uint16 size_caps_list, Source caps_list)
{
#ifdef AVRCP_DEBUG_LIB	
    if ((caps != avrcp_capability_company_id) && (caps != avrcp_capability_event_supported))
        AVRCP_DEBUG(("Invalid capability type requested 0x%x\n", caps));
#endif

	/* Only allow a response to be sent if the corresponding command arrived. */
	if (avrcp->block_received_data == avrcp_get_caps)
    {
		avrcpSendGetCapsResponse(avrcp, response, caps, size_caps_list, caps_list);
    }
	else
		PRINT(("AvrcpGetCapabilitiesResponse: CT not waiting for response\n"));
}

