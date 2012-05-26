/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2009
Part of Stereo-Headset-SDK 2009.R2

FILE NAME
	hfp_csr_features.c

DESCRIPTION
	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_csr_features_handler.h"
#include "hfp_send_data.h"
#include "hfp_parse.h"

#include <panic.h>
#include <print.h>
#include <stdio.h>
#include <string.h>


/*
	Maximum Power Level supported by the CSR Extensions.
*/
#define HFP_CSR_MAX_PWR (9)

/*
	Maximum length of names and messages in bytes
*/
#define HFP_CSR_MAX_STRING (32)

static void hfpSendCsrSupportedFeaturesCfm(HFP *hfp, hfp_lib_status status, bool callerName, bool rawText, bool smsInd, bool battLevel, bool pwrSource , uint16 codecs)
{
    /* Send an internal message */
    MAKE_HFP_MESSAGE(HFP_CSR_SUPPORTED_FEATURES_CFM);
    message->hfp = hfp;
    message->status = status;
	message->callerName = callerName;
	message->rawText = rawText;
	message->smsInd = smsInd;
	message->battLevel = battLevel;
	message->pwrSource = pwrSource;
	message->codecs = codecs ;
    MessageSend(hfp->clientTask, HFP_CSR_SUPPORTED_FEATURES_CFM, message);
}

void hfpSendCsrModifyIndicatorsCfm(HFP *hfp, hfp_lib_status status)
{
    MAKE_HFP_MESSAGE(HFP_CSR_MODIFY_INDICATORS_CFM);
    message->hfp = hfp;
    message->status = status;
    MessageSend(hfp->clientTask, HFP_CSR_MODIFY_INDICATORS_CFM, message);
}

void hfpSendCsrSmsCfm(HFP *hfp, hfp_lib_status status, uint16 length, const uint8 *sms)
{
	uint16 len = (length < HFP_CSR_MAX_STRING) ? length+1 : HFP_CSR_MAX_STRING;  /* AT Parser strings are NOT NULL terminated */
	
    MAKE_HFP_MESSAGE_WITH_LEN(HFP_CSR_SMS_CFM, len);
    message->hfp = hfp;
    message->status = status;
    if (len > 1)
    {
	    message->size_sms = len;
	    memmove(message->sms, sms, len-1);
	    message->sms[len-1] = '\0';
    }
    else
    {
	    message->size_sms = 0;
	    message->sms[0] = 0;
    }
    MessageSend(hfp->clientTask, HFP_CSR_SMS_CFM, message);
}


/*
	Handle CSR Supported Features request.
*/
void hfpHandleCsrSupportedFeaturesReq(HFP *hfp, HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ_T *msg)
{
	switch (hfp->state)
	{
	case hfpInitialising:
	case hfpReady:
	case hfpSlcConnecting:
		/* SLC not connected, return an error */
		hfpSendCsrSupportedFeaturesCfm(hfp, hfp_csr_no_slc, FALSE, FALSE, FALSE, FALSE, FALSE , 0 );
		break;
	case hfpSlcConnected:
	case hfpIncomingCallEstablish:
	case hfpOutgoingCallEstablish:
	case hfpOutgoingCallAlerting:
	case hfpActiveCall:
		{
			/* SLC connected, send the command */
			char cmd[22];
				
			/* Create the AT cmd we're sending */
			sprintf(cmd, "AT+CSRSF=%d,%d,%d,%d,%d,%d\r", (msg->callerName?1:0), (msg->rawText?1:0),
										(msg->smsInd?1:0), (msg->battLevel?1:0), (msg->pwrSource?1:0) , msg->codecs);
										
		
			/* Send the AT cmd over the air */
			hfpSendAtCmd(&hfp->task, strlen(cmd), cmd);
          	hfp->use_csr2csr = TRUE;
            
			break;
		}
	default:
		HFP_DEBUG(("Unknown State [0x%x]\n", hfp->state));
		break;
	}
}

/*
	Have received response to AT+CSRSF
*/
void hfpHandleCsrSfAtAck(HFP *hfp, hfp_lib_status status)
{
	if ((status != hfp_success)&&(hfp->use_csr2csr))
	{
    	hfp->use_csr2csr = FALSE;
              
		/* AG does not support any extensions.  Inform the application. */
		hfpSendCsrSupportedFeaturesCfm(hfp, hfp_fail, FALSE, FALSE, FALSE, FALSE, FALSE , 0 );
	}
	/* If the AG responded with OK, we should already have received a +CSRSF containing the information required,
		and this will have been sent to the application, so there is no need to do anything else.
	*/
}

