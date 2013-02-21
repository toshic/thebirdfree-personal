/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
    avrcp_metadata_control_command_req.h
    
DESCRIPTION
	
*/

#ifndef AVRCP_METADATA_CONTROL_COMMAND_REQ_H_
#define AVRCP_METADATA_CONTROL_COMMAND_REQ_H_


#include "avrcp_private.h"


#ifdef AVRCP_CT_SUPPORT
/****************************************************************************
NAME
    avrcpMetadataControlCommand

DESCRIPTION
    Prepares the Metadata command for sending to the remote device.
*/
avrcp_status_code avrcpMetadataControlCommand(AVRCP *avrcp, uint16 id, avrcpPending pending, uint16 size_params, Source params);
#endif

#endif /* AVRCP_METADATA_CONTROL_COMMAND_REQ_H_ */
