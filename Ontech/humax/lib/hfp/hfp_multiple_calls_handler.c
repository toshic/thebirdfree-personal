/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_multiple_calls_handler.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_multiple_calls_handler.h"
#include "hfp_parse.h"
#include "hfp_send_data.h"
#include "hfp_slc_handler.h"

#include <panic.h>
#include <stdio.h>
#include <string.h>


/* Create a message to the app telling it the outcome of the call waiting enable request */
static void sendCallWaitingEnableCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_CALL_WAITING_ENABLE_CFM, hfp, status);
}


/* Format and send a CHLD command to the AG with the specified value */
static void sendChldCmdToAg(HFP *hfp, uint16 chld_value)
{
	char chld[15];
	sprintf(chld, "AT+CHLD=%d\r", chld_value);

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(chld), chld);
}


/* Format and send a CHLD command to the AG with the specified value and call index*/
static void sendChldIdxCmdToAg(HFP *hfp, uint16 chld_value, uint16 call_idx)
{
	char chld[21];
	sprintf(chld, "AT+CHLD=%d%d\r", chld_value, call_idx);

	/* Send the AT cmd over the air */
	hfpSendAtCmd(&hfp->task, strlen(chld), chld);
}


/* Send a cfm to the app telling it the outcome of sending AT+CHLD=0 to the AG */
static void sendChldZeroCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_RELEASE_HELD_REJECT_WAITING_CALL_CFM, hfp, status);
}


/* Send a cfm to the app telling it the outcome of sending AT+CHLD=1 to the AG */
static void sendChldOneCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_RELEASE_ACTIVE_ACCEPT_OTHER_CALL_CFM, hfp, status);
}


/* Send a cfm to the app telling it the outcome of sending AT+CHLD=1<idx> to the AG */
static void sendChldOneIdxCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_RELEASE_SPECIFIED_ACCEPT_OTHER_CALL_CFM, hfp, status);
}


/* Send a cfm to the app telling it the outcome of sending AT+CHLD=2 to the AG */
static void sendChldTwoCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_HOLD_ACTIVE_ACCEPT_OTHER_CALL_CFM, hfp, status);
}


/* Send a cfm to the app telling it the outcome of sending AT+CHLD=2<idx> to the AG */
static void sendChldTwoIdxCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_REQUEST_PRIVATE_HOLD_OTHER_CALL_CFM, hfp, status);
}


/* Send a cfm to the app telling it the outcome of sending AT+CHLD=3 to the AG */
static void sendChldThreeCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_ADD_HELD_CALL_CFM, hfp, status);
}


/* Send a cfm to the app telling it the outcome of sending AT+CHLD=4 to the AG */
static void sendChldFourCfmToApp(HFP *hfp, hfp_lib_status status)
{
	/* Send a cfm message to the application. */
	hfpSendCommonCfmMessageToApp(HFP_EXPLICIT_CALL_TRANSFER_CFM, hfp, status);
}


/* Send call waiting notification to the app */
static void sendCallWaitingNotificationToApp(HFP *hfp, uint8 type, uint16 length, const uint8 *number)
{	
	/* Only pass this info to the app if the HFP supports this functionality */
	if (hfp->hfpSupportedFeatures & HFP_THREE_WAY_CALLING)
	{
		checkHfpProfile(hfp->hfpSupportedProfile);

		/* Send a call waiting indication to the app */
		{
		MAKE_HFP_MESSAGE_WITH_LEN(HFP_CALL_WAITING_IND, length);
		message->hfp = hfp;
		message->number_type = hfpConvertNumberType(type);
		message->size_caller_number = length;

		if (length)
			memmove(message->caller_number, number, length);
		else
			message->caller_number[0] = 0;
		
		MessageSend(hfp->clientTask, HFP_CALL_WAITING_IND, message);
		}
	}
}


/****************************************************************************
NAME	
	hfpHandleCallHoldInfo

DESCRIPTION
	Generic call hold parameter handler. Called when we don't have a 
	dictionary match for the call hold string.

AT INDICATION
	+CHLD

RETURNS
	void
*/
void hfpHandleCallHoldInfo(Task profileTask, const struct hfpHandleCallHoldInfo *ind)
{
	HFP *hfp = (HFP *) profileTask;	

	ind = ind;

	/* If you add anything here, remember to put it in hfpHandleCallHoldInfoCommon
	 * too (see below).*/

	/* Send a message to indicate we've received the call hold features */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CALL_HOLD_SUPPORT_IND, 0);
}


