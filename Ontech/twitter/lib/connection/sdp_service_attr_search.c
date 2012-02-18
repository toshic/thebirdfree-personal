
/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Audio-Adaptor-SDK 2009.R1

FILE NAME
    sdp_service_attr_search.c        

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
void ConnectionSdpServiceSearchAttributeRequest(Task appTask, const bdaddr *addr, uint16 max_attributes, uint16 size_search_pattern, const uint8 *search_pattern, uint16 size_attr_list, const uint8 *attr_list)
{
#ifdef CONNECTION_DEBUG_LIB	
	if (size_search_pattern == 0)
		CL_DEBUG(("sdp - search pattern not supplied\n"));
	if (size_attr_list == 0)
		CL_DEBUG(("sdp - attribute search pattern not supplied\n"));
	if (max_attributes == 0)
		CL_DEBUG(("sdp - max number of attribute bytes set to zero\n"));
#endif

	{
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ);
		message->theAppTask = appTask;
		message->bd_addr = *addr;
		message->max_num_attributes = max_attributes;
		message->size_search_pattern = size_search_pattern;
	
		if (size_search_pattern)
		{
			message->search_pattern = (uint8 *)PanicUnlessMalloc(size_search_pattern);
			memcpy(message->search_pattern, search_pattern, size_search_pattern);
		}
		else
			message->search_pattern = 0;

		message->size_attribute_list = size_attr_list;

		if (size_attr_list)
		{
			message->attribute_list = (uint8 *) PanicUnlessMalloc(size_attr_list);
			memcpy(message->attribute_list, attr_list, size_attr_list);
		}
		else
			message->attribute_list = 0;

		MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ, message);
	}
}

