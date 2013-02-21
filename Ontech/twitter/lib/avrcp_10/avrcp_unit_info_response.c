/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
	avrcp_signal.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_signal_handler.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME	
	AvrcpUnitInfoResponse

DESCRIPTION
	This function is called in response to an AVRCP_UNITINFO_IND message
	requesting information about this device.
	
MESSAGE RETURNED

*/
void AvrcpUnitInfoResponse(AVRCP *avrcp, bool accept, avc_subunit_type unit_type, uint8 unit, uint32 company_id)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_UNITINFO_RES);
    
#ifdef AVRCP_DEBUG_LIB	
	if (unit_type > 0x1F)
	{
		AVRCP_DEBUG(("Out of range subunit type  0x%x\n", unit_type));
	}
    if (unit > 0x07)
	{
		AVRCP_DEBUG(("Out of range unit  0x%x\n", unit));
	}
    if (company_id > 0xFFFFFF)
	{
		AVRCP_DEBUG(("Out of range company id  0x%lx\n", company_id));
	}
#endif
    
	message->accept = accept;
	message->unit_type = unit_type;
	message->unit = unit;
	message->company_id = company_id;
	MessageSend(&avrcp->task, AVRCP_INTERNAL_UNITINFO_RES, message);
}