/****************************************************************************
NAME	
	hfpHandleCallHoldInfoCommon

DESCRIPTION
	Call hold parameter handler for one very common set of call hold parameters -
	namely when the string looks like "\r\n+CHLD: (0,1,1x,2,2x,3,4)\r\n". 

AT INDICATION
	+CHLD

RETURNS
	void
*/
void hfpHandleCallHoldInfoCommon(Task profileTask)
{
	/* Do whatever hfpHandleCallHoldInfo would do when called with the
	 * string "\r\n+CHLD: (0,1,1x,2,2x,3,4)\r\n".*/

	HFP *hfp = (HFP *) profileTask;

	/* Send a message to indicate we've received the call hold features */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CALL_HOLD_SUPPORT_IND, 0);
}


/****************************************************************************
NAME	
	hfpHandleCallHoldInfoRange

DESCRIPTION
	Generic call hold parameter handler used when the params are specified as 
	a range of values. Called when we don't have a dictionary match for the 
	call hold string.

AT INDICATION
	+CHLD

RETURNS
	void
*/
void hfpHandleCallHoldInfoRange(Task profileTask, const struct hfpHandleCallHoldInfoRange *ind)
{
	HFP *hfp = (HFP *) profileTask;

	ind= ind;

	/* Send a message to indicate we've received the call hold features */
	MessageSend(&hfp->task, HFP_INTERNAL_AT_CALL_HOLD_SUPPORT_IND, 0);
}


/****************************************************************************
NAME	
	hfpHandleCallHoldSupportInd

DESCRIPTION
	Call hold parameters received and parsed. 

RETURNS
	void
*/
void hfpHandleCallHoldSupportInd(HFP *hfp)
{
	/* Inform the app the SLC has been established */
	hfpSendSlcConnectCfmToApp(hfp_connect_success, hfp);
}


