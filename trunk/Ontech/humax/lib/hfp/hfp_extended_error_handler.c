/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
	hfp_extended_error_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_extended_error_handler.h"
#include "hfp_common.h"
#include "hfp_send_data.h"

#include <panic.h>
#include <string.h>



static void sendExtendedErrorCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_EXTENDED_ERROR_CFM, hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleExtendedErrorReq

DESCRIPTION
	Request that extended error reporting is enabled on the AG.

RETURNS
	void
*/
void hfpHandleExtendedErrorReq(HFP *hfp)
{
	/* Only send the cmd if the AG supports the extended error result codes feature.*/
	if (hfp->agSupportedFeatures & AG_EXTENDED_ERROR_CODES)
	{
    	static const char cmee[] = "AT+CMEE=1\r";
    
    	/* Send the AT cmd over the air */
    	hfpSendAtCmd(&hfp->task, strlen(cmee), cmee);
    }
    else
    {
    	sendExtendedErrorCfmToApp(hfp, hfp_fail);
    }
}


/****************************************************************************
NAME	
	hfpHandleExtendedErrorReqError

DESCRIPTION
	Extended error reporting can't be requested from the AG because the 
    profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleExtendedErrorReqError(HFP *hfp)
{
	/* Send a cfm indicating an error has occurred */
	sendExtendedErrorCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleCmeeAtAck

DESCRIPTION
	Extended error reporting request command has been acknowledged by the AG.

RETURNS
	void
*/
void hfpHandleCmeeAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Tell the app about this */
	sendExtendedErrorCfmToApp(hfp, status);
}
