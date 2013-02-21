/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_character_handler.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "avrcp_character_handler.h"
#include "avrcp_common.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_private.h"
#include "avrcp_send_response.h"

#include <source.h>


/*****************************************************************************/
bool avrcpHandleInformCharSetCommand(AVRCP *avrcp, uint16 ctp_packet_type, uint16 meta_packet_type, Source source, uint16 packet_size)
{
	const uint8 *ptr = SourceMap(source);

	uint16 total_packets;
	uint16 data_offset;
	uint8 data[1];


	if (!avrcpMetadataAppSettingsEnabled(avrcp))
	{
		avrcpSendInformCharSetResponse(avrcp, avctp_response_not_implemented);
		
		avrcpSourceProcessed(avrcp);
	}
	else
	{

		if (!getMessageDataFromPacket(ctp_packet_type, meta_packet_type, 1, ptr, &total_packets, &data_offset, &data[0]))
			return FALSE;
	
		avrcpSendCommonFragmentedMetadataInd(avrcp, AVRCP_INFORM_CHARACTER_SET_IND, (ptr[0] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F, total_packets, ctp_packet_type, meta_packet_type, data[0], packet_size - data_offset, data_offset, source);
	}

	return TRUE;
}