void hfpHandleReponseCSRSupportedFeatures(Task task, const struct hfpHandleReponseCSRSupportedFeatures *features)
{
	HFP *hfp = (HFP*)task;
	if (!hfp->use_csr2csr)
    { /* CSR 2 CSR Extensions not initialised.  Ignore */
	    return;
    }
    else
    {
		/* Send internal message */
		MAKE_HFP_MESSAGE(HFP_INTERNAL_CSR_SUPPORTED_FEATURES_ACK);
		message->callerName = (features->callerName=='0'?FALSE:TRUE);
		message->rawText = (features->rawText=='0'?FALSE:TRUE);
		message->smsInd = (features->smsInd=='0'?FALSE:TRUE);
		message->battLevel = (features->battLevel=='0'?FALSE:TRUE);
		message->pwrSource = (features->pwrSource=='0'?FALSE:TRUE);
		MessageSend(task, HFP_INTERNAL_CSR_SUPPORTED_FEATURES_ACK, message);
	}
}

/*
	Handle CSR Supported Features ack.
*/
void hfpHandleCsrSupportedFeaturesAck(HFP *hfp, HFP_INTERNAL_CSR_SUPPORTED_FEATURES_ACK_T *msg)
{
	hfpSendCsrSupportedFeaturesCfm(hfp, hfp_success, msg->callerName, msg->rawText, msg->smsInd, msg->battLevel, msg->pwrSource , msg->codecs);
}


/*
	Handle CSR Power Level Request
*/
void hfpHandleCsrPowerLevelReq(HFP *hfp, HFP_INTERNAL_CSR_POWER_LEVEL_REQ_T *msg)
{
	if (msg->pwr_level > HFP_CSR_MAX_PWR)
	{
		HFP_DEBUG(("hfpHandleCsrSupportedFeaturesAck : Invalid Power Level\n"));
		return;
	}
	
	switch (hfp->state)
	{
	case hfpInitialising:
	case hfpReady:
	case hfpSlcConnecting:
		/* SLC not connected, Do Nothing */
		break;
	case hfpSlcConnected:
	case hfpIncomingCallEstablish:
	case hfpOutgoingCallEstablish:
	case hfpOutgoingCallAlerting:
	case hfpActiveCall:
		{
			/* SLC connected, send the command */
			char cmd[15];
				
			/* Create the AT cmd we're sending */
			sprintf(cmd, "AT+CSRBATT=%d\r", msg->pwr_level);
		
			/* Send the AT cmd over the air */
			hfpSendAtCmd(&hfp->task, strlen(cmd), cmd);
			break;
		}
	default:
		HFP_DEBUG(("Unknown State [0x%x]\n", hfp->state));
		break;
	}
}

/*
	Handle CSR Power Source Request
*/
void hfpHandleCsrPowerSourceReq(HFP *hfp, HFP_INTERNAL_CSR_POWER_SOURCE_REQ_T *msg)
{
	if (msg->pwr_status > hfp_csr_pwr_rep_external)
	{
		HFP_DEBUG(("hfpHandleCsrSupportedFeaturesAck : Invalid Power Status parameter\n"));
		return;
	}
	
	switch (hfp->state)
	{
	case hfpInitialising:
	case hfpReady:
	case hfpSlcConnecting:
		/* SLC not connected, Do Nothing */
		break;
	case hfpSlcConnected:
	case hfpIncomingCallEstablish:
	case hfpOutgoingCallEstablish:
	case hfpOutgoingCallAlerting:
	case hfpActiveCall:
		{
			/* SLC connected, send the command */
			char cmd[15];
				
			/* Create the AT cmd we're sending */
			sprintf(cmd, "AT+CSRPWR=%d\r", msg->pwr_status);
		
			/* Send the AT cmd over the air */
			hfpSendAtCmd(&hfp->task, strlen(cmd), cmd);
			break;
		}
	default:
		HFP_DEBUG(("Unknown State [0x%x]\n", hfp->state));
		break;
	}
}

