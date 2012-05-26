/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
    sdp_search.c        

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
void ConnectionSdpOpenSearchRequest(Task appTask, const bdaddr* bd_addr)
{
#ifdef CONNECTION_DEBUG_LIB
    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr)); 
    }
#endif

    if ((appTask != connectionGetCmTask()) || !theCm.sdpState.sdpLock)
    {
	    /* Send an internal message */
	    MAKE_CL_MESSAGE(CL_INTERNAL_SDP_OPEN_SEARCH_REQ);
	    message->theAppTask = appTask;
	    message->bd_addr = *bd_addr;
	    MessageSendConditionallyOnTask(connectionGetCmTask(), CL_INTERNAL_SDP_OPEN_SEARCH_REQ, message, &theCm.sdpState.sdpLock);
    }
}


/*****************************************************************************/
void ConnectionSdpCloseSearchRequest(Task appTask)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_SDP_CLOSE_SEARCH_REQ);
	message->theAppTask = appTask;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_CLOSE_SEARCH_REQ, message);	
}


/*****************************************************************************/
void ConnectionSdpServiceSearchRequest(Task appTask, const bdaddr *bd_addr, uint16 max_num_recs, uint16 size_srch_pttrn, const uint8 *search_pattern)
{
#ifdef CONNECTION_DEBUG_LIB	
	if (size_srch_pttrn == 0)
		CL_DEBUG(("sdp - search pattern not supplied\n"));
	if (max_num_recs == 0)
		CL_DEBUG(("sdp - max number of records set to zero\n"));
    if(bd_addr == NULL)
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr)); 
#endif

	{
		/* Create an internal message and send it */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_SERVICE_SEARCH_REQ);
		message->theAppTask = appTask;
		message->bd_addr = *bd_addr;
		message->max_responses = max_num_recs;
		message->length = size_srch_pttrn;

		if (size_srch_pttrn)
		{
			message->search_pattern = (uint8 *)PanicUnlessMalloc(size_srch_pttrn);
			memmove(message->search_pattern, search_pattern, size_srch_pttrn);
		}
		else
			message->search_pattern = 0;
	
		MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_SERVICE_SEARCH_REQ, message);
	}
}





