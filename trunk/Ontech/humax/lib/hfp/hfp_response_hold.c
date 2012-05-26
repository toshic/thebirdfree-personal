/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
	hfp_response_hold.c        

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
	HfpGetResponseHoldStatus

DESCRIPTION
	Requests response hold status from the AG. An SLC for the supplied
	profile instance (hfp) must already be established before calling this
	function. If the AG is in a response hold state it will return a
    HFP_RESPONSE_HOLD_STATUS_IND message, otherwise no HFP_RESPONSE_HOLD_STATUS_IND 
    message will be sent.  When the application receives a HFP_RESPONSE_HOLD_STATUS_CFM 
    message it will indicate that the request has been completed.

MESSAGE RETURNED
	HFP_RESPONSE_HOLD_STATUS_CFM

RETURNS
	void
*/
void HfpGetResponseHoldStatus(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif

    /* Send an internal message */
    MessageSend(&hfp->task, HFP_INTERNAL_AT_BTRH_STATUS_REQ, 0);
}


/****************************************************************************
NAME	
	HfpHoldIncomingCall

DESCRIPTION
    Requests that the AG places the current incoming call on hold.
    
MESSAGE RETURNED
	HFP_RESPONSE_HOLD_HELD_CFM

RETURNS
	void
*/
void HfpHoldIncomingCall(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
	if (!hfp)
    {
		HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif

    /* Send an internal message */
    MessageSend(&hfp->task, HFP_INTERNAL_AT_BTRH_HOLD_REQ, 0);
}


/****************************************************************************
NAME	
	HfpAcceptHeldIncomingCall

DESCRIPTION
    Request that Ag accepts the incoming call that is currently on hold.
    
MESSAGE RETURNED
	HFP_RESPONSE_HOLD_ACCEPT_CFM

RETURNS
	void
*/
void HfpAcceptHeldIncomingCall(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif

    /* Send an internal message */
    MessageSend(&hfp->task, HFP_INTERNAL_AT_BTRH_ACCEPT_REQ, 0);
}


/****************************************************************************
NAME	
	HfpRejectHeldIncomingCall

DESCRIPTION
    Requests that AG rejects the incoming call that is currently on hold.
    
MESSAGE RETURNED
	HFP_RESPONSE_HOLD_REJECT_CFM

RETURNS
	void
*/
void HfpRejectHeldIncomingCall(HFP *hfp)
{
#ifdef HFP_DEBUG_LIB
    if (!hfp)
    {
        HFP_DEBUG(("Null hfp task ptr passed in.\n"));
    }
#endif

    /* Send an internal message */
    MessageSend(&hfp->task, HFP_INTERNAL_AT_BTRH_REJECT_REQ, 0);
}


