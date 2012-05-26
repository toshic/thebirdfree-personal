/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_receive_data.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_receive_data.h"

#include <panic.h>


/****************************************************************************
NAME	
	hfpHandleReceivedData

DESCRIPTION
	Called when we get an indication from the firmware that there's more data 
	received and waiting in the RFCOMM buffer. Parse it.

RETURNS
	void
*/
void hfpHandleReceivedData(HFP *hfp, Source source)
{
    uint16 len;

    /* Cancel all more data messages as we're about to process the whole buffer */
    (void) MessageCancelAll(&hfp->task, MESSAGE_MORE_DATA);

    len = SourceSize(source);

    /* Only bother parsing if there is something to parse */
    while (len > 0)
    {
		/* Keep parsing while we have data in the buffer */
        if (!parseSource(source, &hfp->task))
            break;

		/* Check we have more data to parse */
        len = SourceSize(source);
    }
}
