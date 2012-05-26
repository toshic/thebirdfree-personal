/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_network_operator_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_network_operator_handler.h"
#include "hfp_common.h"
#include "hfp_send_data.h"

#include <panic.h>
#include <string.h>


static void sendNetworkOperatorCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_NETWORK_OPERATOR_CFM, hfp, status);
}


static void handleNetworkOperator( HFP *hfp, uint8 mode, uint8 format, uint16 length, const uint8 *name )
{	
	checkHfpProfile15(hfp->hfpSupportedProfile);
	
	/* Silently ignore AT command if we are not HFP v.15 */
	if (supportedProfileIsHfp15(hfp->hfpSupportedProfile))
	{
		MAKE_HFP_MESSAGE_WITH_LEN(HFP_NETWORK_OPERATOR_IND, length);
		message->hfp = hfp;
		message->mode = mode;
	
    	/* Only supply operator name if it's in the correct format (i.e format==0) */	
    	if (!format && length)
		{
			message->size_operator_name = length;
        	memmove(message->operator_name, name, length);
		}
    	else
		{
			message->size_operator_name = 0;
        	message->operator_name[0] = 0;
		}
		
		MessageSend(hfp->clientTask, HFP_NETWORK_OPERATOR_IND, message);
	}
}


static void sendNetworkOperatorReportingFormat(HFP *hfp)
{
	static const char cops[] = "AT+COPS=3,0\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(cops), cops);
}


static void sendNetworkOperatorRequest(HFP *hfp)
{
	static const char cops[] = "AT+COPS?\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(cops), cops);
}


/****************************************************************************
NAME	
	hfpHandleNetworkOperatorReq

DESCRIPTION
	Request network operator information from the AG.

RETURNS
	void
*/
void hfpHandleNetworkOperatorReq(HFP *hfp)
{
	if ( hfp->cops_format_set )
	{	/* Reporting format already set, just issue a request for information */
		sendNetworkOperatorRequest(hfp);
	}
	else
	{	/* Need to set the reporting format first */
		sendNetworkOperatorReportingFormat(hfp);
	}
}


/****************************************************************************
NAME	
	hfpHandleNetworkOperatorReqError

DESCRIPTION
	Network operator information can't be obtained from the AG because the 
    profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleNetworkOperatorReqError(HFP *hfp)
{
	/* Send a cfm indicating an error has occurred */
	sendNetworkOperatorCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleCopsFormatAtAck

DESCRIPTION
	Network operator reporting format command has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleCopsFormatAtAck(HFP *hfp, hfp_lib_status status)
{
	if ( status==hfp_success )
	{	/* Reporting format now set, issue request for information */
		hfp->cops_format_set = TRUE;
		sendNetworkOperatorRequest(hfp);
	}
	else
	{	/* Inform app of failure */
		sendNetworkOperatorCfmToApp(hfp, status);
	}
}


/****************************************************************************
NAME	
	hfpHandleCopsReqAtAck

DESCRIPTION
	Network operator information request command has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleCopsReqAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Tell the app about this */
	sendNetworkOperatorCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleNetworkOperatorMode

DESCRIPTION
	Network operator info indication received from the AG containing only the
	network mode.

AT INDICATION
	+COPS

RETURNS
	void
*/
void hfpHandleNetworkOperatorMode(Task profileTask, const struct hfpHandleNetworkOperatorMode *ind)
{
	HFP *hfp = (HFP *) profileTask;
    handleNetworkOperator(hfp, ind->mode, 0, 0, 0);
}


/****************************************************************************
NAME	
	hfpHandleNetworkOperatorModeName

DESCRIPTION
	Network operator info indication received from the AG containing both the
	network mode and operator name.

AT INDICATION
	+COPS

RETURNS
	void
*/
void hfpHandleNetworkOperatorModeName(Task profileTask, const struct hfpHandleNetworkOperatorModeName *ind)
{
	HFP *hfp = (HFP *) profileTask;
    handleNetworkOperator(hfp, ind->mode, ind->format, ind->operator.length, ind->operator.data);
}
