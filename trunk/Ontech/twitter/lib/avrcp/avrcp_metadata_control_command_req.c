/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_metadata_transfer.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_common.h"
#include "avrcp_continuation_handler.h"
#include "avrcp_metadata_control_command_req.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_private.h"


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
avrcp_status_code avrcpMetadataControlCommand(AVRCP *avrcp, uint16 id, avrcpPending pending, uint16 size_params, Source params)
{
	if (!avrcpMetadataEnabled(avrcp))
	{
		/* Must have metadata supported at this end to be able to send this command.  
		   Maybe should look at remote supported features also. 
		*/
  
		return avrcp_unsupported;
	}

    if (!avrcp->dataFreeTask.sent_data && !avrcp->block_received_data && !avrcp->pending)
    {
        /* Create the GetCapabilities PDU here and use the vendordependent interface to send it. */
        uint16 length = 0;        
	
		uint16 metadata_packet_type = AVCTP0_PACKET_TYPE_SINGLE;

		/* 
			The identifier is used to hold metadata that should be sent as part of the
			AvrcpVendorDependent command. So if this isn't empty then another command
			must already be being sent, so return a busy confirmation,
		*/
		if (avrcp->identifier)
		{
			return avrcp_busy;
		}

		/* If there is too much data to be sent, the data must be fragmented. */
		if (size_params  > 501)
			metadata_packet_type = AVCTP0_PACKET_TYPE_START;

		/* 
			Create enough space to hold the Source data passed in, as well as the extra 
			fields required (in this case the number of attributes sent).
		*/
		avrcp->identifier = avrcpCreateMetadataTransferCmd(id, size_params+1, 0, 2, metadata_packet_type, &length);        
	
		if (avrcp->identifier)
		{
			/* Store the total amount of data stored in identifier. */
			avrcp->identifier[METADATA_HEADER_SIZE] = METADATA_HEADER_SIZE + 1;
			/* Store the number of attributes supplied. */
			avrcp->identifier[METADATA_HEADER_SIZE+1] = size_params / 2;	
	
			avrcp->pending = pending;

			if (size_params  > 501)
			{
				/* Store the continuation packets as this command will be fragmented. */
				avrcpStoreNextContinuationPacket(avrcp, params, size_params + 1, id, 2, 0, avrcpFindNextTransactionLabel(avrcp));
				/* Metadata transfer PDUs use the Vendordependent  API so construct the right message and send it from here */
				AvrcpVendorDependent(avrcp, subunit_panel, 0x00, AVRCP0_CTYPE_CONTROL, AVRCP_BT_COMPANY_ID, 501 + METADATA_HEADER_SIZE + 1, params);
			}
			else
				AvrcpVendorDependent(avrcp, subunit_panel, 0x00, AVRCP0_CTYPE_CONTROL, AVRCP_BT_COMPANY_ID, size_params + METADATA_HEADER_SIZE + 1, params);

			return avrcp_success;
		}
		else
		{
			/* Return message to client indicating failure. */
			return avrcp_no_resource;
		}	
    }
    else
	{
        /* Already have data pending tell the client we're busy */
        return avrcp_busy;
	}
}
#endif

