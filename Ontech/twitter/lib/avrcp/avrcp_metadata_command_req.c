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
#include "avrcp_metadata_command_req.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_private.h"


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
avrcp_status_code avrcpMetadataStatusCommand(AVRCP *avrcp, uint16 id, avrcpPending pending, uint16 size_params, uint8 *params, uint16 size_extra_params, uint8 *extra_params, Source source_params)
{
	if (!avrcpMetadataEnabled(avrcp))
	{
		/* Must have metadata supported at this end to be able to send this command.  
		   Maybe should look at remote supported features also. 
		*/  
		return avrcp_unsupported;
	}

    if (!avrcp->dataFreeTask.sent_data)
    {
		if ((id == AVRCP_REQUEST_CONTINUING_RESPONSE_PDU_ID) || ((id != AVRCP_REQUEST_CONTINUING_RESPONSE_PDU_ID) && !avrcp->block_received_data && !avrcp->pending))
		{
			/* Create the GetCapabilities PDU here and use the vendordependent interface to send it. */
			uint16 length = 0;
			Source pdu_src;
			uint8 ctype;
			uint8 *cmd = 0;
			
			if (source_params)
			{
				if (avrcp->identifier)
				{
					return avrcp_busy;
				}

				avrcp->identifier = avrcpCreateMetadataTransferCmd(id, (size_extra_params+size_params-1), 0, size_extra_params, AVCTP0_PACKET_TYPE_SINGLE, &length);        

				cmd = avrcp->identifier;	
			}
			else
			{
				/* Create the Metadata Transfer PDU */
				uint8 *pdu = avrcpCreateMetadataTransferCmd(id, size_params, params, 0, AVCTP0_PACKET_TYPE_SINGLE, &length);        

				cmd = pdu;
			}

			if (cmd)
			{
				if (size_extra_params && extra_params)
				{
					uint16 i;
					for (i=0; i<size_extra_params;i++)
					{
						/* Add the specific data that will be sent (PDU ID). */
						cmd[METADATA_HEADER_SIZE+i] = extra_params[i];
					}
				}
				if (source_params)
				{
					length = size_params + METADATA_HEADER_SIZE + size_extra_params - 1;
					pdu_src = source_params;					
				}
				else
				{
					/* Create a source from the data block */
					pdu_src = avrcpSourceFromData(avrcp, cmd, length);
				}
			}
			else
			{
				return avrcp_no_resource;
			}
        
			avrcp->pending = pending;

			if (id == AVRCP_REGISTER_NOTIFICATION_PDU_ID)
				ctype = AVRCP0_CTYPE_NOTIFY;
			else
				ctype = AVRCP0_CTYPE_STATUS;

			/* Metadata transfer PDUs use the Vendordependent API so construct the right message and send it from here */
			AvrcpVendorDependent(avrcp, subunit_panel, 0x00, ctype, AVRCP_BT_COMPANY_ID, length, pdu_src);

			return avrcp_success;
		}
    }
    
    /* Already have data pending tell the client we're busy */
    return avrcp_busy;
}
#endif

/*****************************************************************************/
Source avrcpSourceFromData(AVRCP *avrcp, uint8 *data, uint16 length)
{
    /* Create a source from the data */
    Source src = StreamRegionSource(data, length);

    /* Register a task for freeing the data and store a ptr to it */
    avrcp->dataFreeTask.sent_data = data;
    MessageSinkTask(StreamSinkFromSource(src), &avrcp->dataFreeTask.cleanUpTask);

    return src;
}
