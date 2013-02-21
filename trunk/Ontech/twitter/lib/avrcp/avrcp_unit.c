/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release


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
#include "avrcp_signal_unit_info.h"
#include "avrcp_send_response.h"


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME	
	AvrcpUnitInfo

DESCRIPTION
	This function is called to request that a UnitInfo control command
	is sent to the target on the connection identified by the sink.
	
	The UnitInfo command is used to obtain information that pertains to the
	AV/C unit as a whole
	
	
					 -----------------------------------------------
					| MSB |		|	  |		|	  |		|	  |	LSB |
					|-----------------------------------------------|
	opcode			|		       UNITINFO (0x30) 					|
					 -----------------------------------------------
	operand[0]		|	  				0xFF						|
	 				 -----------------------------------------------
	operand[1]		|  				    0xFF						|
					 -----------------------------------------------
	operand[2]		|  				    0xFF						|
					 -----------------------------------------------
	operand[3]		|  				    0xFF						|
					 -----------------------------------------------
	operand[4]		|  				    0xFF						|
					 -----------------------------------------------

MESSAGE RETURNED
	AVRCP_UNITINFO_CFM - This message contains the unit_type and a
	unique 24-bit Company ID
*/

/*****************************************************************************/
void AvrcpUnitInfo(AVRCP *avrcp)
{
	if (avrcp->dataFreeTask.sent_data || avrcp->block_received_data || avrcp->pending)
		avrcpSendUnitInfoCfmToClient(avrcp, avrcp_busy, 0, 0, (uint32) 0);
	else if (!avrcp->sink)
		/* Immediately reject the request if we have not been passed a valid sink */
		avrcpSendUnitInfoCfmToClient(avrcp, avrcp_invalid_sink, 0, 0, (uint32) 0);
	else
	{
		MessageSend(&avrcp->task, AVRCP_INTERNAL_UNITINFO_REQ, 0);
	}
}
#endif

/*****************************************************************************/
void AvrcpUnitInfoResponse(AVRCP *avrcp, bool accept, avc_subunit_type unit_type, uint8 unit, uint32 company_id)
{
	sendUnitInfoResponse(avrcp, accept, unit_type, unit, company_id);
}



