/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
	hfp_subscriber_num_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_subscriber_num_handler.h"
#include "hfp_common.h"
#include "hfp_send_data.h"

#include <panic.h>
#include <string.h>


static void sendSubscriberNumberCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_SUBSCRIBER_NUMBER_CFM, hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleSubscriberNumberGetReq

DESCRIPTION
	Request subscriber number information from the AG.

RETURNS
	void
*/
void hfpHandleSubscriberNumberGetReq(HFP *hfp)
{
	static const char cnum[] = "AT+CNUM\r";

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(cnum), cnum);
}


/****************************************************************************
NAME	
	hfpHandleSubscriberNumberGetReqError

DESCRIPTION
	Subscriber number information can't be obtained from the AG because the 
    profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleSubscriberNumberGetReqError(HFP *hfp)
{
	/* Send a cfm indicating an error has occurred */
	sendSubscriberNumberCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleCnumAtAck

DESCRIPTION
	Subscriber number information request command has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleCnumAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Tell the app about this */
	sendSubscriberNumberCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleSubscriberNumber

DESCRIPTION
	Subscriber number info indication received from the AG

AT INDICATION
	+CNUM

RETURNS
	void
*/
void hfpHandleSubscriberNumber(Task profileTask, const struct hfpHandleSubscriberNumber *ind)
{
	HFP *hfp = (HFP *) profileTask;

	/* Silently ignore AT command if we are not HFP v.15 */
	if (supportedProfileIsHfp15(hfp->hfpSupportedProfile))
	{
		checkHfpProfile15(hfp->hfpSupportedProfile);
    
		if ( ind->service <= hfp_service_fax )
    	{
			/* Send Indication to App */
    		MAKE_HFP_MESSAGE_WITH_LEN(HFP_SUBSCRIBER_NUMBER_IND, ind->number.length);
    		message->hfp = hfp;
    		message->service = (hfp_subscriber_service)ind->service;
    		message->number_type = hfpConvertNumberType(ind->type);
    		message->size_number = ind->number.length;
    	
    		if (ind->number.length)
    	   		memmove(message->number, ind->number.data, ind->number.length);
        	else
            	message->number[0] = 0;
        
    		MessageSend(hfp->clientTask, HFP_SUBSCRIBER_NUMBER_IND, message);
    	}
    }
}
