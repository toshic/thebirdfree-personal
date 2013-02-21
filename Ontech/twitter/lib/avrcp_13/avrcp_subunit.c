/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release


FILE NAME
	avrcp_subunit.c      

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_signal_unit_info.h"
#include "avrcp_send_response.h"


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
/*lint -e818 -e830 */
void AvrcpSubUnitInfo(AVRCP *avrcp, uint8 page)
{
    
#ifdef AVRCP_DEBUG_LIB	
	if (page > 0x07)
	{
		AVRCP_DEBUG(("Out of range page  0x%x\n", page));
	}
#endif
    
	if (avrcp->dataFreeTask.sent_data || avrcp->block_received_data || avrcp->pending)
		avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_busy, 0, 0);
	else if (!avrcp->sink)
		/* Immediately reject the request if we have not been passed a valid sink */
		avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_invalid_sink, 0, 0);
	else
	{
		MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SUBUNITINFO_REQ);
		message->page = page;
		MessageSend(&avrcp->task, AVRCP_INTERNAL_SUBUNITINFO_REQ, message);
	}	
}
/*lint +e818 +e830 */
#endif

/****************************************************************************
NAME	
	AvrcpSubUnitInfoResponse

DESCRIPTION	
	The SubUnitInfo command is used to obtain information about the subunit(s)
	of a device.
	
	accept					- Flag accepting or rejecting request for SubUnitInfo
	page_data				- Four entries from the subunit table for the requested
	   						  page on the target device
			
MESSAGE RETURNED

*/
/*****************************************************************************/
void AvrcpSubUnitInfoResponse(AVRCP *avrcp, bool accept, const uint8 *page_data)
{
	sendSubunitInfoResponse(avrcp, accept, page_data);
}



