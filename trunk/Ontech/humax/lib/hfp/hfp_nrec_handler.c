/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_nrec_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_send_data.h"
#include "hfp_nrec_handler.h"

#include <panic.h>
#include <string.h>


/* Send a message to the app telling it the outcome of the NREC disable request */
static void sendNrecCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_DISABLE_NREC_CFM, hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleNrEcDisable

DESCRIPTION
	Send a request to the AG to disable its Noise Reduction (NR) and Echo
	Cancellation (EC) capabilities.

RETURNS
	void
*/
void hfpHandleNrEcDisable(HFP *hfp)
{
	/* Only send the AT cmd if the AG and HFP support this functionality */
	if ((hfp->agSupportedFeatures & AG_NREC_FUNCTION) &&
		(hfp->hfpSupportedFeatures & HFP_NREC_FUNCTION))
	{
		static const char nrec[] = "AT+NREC=0\r";

		/* Send the AT cmd over the air */
		hfpSendAtCmd(&hfp->task, strlen(nrec), nrec);
	}
	else
		/* Send an error */
		sendNrecCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleNrEcDisableError

DESCRIPTION
	Send an error message to the application, the profile instance is in
	the wrong state.

RETURNS
	void
*/
void hfpHandleNrEcDisableError(HFP *hfp)
{
	/* Send an error straight away, we're in the wrong state */
	sendNrecCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleNrecAtAck

DESCRIPTION
	Received an ack from the AG for the AT+NREC cmd.

RETURNS
	void
*/
void hfpHandleNrecAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Pass the result to the app */
	sendNrecCfmToApp(hfp, status);
}
