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
	AvrcpPassthrough

DESCRIPTION
	This function is called to request that a Pass Through control command
	is sent to the target on the the connection identified by the sink.
	
	The Passthrough command is used to convey the proper user operation to 
	the target(transparently to the user).
	
	
	identifier 				- Used to identify the matching AV/C Command 
    subunit_type
	subunit_id				- AV/C protocol - Used to form the targets address
	opid					- Identifies the button pressed
	state					- Indicates the user action of pressing or releasing
	   				  		  the button identified by opid.  Active low.
	operation_data			- Required for the Vendor Unique operation.  For other 
	operation_dataLlength 	  operations both fields should be zero
	
					 -----------------------------------------------
					| MSB |		|	  |		|	  |		|	  |	LSB |
					|-----------------------------------------------|
	opcode			|		      PASSTHROUGH (0x7C) 				|
					 -----------------------------------------------
	operand(0)		|state|			operation_id					|
	 				 -----------------------------------------------
	operand(1)		|  		operation data field length				|
					 -----------------------------------------------
	operand(2)		|	 operation data(operation id dependant)		|
		:			|												|
					 -----------------------------------------------

MESSAGE RETURNED
	AVRCP_PASSTHROUGH_CFM 
*/
/*lint -e818 -e830 */
void AvrcpPassthrough(AVRCP *avrcp, avc_subunit_type subunit_type, avc_subunit_id subunit_id, bool state, avc_operation_id opid, uint16 operation_data_length, Source operation_data)
{
    
#ifdef AVRCP_DEBUG_LIB	
	if (subunit_type > 0x1F)
	{
		AVRCP_DEBUG(("Out of range subunit type  0x%x\n", subunit_type));
	}
    if (subunit_id > 0x07)
	{
		AVRCP_DEBUG(("Out of range subunit id  0x%x\n", subunit_id));
	}
    if (opid > 0x7F)
	{
		AVRCP_DEBUG(("Out of range op id  0x%x\n", opid));
	}
#endif
    
	if (!avrcp->sink)
		/* Immediately reject the request if we have not been passed a valid sink */
		avrcpSendPassthroughCfmToClient(avrcp, avrcp_invalid_sink);
	else
	{
		MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_PASSTHROUGH_REQ);
		message->subunit_type = subunit_type;
		message->subunit_id = subunit_id;
		message->state = state;
		message->opid = opid;

		if (opid == opid_vendor_unique)
		{
			message->operation_data = operation_data;
			message->operation_data_length = operation_data_length;
		}
		else
		{
            if (SourceSize(operation_data))
                SourceEmpty(operation_data);
			message->operation_data = 0;
			message->operation_data_length = 0;
		}

		MessageSend(&avrcp->task, AVRCP_INTERNAL_PASSTHROUGH_REQ, message);
	}
}
/*lint +e818 +e830 */

