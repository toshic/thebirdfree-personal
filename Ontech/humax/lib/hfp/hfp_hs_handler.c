/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_hs_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_hs_handler.h"
#include "hfp_send_data.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME	
	hfpHandleHsButtonPress

DESCRIPTION
	Send a button press AT cmd to the AG.

RETURNS
	void
*/
void hfpHandleHsButtonPress(HFP *hfp)
{
	static const char ckpd[] = "AT+CKPD=200\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(ckpd), ckpd);
}


/****************************************************************************
NAME	
	hfpHandleHsButtonPressError

DESCRIPTION
	The HFP library received a request to send a button press to the AG,
	but this profile instance is in the wrong state, so send back an error
	message to the app instead.

RETURNS
	void
*/
void hfpHandleHsButtonPressError(HFP *hfp)
{
	/* Send an error message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_HS_BUTTON_PRESS_CFM, hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleHsCkpdAtAck

DESCRIPTION
	Send a button press cfm to the app telling it whether the AT+CKPD
	succeeded or not.

RETURNS
	void
*/
void hfpHandleHsCkpdAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_HS_BUTTON_PRESS_CFM, hfp, status);
}
