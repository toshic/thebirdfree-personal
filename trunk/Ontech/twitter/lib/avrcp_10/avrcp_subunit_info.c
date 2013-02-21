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
	AvrcpSubUnitInfo

DESCRIPTION
	This function is called to request that a SubUnitInfo control command
	is sent to the target on the connection identified by the sink.
	
	The UnitInfo command is used to obtain information about the subunit(s)
	of a device. The extension code is not used at present, should always be 0x7
	
	page					- Specifies which part of the subunit table is
							  to be returned.  Each page consists of at most
							  four subunits, and each AV/C unit contains upto
							  32 AV/C subunits
	
					 -----------------------------------------------
					| MSB |		|	  |		|	  |		|	  |	LSB |
					|-----------------------------------------------|
	opcode			|		      SUBUNITINFO (0x31) 				|
					 -----------------------------------------------
	operand[0]		|  0  |       Page      |  0  | Extension code	|
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
	AVRCP_SUBUNITINFO_CFM
*/
/*lint -e818 -e830 */
void AvrcpSubUnitInfo(AVRCP *avrcp, uint8 page)
{
    
#ifdef AVRCP_DEBUG_LIB	
	if (page > 0x07)
	{
		AVRCP_DEBUG(("Out of range page  0x%x\n", page));
	}
#endif
    
	if (!avrcp->sink)
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


