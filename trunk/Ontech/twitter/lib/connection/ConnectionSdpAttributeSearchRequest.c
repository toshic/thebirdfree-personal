/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    ConnectionSdpAttributeSearchRequest.c        

DESCRIPTION
		

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"

#include <panic.h>
#include <string.h>


/*****************************************************************************/
void ConnectionSdpAttributeSearchRequest(Task appTask, const bdaddr *bd_addr, uint16 max_num_recs, uint32 service_hdl, uint16 size_attribute_list, const uint8 *attribute_list)
{
#ifdef CONNECTION_DEBUG_LIB	
	if (size_attribute_list == 0)
		CL_DEBUG(("sdp - attribute search pattern not supplied\n"));
	if (max_num_recs == 0)
		CL_DEBUG(("sdp - max number of attribute bytes set to zero\n"));
    if(bd_addr == NULL)
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr)); 
#endif

	{
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ);
		message->theAppTask = appTask;
		message->bd_addr = *bd_addr;
		message->service_handle = service_hdl;
		message->size_attribute_list = size_attribute_list;
	
		if (size_attribute_list)
		{
			message->attribute_list = (uint8 *)PanicUnlessMalloc(size_attribute_list);
			memcpy(message->attribute_list, attribute_list, size_attribute_list);
		}
		else
			message->attribute_list = 0;
	
		message->max_num_attr = max_num_recs;
		MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ, message);
	}
}
