/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_metadata_command_req.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_METADATA_COMMAND_REQ_H_
#define AVRCP_METADATA_COMMAND_REQ_H_


#include "avrcp_private.h"


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME
    avrcpMetadataStatusCommand

DESCRIPTION
    Prepares the Metadata command for sending to the remote device.
*/
avrcp_status_code avrcpMetadataStatusCommand(AVRCP *avrcp, uint16 id, avrcpPending pending, uint16 size_params, uint8 *params, uint16 size_extra_params, uint8 *extra_params, Source source_params);
#endif

/****************************************************************************
NAME
    avrcpSourceFromData

DESCRIPTION
    This function takes an allocated data block and returns a source. It 
    also stored the ptr to the data block so it can be frees when the
    source empties.
*/
Source avrcpSourceFromData(AVRCP *avrcp, uint8 *data, uint16 length);



#endif /* AVRCP_METADATA_COMMAND_REQ_H_ */