/*
	Handle CSR Pwr Source Ack.
*/
void hfpHandleCsrPwrAtAck(HFP *hfp, hfp_lib_status status)
{
#ifdef PRINT_DEBUG_ENABLED
	/* Currently no need to return anything to the application on completion.
		If this becomes a requirement, and internal message should be used in
		the same way as other AT command handlers.
	*/
	PRINT(("hfpHandleCsrPwrAtAck - "));
	if (status == hfp_success)
		PRINT(("Success\n"));
	else
		PRINT(("Failure [0x%x]\n", status));
#endif /* PRINT_DEBUG_ENABLED */
}

/*
	Handle CSR Battery Level Ack.
*/
void hfpHandleCsrBatAtAck(HFP *hfp, hfp_lib_status status)
{
#ifdef PRINT_DEBUG_ENABLED
	/* Currently no need to return anything to the application on completion.
		If this becomes a requirement, and internal message should be used in
		the same way as other AT command handlers.
	*/
	PRINT(("hfpHandleCsrBatAtAck - "));
	if (status == hfp_success)
		PRINT(("Success\n"));
	else
		PRINT(("Failure [0x%x]\n", status));
#endif /* PRINT_DEBUG_ENABLED */
}

/*
	Handle Unsolicited +CSRTTS AT response
*/
void hfpHandleResponseCSRText(Task task, const struct hfpHandleResponseCSRText *text)
{
	HFP *hfp = (HFP*)task;
	if (!hfp->use_csr2csr)
    { /* CSR 2 CSR Extensions not initialised.  Ignore */
	    return;
    }
    else
    {
		uint16 length = (text->text.length < HFP_CSR_MAX_STRING) ? text->text.length+1 : HFP_CSR_MAX_STRING;  /* AT Parser strings are NOT NULL terminated */
		
	    MAKE_HFP_MESSAGE_WITH_LEN(HFP_CSR_TXT_IND, length);
	    message->hfp = hfp;
	    message->size_text = length;
	    memmove(message->text, text->text.data, length-1);
	    message->text[length-1] = '\0';
	    MessageSend(hfp->clientTask, HFP_CSR_TXT_IND, message);
    }
}


/*
	Handle CSR Modify Indicators Request
	
	NOTE: If number of valid indicators becomes more than 99, then the size calculation will need to be modified
*/
void hfpHandleCsrModIndsReq(HFP *hfp, HFP_INTERNAL_CSR_MOD_INDS_REQ_T *msg)
{
	static const char atCmd[] = {'A','T','+','C','S','R','='};
	static const char atEnd[] = {'\r'};
	char *atStr;
	uint16 atLen = sizeof(atCmd) + (msg->size_indicators * (3+2+1)) + msg->size_indicators + sizeof(atEnd); /* size = (,) + 2 for ind + 1 for val */ 
	
	atStr = (char*)malloc(atLen);
	
	if (atStr)
	{
		uint16 used = sizeof(atCmd);
		uint16 cnt;
		memmove(atStr, atCmd, sizeof(atCmd));
		
		for (cnt = 0; cnt < msg->size_indicators; cnt++)
		{
			if (cnt > 0)
			{
				atStr[used] = ',';
				used++;
			}
			atStr[used] = '(';
			used++;
			used += sprintf(&atStr[used], "%d", msg->indicators[cnt].indicator);
			atStr[used] = ',';
			used++;
			used += sprintf(&atStr[used], "%d", msg->indicators[cnt].value);
			atStr[used] = ')';
			used++;
			
		}
		memmove(&atStr[used], atEnd, sizeof(atEnd));
		used++;

		hfpSendAtCmd(&hfp->task, used, atStr);		
		free(atStr);
	}
	else
	{
		hfpSendCsrModifyIndicatorsCfm(hfp, hfp_csr_mod_ind_no_mem);
	}
	free(msg->indicators);
}

/*
	Handle CSR Modify Indicators Ack.
*/
void hfpHandleCsrModIndsReqAck(HFP *hfp, hfp_lib_status status)
{
	if (!hfp->use_csr2csr)
    { /* CSR 2 CSR Extensions not initialised.  Ignore */
	    return;
    }
	hfpSendCsrModifyIndicatorsCfm(hfp, status);
}


