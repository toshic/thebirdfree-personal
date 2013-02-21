/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_battery.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_battery_handler.h"
#include "avrcp_common.h"
#include "avrcp_metadata_command_req.h"
#include "avrcp_send_response.h"


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void AvrcpInformBatteryStatusOfCt(AVRCP *avrcp, avrcp_battery_status battery_status)
{
	uint8 status_params[] = {0};
	avrcp_status_code status;

	status_params[0] = battery_status;

	status = avrcpMetadataStatusCommand(avrcp, AVRCP_INFORM_BATTERY_STATUS_PDU_ID, avrcp_battery_information, sizeof(status_params), status_params, 0, 0, 0);

	if (status != avrcp_success)
	{
		avrcpSendCommonMetadataCfm(avrcp, status, 0, AVRCP_INFORM_BATTERY_STATUS_CFM, 0, 0);
	}
}
#endif

/*****************************************************************************/
void AvrcpInformBatteryStatusOfCtResponse(AVRCP *avrcp, avrcp_response_type response)
{
	/* Only allow a response to be sent if the corresponding command arrived. */
	if (avrcp->block_received_data == avrcp_battery_information)
    {
		avrcpSendInformBatteryResponse(avrcp, response);       
    }
	else
		PRINT(("AvrcpInformBatteryStatusOfCtResponse: CT not waiting for response\n"));
}

