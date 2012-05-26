/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
	hfp_network_operator.c        

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"

#include <panic.h>


/****************************************************************************
NAME	
	HfpGetNetworkOperator

DESCRIPTION
	Requests network operator information from the AG. An SLC for the supplied
	profile instance (hfp) must already be established before calling this
	function. The network operator information received from the AG will be
    sent to the	application using a HFP_NETWORK_OPERATOR_IND message.  The 
    application will receive a HFP_NETWORK_OPERATOR_CFM message to indicate 
    that the request has been completed.

MESSAGE RETURNED
	HFP_NETWORK_OPERATOR_CFM

RETURNS
	void
*/
void HfpGetNetworkOperator(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
	{
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
	}
#endif

	{
		/* Send an internal message */
		MessageSend(&hfp->task, HFP_INTERNAL_AT_COPS_REQ, 0);
    }
}
