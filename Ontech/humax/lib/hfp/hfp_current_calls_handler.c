/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005-2009
Part of Stereo-Headset-SDK 2009.R2

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
#include "hfp_current_calls_handler.h"
#include "hfp_common.h"
#include "hfp_send_data.h"

#include <panic.h>
#include <string.h>


static void sendCurrentCallsCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_CURRENT_CALLS_CFM, hfp, status);
}

static void sendCurrentCallsIndToApp(HFP *hfp, uint16 call_idx, uint8 direction, uint8 status, uint8 mode, uint8 multiparty, uint8 type, uint8 number_length, const uint8 *number)
{
    bool valid = FALSE;
	
    /* Validate message parameters */
    if ( direction <= hfp_call_mobile_terminated )
    {
        if ( status <= hfp_call_waiting )
        {
            if ( mode <= hfp_call_fax )
            {
                if ( multiparty <= hfp_multiparty_call )
                {
                    valid = TRUE;
                }
            }
        }
    }
         
    /* If any parameters are out of spec, silently ignore this message */           
    if ( valid )
    {
	    MAKE_HFP_MESSAGE_WITH_LEN(HFP_CURRENT_CALLS_IND, number_length);
	    message->hfp = hfp;
        message->call_idx = call_idx;
        message->direction = (hfp_call_direction)direction;
        message->status = (hfp_call_status)status;
        message->mode = (hfp_call_mode)mode;
        message->multiparty = (hfp_call_multiparty)multiparty;
        message->number_type = hfpConvertNumberType(type);
        message->size_number = number_length;
    
	    if ( number_length )
	    {
    	    memmove(message->number, number, number_length);
    	}
    	else
    	{
        	message->number[0] = 0;
    	}
    	
    	MessageSend(hfp->clientTask, HFP_CURRENT_CALLS_IND, message);
	}
}

/****************************************************************************
NAME	
	hfpHandleCurrentCallsGetReq

DESCRIPTION
	Request current calls list from the AG.

RETURNS
	void
*/
void hfpHandleCurrentCallsGetReq(HFP *hfp)
{
	/* Only send the cmd if the AG and local device support the enhanced call status feature.*/
	if ((hfp->hfpSupportedFeatures & HFP_ENHANCED_CALL_STATUS) /*&&
        (hfp->agSupportedFeatures & AG_ENHANCED_CALL_STATUS)*/)
	{
	    static const char clcc[] = "AT+CLCC\r";

	    /* Send the AT cmd over the air */
	    hfpSendAtCmd(&hfp->task, strlen(clcc), clcc);
    }
    else
    {
    	sendCurrentCallsCfmToApp(hfp, hfp_fail);
    }
}


/****************************************************************************
NAME	
	hfpHandleCurrentCallsGetReqError

DESCRIPTION
	Current calls list can't be obtained from the AG because the 
    profile instance is in the wrong state.

RETURNS
	void
*/
void hfpHandleCurrentCallsGetReqError(HFP *hfp)
{
	/* Send a cfm indicating an error has occurred */
	sendCurrentCallsCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleClccAtAck

DESCRIPTION
	Current calls list request command has been acknowledged (completed)
    by the AG.

RETURNS
	void
*/
void hfpHandleClccAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Tell the app about this */
	sendCurrentCallsCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleCurrentCalls

DESCRIPTION
	Current calls info indication received from the AG

AT INDICATION
	+CNUM

RETURNS
	void
*/
void hfpHandleCurrentCalls(Task profileTask, const struct hfpHandleCurrentCalls *ind)
{
	HFP *hfp = (HFP *) profileTask;
		
	/* Silently ignore AT command if either HF does not support the enhanced call status 
	   feature or we are not HFP v.15 */
	if ((hfp->hfpSupportedFeatures & HFP_ENHANCED_CALL_STATUS) && supportedProfileIsHfp15(hfp->hfpSupportedProfile))
		sendCurrentCallsIndToApp(hfp, ind->idx, ind->dir, ind->status, ind->mode, ind->mprty, 0, 0, 0);
}


/****************************************************************************
NAME	
	hfpHandleCurrentCallsWithNumber

DESCRIPTION
	Current calls info indication received from the AG

AT INDICATION
	+CNUM

RETURNS
	void
*/
void hfpHandleCurrentCallsWithNumber(Task profileTask, const struct hfpHandleCurrentCallsWithNumber *ind)
{
	HFP *hfp = (HFP *) profileTask;
		
	/* Silently ignore AT command if either HF does not support the enhanced call status 
	   feature or we are not HFP v.15 */
	if ((hfp->hfpSupportedFeatures & HFP_ENHANCED_CALL_STATUS) && supportedProfileIsHfp15(hfp->hfpSupportedProfile))
	{
		checkHfpProfile15(hfp->hfpSupportedProfile);
		
    	if (ind->number.length)
			sendCurrentCallsIndToApp(hfp, ind->idx, ind->dir, ind->status, ind->mode, ind->mprty, ind->type, ind->number.length, ind->number.data);
    	else
    		sendCurrentCallsIndToApp(hfp, ind->idx, ind->dir, ind->status, ind->mode, ind->mprty, ind->type, 0, 0);
	}
}