/*
	Handle internal Disable Indicators
*/
void hfpCsrMofifyIndicatorsDisableReq(HFP *hfp)
{
	switch (hfp->state)
	{
	case hfpInitialising:
	case hfpReady:
	case hfpSlcConnecting:
		/* SLC not connected, return an error */
		hfpSendCsrModifyIndicatorsCfm(hfp, hfp_csr_no_slc);
		break;
	case hfpSlcConnected:
	case hfpIncomingCallEstablish:
	case hfpOutgoingCallEstablish:
	case hfpOutgoingCallAlerting:
	case hfpActiveCall:
		{
			/* SLC connected, send the command */
			char cmd[20];
				
			/* Create the AT cmd we're sending */
			sprintf(cmd, "AT+CSR=0\r");
		
			/* Send the AT cmd over the air */
			hfpSendAtCmd(&hfp->task, strlen(cmd), cmd);
			break;
		}
	default:
		HFP_DEBUG(("Unknown State [0x%x]\n", hfp->state));
		break;
	}
}


void hfpHandleFeatureNegotiation  ( Task task , const struct hfpHandleFeatureNegotiation *feat ) 
{
	HFP *hfp = (HFP*)task;
 
    PRINT(("CSFN received\n" ));
    
    if ( hfp->use_csr2csr)
    {
        MAKE_HFP_MESSAGE(HFP_CSR_FEATURE_NEGOTIATION_IND);
        message->hfp = hfp;
	    message->indicator = feat->ind;
        message->value = feat->val ;        
        MessageSend(hfp->clientTask, HFP_CSR_FEATURE_NEGOTIATION_IND, message);        
    }
}


void hfpHandleFeatureNegotiationRes ( HFP * hfp , HFP_INTERNAL_CSR_FEATURE_NEGOTIATION_RES_T * msg )
{
    static const char atCmd[] = {'A','T','+','C','S','R','F','N','=','('};
	static const char atEnd[] = {')','\r'};
	char *atStr;
	uint16 atLen = sizeof(atCmd) + sizeof(atEnd) + 3 ; /*one for the indicator, 1 for the comma, 1 for the value*/ 
	
	atStr = (char*)malloc(atLen);
	
	if (atStr)
	{
		uint16 used = sizeof(atCmd);
		memmove(atStr, atCmd, sizeof(atCmd));
		
        
        used += sprintf(&atStr[used], "%d", msg->indicator);
	
    	atStr[used] = ',';
		used++;
	
    	used += sprintf(&atStr[used], "%d", msg->value);
	
    	memmove(&atStr[used], atEnd, sizeof(atEnd));
		
		used+= sizeof(atEnd) ;

		hfpSendAtCmd(&hfp->task, used, atStr);		
		free(atStr);
	}
	else
	{
	   HFP_DEBUG(("malloc failed\n")) ;
	}
}

/*
	Handle new SMS arrival
*/
void hfpHandleResponseCSRSms(Task task, const struct hfpHandleResponseCSRSms *sms)
{
	HFP *hfp = (HFP*)task;
	if (hfp->use_csr2csr)
	{
	    MAKE_HFP_MESSAGE_WITH_LEN(HFP_CSR_NEW_SMS_IND, sms->senderNum.length+1);
	    message->hfp = hfp;
	    message->index = sms->smsIndex;
	    message->size_sender_number = sms->senderNum.length+1;
	    memmove(message->sender_number, sms->senderNum.data, sms->senderNum.length);
	    message->sender_number[sms->senderNum.length] = '\0';
	    MessageSend(hfp->clientTask, HFP_CSR_NEW_SMS_IND, message);
	    
	    if (sms->senderName.length > 0)
	    {
			uint16 length = (sms->senderName.length < HFP_CSR_MAX_STRING) ? sms->senderName.length+1 : HFP_CSR_MAX_STRING;  /* AT Parser strings are NOT NULL terminated */
		    MAKE_HFP_MESSAGE_WITH_LEN(HFP_CSR_NEW_SMS_NAME_IND, length);
		    message->hfp = hfp;
		    message->index = sms->smsIndex;
		    message->size_sender_name = length;
		    memmove(message->sender_name, sms->senderName.data, length-1);
		    message->sender_name[length-1] = '\0';
		    MessageSend(hfp->clientTask, HFP_CSR_NEW_SMS_NAME_IND, message);
	    }
	}
}


