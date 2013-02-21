/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2004-2009
Part of BlueLab 4.1.2-Release

FILE NAME
	avrcp_sdp.c        

DESCRIPTION
	This file contains the initialisation code for the avrcp profile library.

NOTES

*/
/*lint -e655 */


/****************************************************************************
	Header files
*/
#include "avrcp.h"
#include "avrcp_private.h"
#include "avrcp_sdp_handler.h"


#ifdef AVRCP_CT_SUPPORT
/*****************************************************************************/
void AvrcpGetSupportedFeatures(AVRCP *avrcp)
{
	bdaddr my_addr;

	if (SinkGetBdAddr(avrcp->sink, &my_addr))
	{
		MessageSendConditionally(&avrcp->task, AVRCP_INTERNAL_GET_FEATURES, 0, &avrcp->sdp_search_mode);
	}
	else
	{
		avrcpSendGetSupportedFeaturesCfm(avrcp, avrcp_device_not_connected, 0);
	}
}


/*****************************************************************************/
void AvrcpGetProfileExtensions(AVRCP *avrcp)
{
	bdaddr my_addr;

	if (SinkGetBdAddr(avrcp->sink, &my_addr))
	{
		MessageSendConditionally(&avrcp->task, AVRCP_INTERNAL_GET_EXTENSIONS, 0, &avrcp->sdp_search_mode);
	}
	else
	{
		avrcpSendGetExtensionsCfm(avrcp, avrcp_device_not_connected, 0);	
	}
}
#endif