/****************************************************************************
NAME	
	hfpHandleCallWaitingNotificationEnable

DESCRIPTION
	Enable call waiting notifications from the AG.

RETURNS
	void
*/
void hfpHandleCallWaitingNotificationEnable(HFP *hfp, const HFP_INTERNAL_AT_CCWA_REQ_T *req)
{
	/* Only send this message if the AG and HF support this functionality */
	if ((hfp->hfpSupportedFeatures & HFP_THREE_WAY_CALLING) &&
		(hfp->agSupportedFeatures & AG_THREE_WAY_CALLING))
	{
		char ccwa_en[15];
		sprintf(ccwa_en, "AT+CCWA=%d\r", req->enable);

		/* Send the AT cmd over the air */
		hfpSendAtCmd(&hfp->task, strlen(ccwa_en), ccwa_en);
	}
	else
		/* Send an error message */
		sendCallWaitingEnableCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleCallWaitingNotificationEnableError

DESCRIPTION
	Send an error immediately because the cmd could not be sent out.

RETURNS
	void
*/
void hfpHandleCallWaitingNotificationEnableError(HFP *hfp)
{
	/* Send an error message the profile instance is in the wrong state */
	sendCallWaitingEnableCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleCcwaAtAck

DESCRIPTION
	AG has responded with OK or ERROR to out call waiting notification enable 
	request so pass this result directly to the app.

RETURNS
	void
*/
void hfpHandleCcwaAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Pass the result directly to the app */
	sendCallWaitingEnableCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleCallWaitingNotification

DESCRIPTION
	This function handles the +CCWA notification received from the AG 
	indicating there is an incoming third party call. The profileTask 
	argument specifies which profile instance this indication was sent to.
	The call_notification contains the calling number of the remote party
	that the AG sent in the +CCWA indication. The received information is 
	put into a HFP_CALL_WAITING_IND message and sent to the application.

AT INDICATION
	+CCWA

RETURNS
	void
*/
void hfpHandleCallWaitingNotification(Task profileTask, const struct hfpHandleCallWaitingNotification *call_notification)
{
    sendCallWaitingNotificationToApp((HFP *) profileTask, call_notification->type, call_notification->num.length, call_notification->num.data);
}

/****************************************************************************
NAME	
	hfpHandleCallWaitingNotificationWithName

DESCRIPTION
	This function handles the +CCWA notification received from the AG 
	indicating there is an incoming third party call. The profileTask 
	argument specifies which profile instance this indication was sent to.
	The call_notification contains the calling number of the remote party
	that the AG sent in the +CCWA indication. The received information is 
	put into a HFP_CALL_WAITING_IND message and sent to the application.

AT INDICATION
	+CCWA

RETURNS
	void
*/
void hfpHandleCallWaitingNotificationWithName(Task profileTask, const struct hfpHandleCallWaitingNotificationWithName *call_notification)
{
	HFP *hfp = (HFP*)profileTask;
	
    sendCallWaitingNotificationToApp(hfp, call_notification->type, call_notification->num.length, call_notification->num.data);
    
    /* Send a single ind message with the name (if supplied) */
    if (call_notification->name.length)
    {
	    uint16 length = (call_notification->name.length < 32) ? call_notification->name.length+1 : 32;  /* AT Parser strings are NOT NULL terminated */
	    
		checkHfpProfile(hfp->hfpSupportedProfile);
		
		/* Send a call waiting indication to the app */
		{
		MAKE_HFP_MESSAGE_WITH_LEN(HFP_CALL_WAITING_NAME_IND, length);
		message->hfp = hfp;
		message->size_caller_name = length-1;
		memmove(message->caller_name, call_notification->name.data, length-1);
			
		MessageSend(hfp->clientTask, HFP_CALL_WAITING_NAME_IND, message);
		}
    }
}

/****************************************************************************
NAME	
	hfpHandleCallWaitingNotificationIllegal

DESCRIPTION
	This function handles the +CCWA notification received from the AG 
	indicating there is an incoming third party call. This function is
    identical to hfpHandleCallWaitingNotification but has been added to handle
    AGs which illegally send the +CCWA indication with fewer arguments
    than specified by the HFP and GSM07.07 specs. Since this is a minor spec
    violation we've got this workaround to handle it.

AT INDICATION
	+CCWA

RETURNS
	void
*/
void hfpHandleCallWaitingNotificationIllegal(Task profileTask, const struct hfpHandleCallWaitingNotificationIllegal *notification)
{
    sendCallWaitingNotificationToApp((HFP *) profileTask, 0, notification->num.length, notification->num.data);
}


/****************************************************************************
NAME	
	hfpHandleChldZero

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=0
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this either results in all held calls being released
	or it sets User Determined User Busy (UDUB) for a waiting call.

RETURNS
	void
*/
void hfpHandleChldZero(HFP *hfp)
{
	/* Send AT+CHLD=0 */
	sendChldCmdToAg(hfp, 0);
}


/****************************************************************************
NAME	
	hfpHandleChldZeroError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldZeroError(HFP *hfp)
{
	/* Send a cfm with status fail */
	sendChldZeroCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleChldZeroAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=0. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldZeroAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Pass the result on to the app */
	sendChldZeroCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleChldOne

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=1
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this results in all active calls being released 
	and the held or waiting call being accepted.

RETURNS
	void
*/
void hfpHandleChldOne(HFP *hfp)
{
	/* Send AT+CHLD=1 */
	sendChldCmdToAg(hfp, 1);
}


/****************************************************************************
NAME	
	hfpHandleChldOneError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldOneError(HFP *hfp)
{
	/* Send a cfm with status fail */
	sendChldOneCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleChldOneAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=1. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldOneAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Pass the result on to the app */
	sendChldOneCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleChldOneIdx

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=1<idx>
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this results in all active calls being released 
	and the held or waiting call being accepted.

RETURNS
	void
*/
void hfpHandleChldOneIdx(HFP *hfp, const HFP_INTERNAL_AT_CHLD_1X_REQ_T *msg)
{
	/* Only send the cmd if the AG and local device support the enhanced call control feature */
	if ((hfp->hfpSupportedFeatures & HFP_ENHANCED_CALL_CONTROL) &&
            (hfp->agSupportedFeatures & AG_ENHANCED_CALL_CONTROL))
	{
    	/* Send AT+CHLD=1<idx> */
	    sendChldIdxCmdToAg(hfp, 1, msg->call_idx);
    }
    else
    {
    	sendChldOneIdxCfmToApp(hfp, hfp_fail);
    }
}


/****************************************************************************
NAME	
	hfpHandleChldOneIdxError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldOneIdxError(HFP *hfp)
{
	/* Send a cfm with status fail */
	sendChldOneIdxCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleChldOneIdxAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=1. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldOneIdxAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Pass the result on to the app */
	sendChldOneIdxCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleChldTwo

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=2
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this places all active calls on hold and accepts 
	the held or waiting call.

RETURNS
	void
*/
void hfpHandleChldTwo(HFP *hfp)
{
	/* Send AT+CHLD=2 */
	sendChldCmdToAg(hfp, 2);
}


/****************************************************************************
NAME	
	hfpHandleChldTwoError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldTwoError(HFP *hfp)
{
	/* Send a cfm with status fail */
	sendChldTwoCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleChldTwoAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=2. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldTwoAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Pass the result on to the app */
	sendChldTwoCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleChldTwoIdx

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=2<idx>
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this places all active calls on hold and accepts 
	the held or waiting call.

RETURNS
	void
*/
void hfpHandleChldTwoIdx(HFP *hfp, const HFP_INTERNAL_AT_CHLD_2X_REQ_T *msg)
{
	/* Only send the cmd if the AG and local device support the enhanced call control feature */
	if ((hfp->hfpSupportedFeatures & HFP_ENHANCED_CALL_CONTROL) &&
            (hfp->agSupportedFeatures & AG_ENHANCED_CALL_CONTROL))
	{
    	/* Send AT+CHLD=2<idx> */
	    sendChldIdxCmdToAg(hfp, 2, msg->call_idx);
    }
    else
    {
    	sendChldTwoIdxCfmToApp(hfp, hfp_fail);
    }
}


/****************************************************************************
NAME	
	hfpHandleChldTwoIdxError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldTwoIdxError(HFP *hfp)
{
	/* Send a cfm with status fail */
	sendChldTwoIdxCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleChldTwoIdxAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=2. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldTwoIdxAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Pass the result on to the app */
	sendChldTwoIdxCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleChldThree

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=3
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this adds the held call to the conversation.

RETURNS
	void
*/
void hfpHandleChldThree(HFP *hfp)
{
	/* Send AT+CHLD=3 */
	sendChldCmdToAg(hfp, 3);
}


/****************************************************************************
NAME	
	hfpHandleChldThreeError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldThreeError(HFP *hfp)
{
	/* Send a cfm with status fail */
	sendChldThreeCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleChldThreeAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=3. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldThreeAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Pass the result on to the app */
	sendChldThreeCfmToApp(hfp, status);
}


/****************************************************************************
NAME	
	hfpHandleChldFour

DESCRIPTION
	This function take a profile instance (hfp) and sends an AT+CHLD=4
	AT command to the AG to handle multiparty calls. Depending on the state
	of the calls at the AG this connects the two external calls and disconnects 
	the subscriber from both calls (explicit call transfer). Support for this 
	functionality is optional for the HF device.

RETURNS
	void
*/
void hfpHandleChldFour(HFP *hfp)
{
	/* Send AT+CHLD=4 */
	sendChldCmdToAg(hfp, 4);
}


/****************************************************************************
NAME	
	hfpHandleChldFourError

DESCRIPTION
	When this function is called, the profile instance referenced by hfp
	is in the wrong state for the requested action to be performed and so we
	send an error message to the app immediately.

RETURNS
	void
*/
void hfpHandleChldFourError(HFP *hfp)
{
	/* Send a cfm with status fail */
	sendChldFourCfmToApp(hfp, hfp_fail);
}


/****************************************************************************
NAME	
	hfpHandleChldFourAtAck

DESCRIPTION
	This function handles the receipt of an OK or ERROR response to a
	multiparty call handling request, specifically AT+CHLD=4. The profile 
	instance is specified by hfp. The status argument is used to inform the 
	application whether the AT command was recognised by the AG or not. 

RETURNS
	void
*/
void hfpHandleChldFourAtAck(HFP *hfp, hfp_lib_status status)
{
	/* Pass the result on to the app */
	sendChldFourCfmToApp(hfp, status);
}