/*
	Handle internal Get SMS request.
*/
void hfpHandleCsrGetSmsReq(HFP *hfp, HFP_INTERNAL_CSR_GET_SMS_REQ_T *msg)
{
	switch (hfp->state)
	{
	case hfpInitialising:
	case hfpReady:
	case hfpSlcConnecting:
		/* SLC not connected, return an error */
		hfpSendCsrSmsCfm(hfp, hfp_csr_no_slc, 0, NULL);
		break;
	case hfpSlcConnected:
	case hfpIncomingCallEstablish:
	case hfpOutgoingCallEstablish:
	case hfpOutgoingCallAlerting:
	case hfpActiveCall:
		{
			/* SLC connected, send the command */
			char cmd[20];
				
			/* Create the AT cmd we're sending */
			sprintf(cmd, "AT+CSRGETSMS=%d\r", msg->index);
		
			/* Send the AT cmd over the air */
			hfpSendAtCmd(&hfp->task, strlen(cmd), cmd);
			break;
		}
	default:
		HFP_DEBUG(("Unknown State [0x%x]\n", hfp->state));
		break;
	}
}

/*
	Handle Get SMS ack.
*/
void hfpHandleCsrGetSmsAck(HFP *hfp, hfp_lib_status status)
{
	if ((status != hfp_success) && (hfp->use_csr2csr))
	{
		/* AG does not support any extensions.  Inform the application. */
		hfpSendCsrSmsCfm(hfp, status, 0, NULL);
	}
	/* If the AG responded with OK, we should already have received a +CSRGETSMS containing the information required,
		and this will have been sent to the application, so there is no need to do anything else.
	*/
}

/*
	Handle response to a AT+CSRGETSMS
*/
void hfpHandleResponseCSRGetSms(Task task, const struct hfpHandleResponseCSRGetSms *sms)
{
	HFP *hfp = (HFP*)task;
	if (hfp->use_csr2csr)
	{
		hfpSendCsrSmsCfm(hfp, hfp_success, sms->sms.length, sms->sms.data);
	}
}

/*
	Handle +CSR= sent from AG
*/
void hfpHandleResponseCSRModAgIndicators(Task task, const struct hfpHandleResponseCSRModAgIndicators *inds)
{
	HFP *hfp = (HFP*)task;
	if (hfp->use_csr2csr)
	{
		uint16 num = inds->p.count;
		uint16 cnt;
		struct value_hfpHandleResponseCSRModAgIndicators_p ind;
		
		/* Allocate message */
	    MAKE_HFP_MESSAGE_WITH_LEN(HFP_CSR_MODIFY_AG_INDICATORS_IND, (sizeof(hfp_csr_mod_indicator) * num));
    	message->hfp = hfp;
    	message->size_indicators = num;

		for (cnt = 0; cnt < num; cnt++)
		{
			/* get indicator */
			ind = get_hfpHandleResponseCSRModAgIndicators_p(&inds->p, cnt);
			/* add to buffer */
			message->indicators[cnt].indicator = ind.ind;
			message->indicators[cnt].value = ind.val;
		}
		
		/* Send message */
		MessageSend(hfp->clientTask, HFP_CSR_MODIFY_AG_INDICATORS_IND, message);
	}
}

/*
	Handle +CSR=0 sent from AG
*/
void hfpHandleResponseCSRModAgIndicatorsDisable(Task task)
{
	HFP *hfp = (HFP*)task;
	if (hfp->use_csr2csr)
	{
        MAKE_HFP_MESSAGE(HFP_CSR_AG_INDICATORS_DISABLE_IND);
        message->hfp = hfp;
        MessageSend(hfp->clientTask, HFP_CSR_AG_INDICATORS_DISABLE_IND, message);
	}
}

/*
	Handle AG requesting a battery report
*/
void hfpHandleResponseCSRBatRequest(Task task)
{
	HFP *hfp = (HFP*)task;
	if (hfp->use_csr2csr)
	{
        MessageSend(&hfp->task, HFP_INTERNAL_CSR_AG_BAT_REQ, NULL);
	}
}

/*
	Handle internal message for AG Requesting a battery report
*/
void hfpHandleInternalResponseCSRBatRequest(HFP *hfp)
{
    MAKE_HFP_MESSAGE(HFP_CSR_AG_REQUEST_BATTERY_IND);
    message->hfp = hfp;
    MessageSend(hfp->clientTask, HFP_CSR_AG_REQUEST_BATTERY_IND, message);
}
