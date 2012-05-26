/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_dtmf_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_dtmf_handler.h"
#include "hfp_send_data.h"

#include <stdio.h>
#include <string.h>


/* Send the cfm message to the application */
static void sendDtmfCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_DTMF_CFM, hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleDtmfRequest

DESCRIPTION
	HAndle a request to send a DTMF tone to the AG.

RETURNS
	void
*/
void hfpHandleDtmfRequest(HFP *hfp, const HFP_INTERNAL_AT_VTS_REQ_T *req)
{
	char dtmf[20];
	sprintf(dtmf, "AT+VTS=%c\r", req->dtmf);

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(dtmf), dtmf);
}


/****************************************************************************
NAME	
	hfpHandleDtmfRequestError

DESCRIPTION
	Send an error to the application as the profile instance is in the 
	wrong state.

RETURNS
	void
*/
void hfpHandleDtmfRequestError(HFP *hfp)
{
	/* Send error */
	sendDtmfCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleVtsAtAck

DESCRIPTION
	Have received an ack from the AG for the AT+VTS that we sent.

RETURNS
	void
*/
void hfpHandleVtsAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Pass the result code directly to the app */
	sendDtmfCfmToApp(hfp, status);
}
