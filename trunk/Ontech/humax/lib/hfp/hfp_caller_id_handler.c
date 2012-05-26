/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004

FILE NAME
	hfp_caller_id_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_caller_id_handler.h"
#include "hfp_common.h"
#include "hfp_send_data.h"

#include <panic.h>
#include <string.h>


/*****************************************************************************/
static void sendCallerIdEnableCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_CALLER_ID_ENABLE_CFM, hfp, status);
}


/*****************************************************************************/
static void sendCallerIdMsgToApp(HFP *hfp, uint8 type, uint16 size_name, uint16 size_number, const uint8 *number)
{
    /* If the local device does not support CLI presentation just ignore this */
	if (hfp->hfpSupportedFeatures & HFP_CLI_PRESENTATION)
	{
		checkHfpProfile(hfp->hfpSupportedProfile);
		{
		MAKE_HFP_MESSAGE_WITH_LEN(HFP_CALLER_ID_IND, size_number);
		message->hfp = hfp;
		message->number_type = type;
		message->size_caller_number = size_number;
	    message->size_caller_name   = size_name;
		
		if (size_number)
			memmove(message->caller_number, number, size_number);	
		else
			message->caller_number[0] = 0;

		MessageSend(hfp->clientTask, HFP_CALLER_ID_IND, message);
		}
	}
}


/*****************************************************************************/
void hfpHandleCallerIdEnableReq(HFP *hfp, const HFP_INTERNAL_AT_CLIP_REQ_T *req)
{
	/* Make sure the local device supports caller id */
	if (hfp->hfpSupportedFeatures & HFP_CLI_PRESENTATION)
	{
		char *caller_id;

		if (req->enable)
			caller_id = "AT+CLIP=1\r";
		else
			caller_id = "AT+CLIP=0\r";

		/* Send the AT cmd over the air */
		hfpSendAtCmd(&hfp->task, strlen(caller_id), caller_id);
	}
	else
	{
		/* Send an error - caller id presentation not enabled in supported features */
		sendCallerIdEnableCfmToApp(hfp, hfp_fail);
	}
}


/*****************************************************************************/
void hfpHandleCallerIdEnableReqError(HFP *hfp)
{
	/* Send a cfm indicating an error has occurred */
	sendCallerIdEnableCfmToApp(hfp, hfp_fail);
}


/*****************************************************************************/
void hfpHandleClipAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Tell the app about this */
	sendCallerIdEnableCfmToApp(hfp, status);
}


/****************************************************************************
AT INDICATION
	+CLIP
*/
void hfpHandleCallerId(Task profileTask, const struct hfpHandleCallerId *ind)
{
	HFP *hfp = (HFP *) profileTask;

    /* Only send message if remote end sent us a number */    
    sendCallerIdMsgToApp(hfp, ind->type, 0, ind->num.length, ind->num.data);
}


/****************************************************************************
AT INDICATION
	+CLIP
*/
void hfpHandleCallerIdIllegal(Task profileTask, const struct hfpHandleCallerIdIllegal *ind)
{
	HFP *hfp = (HFP *) profileTask;

    /* Only send message if remote end sent us a number */    
    sendCallerIdMsgToApp(hfp, 0, 0, ind->num.length, ind->num.data);
}


/****************************************************************************
AT INDICATION
	+CLIP
*/
void hfpHandleCallerIdWithName(Task profileTask, const struct hfpHandleCallerIdWithName *ind)
{
	HFP *hfp = (HFP *) profileTask;

	/* If the local device does not support CLI presentation just ignore this */
	if (hfp->hfpSupportedFeatures & HFP_CLI_PRESENTATION)
	{
		/* Only send message if remote end sent us a number */
        sendCallerIdMsgToApp(hfp, ind->type, ind->name.length, ind->num.length, ind->num.data);

        /* Send a single ind message with the name (if supplied) */
        if (ind->name.length && !hfp->caller_id_name_sent)
        {			
			checkHfpProfile(hfp->hfpSupportedProfile);
			{
			MAKE_HFP_MESSAGE_WITH_LEN(HFP_CALLER_ID_NAME_IND, ind->name.length);
    		message->hfp = hfp;
    		message->size_caller_name = ind->name.length;
    		memmove(message->caller_name, ind->name.data, ind->name.length);
    		MessageSend(hfp->clientTask, HFP_CALLER_ID_NAME_IND, message);
			}
            hfp->caller_id_name_sent = 1;
        }
	}
}
